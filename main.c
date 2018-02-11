/*
    Glowey Core - Copyright (C) 2017..2018 Matthew Peters

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

#include "board.h"
#include "hal.h"
#include "ch.h"
#include "sleep.h"
#include "ledControl.h"
#include "powerControl.h"
#include "mma8653fc.h"
#include "common.h"
#include "eeprom.h"



#define ACCEL_12BIT

//#define SIDEWAYS

#define BUTTON_OFF_TIME 2
#define BUTTON_ON_TIME 2
#define BUTTON_PROGRAM_TIME 15
#define SETTING_SHOW_TIME 15
#define BATTERY_STATUS_TIME 10

#define MAG_LIST_SHIFT  8
#define MAG_LIST_LENGTH (1<<MAG_LIST_SHIFT)
#define NOISE_FILTER_PERIOD 8
#define NOISE_SCALING 0 /* 1/4 */
#define NOISE_MARGINE 0x230
#define MIN_BRIGHT 5
#define MAX_BRIGHT 255
#define ACCEL_SCALING 32
#define ACCEL_MARGINE 0x0000

#define TURNOFF_TIME 500 /* errr... something... */
#define FADE_SPEED 2 /* full fade time ~5s */
#define TURNOFF_ON_SIDE_MARGINE 0xC00
#define TURNOFF_NOISE_MARGINE 0x2FF


static const colour charging_colours[] = {
	/*  0 */ { .red = 0, .green=0, .blue=0, .white=0 }, /* 0V */
	/*  1 */ { .red = 0, .green=0, .blue=0, .white=0 }, /* 2.5V */
	/*  2 */ { .red = 1, .green=0, .blue=0, .white=0 }, /* 2.85V */
	/*  3 */ { .red = 1, .green=0, .blue=0, .white=0 }, /* 3.0V */
	/*  4 */ { .red = 1, .green=0, .blue=0, .white=0 }, /* 3.11V */
	/*  5 */ { .red = 1, .green=0, .blue=0, .white=0 }, /* 3.15V */
	/*  6 */ { .red = 1, .green=0, .blue=0, .white=0 }, /* 3.18V */
	/*  7 */ { .red = 1, .green=0, .blue=0, .white=0 }, /* 3.21V */
	/*  8 */ { .red = 1, .green=0, .blue=0, .white=0 }, /* 3.23V */
	/*  9 */ { .red = 1, .green=0, .blue=0, .white=0 }, /* 3.25V */
	/* 10 */ { .red = 1, .green=0, .blue=0, .white=0 }, /* 3.27V */
	/* 11 */ { .red = 1, .green=0, .blue=0, .white=0 }, /* 3.30V */
	/* 12 */ { .red = 1, .green=0, .blue=0, .white=0 }, /* 3.33V */
	/* 13 */ { .red = 1, .green=1, .blue=0, .white=0 }, /* 3.35V */
	/* 14 */ { .red = 1, .green=1, .blue=0, .white=0 }, /* 3.45V */
	/* 15 */ { .red = 0, .green=2, .blue=0, .white=0 }  /* 3.60V */
};

static const colour battery_status_colours[] = {
	/*  0 */ { .red =  0, .green=  0, .blue=0, .white=0 },
	/*  1 */ { .red =  0, .green=  0, .blue=0, .white=0 },
	/*  2 */ { .red = 10, .green=  0, .blue=0, .white=0 },
	/*  3 */ { .red = 10, .green=  0, .blue=0, .white=0 },
	/*  4 */ { .red = 10, .green=  0, .blue=0, .white=0 },
	/*  5 */ { .red = 10, .green=  0, .blue=0, .white=0 },
	/*  6 */ { .red = 10, .green=  0, .blue=0, .white=0 },
	/*  7 */ { .red = 10, .green=  1, .blue=0, .white=0 },
	/*  8 */ { .red =  9, .green=  2, .blue=0, .white=0 },
	/*  9 */ { .red =  9, .green=  5, .blue=0, .white=0 },
	/* 10 */ { .red =  8, .green=  7, .blue=0, .white=0 },
	/* 11 */ { .red =  6, .green=  9, .blue=0, .white=0 },
	/* 12 */ { .red =  3, .green= 10, .blue=0, .white=0 },
	/* 13 */ { .red =  1, .green= 10, .blue=0, .white=0 },
	/* 14 */ { .red =  0, .green= 10, .blue=0, .white=0 },
	/* 15 */ { .red =  0, .green= 10, .blue=0, .white=0 }
};

u8 button_power;

sys_state_type sys_state = {
  .config_state = CONFIG_OFF,
  .colour_down   = { .x = -12000, .y = -16000 },
  .colour_side   = { .x =      0, .y = -16000 },
  .colour_action = { .x =      0, .y =      0 },
  .button_short_press = 0,
  .button_long_press = 0,
  .power_toggle = 0
}; 

colour palette[] = {
  { .red=0x00, .green=0x01, .blue=0x00, .white=0x00 }, // 1
  { .red=0x00, .green=0x00, .blue=0x00, .white=0x00 }, // 2
};
u8 colour_indexes[] = {0,0};//,7,1,2,3,4,5,6,7,1,2,3,4,5,6,7,1,2,3};

led_module_type leds = {
	.enabled      = 1,

	.palette_size = 2,
	.palette      = palette,
	.length       = 2,
	.pixels       = colour_indexes,
};

power_module_type pwr;

THD_WORKING_AREA(waLedControlThread, 100);
THD_WORKING_AREA(waPwrControlThread, 100);
THD_WORKING_AREA(waControlThread, 256);

THD_FUNCTION(controlThread, arg);

/*
 * Threads static table, one entry per thread. The number of entries must
 * match NIL_CFG_NUM_THREADS.
 */
THD_TABLE_BEGIN
    THD_TABLE_ENTRY(waLedControlThread, "LED", ledControlThread, (void*)&leds)
	THD_TABLE_ENTRY(waPwrControlThread, "pwr-ctrl", pwrControlThread, (void*)&pwr)
	THD_TABLE_ENTRY(waControlThread, "ctrl", controlThread, NULL)
THD_TABLE_END





/* Triggered when the button is pressed or released. The LED4 is set to ON.*/
static void extCB_Buttons(EXTDriver *extp, expchannel_t channel) {
	(void)extp;

	if (channel == 0) {
		if (button_power) {
			if ((GPIOA->IDR & GPIOA_BUTTON) == 0) {
				sys_state.button_counter = 0;
				sys_state.button_short_press = 0;
				sys_state.button_long_press = 0;
				
				if (!sys_state.charge_status_acknowledged_received) {
					sys_state.charge_status_acknowledged = 1;
				}
				else {
					button_power = 0;

					chSysLockFromISR();
					chEvtSignalI(&nil.threads[NIL_THREAD_CTRL], CTRL_EVT_WAKEUP);
					chSysUnlockFromISR();
				}
			}
		}
	}
}

static const EXTConfig extcfg = {
	{
		/*  0 */ {EXT_CH_MODE_BOTH_EDGES | EXT_CH_MODE_AUTOSTART | EXT_MODE_GPIOA, extCB_Buttons},
		/*  1 */ {EXT_CH_MODE_BOTH_EDGES | EXT_CH_MODE_AUTOSTART | EXT_MODE_GPIOA, extCB_Buttons},
		/*  2 */ {EXT_CH_MODE_DISABLED, NULL},
		/*  3 */ {EXT_CH_MODE_DISABLED, NULL},
		/*  4 */ {EXT_CH_MODE_DISABLED, NULL},
		/*  5 */ {EXT_CH_MODE_DISABLED, NULL},
		/*  6 */ {EXT_CH_MODE_DISABLED, NULL},
		/*  7 */ {EXT_CH_MODE_DISABLED, NULL},
		/*  8 */ {EXT_CH_MODE_DISABLED, NULL},
		/*  9 */ {EXT_CH_MODE_BOTH_EDGES | EXT_CH_MODE_AUTOSTART | EXT_MODE_GPIOA, extCB_Buttons},
		/* 10 */ {EXT_CH_MODE_BOTH_EDGES | EXT_CH_MODE_AUTOSTART | EXT_MODE_GPIOA, extCB_Buttons},
		/* 11 */ {EXT_CH_MODE_DISABLED, NULL},
		/* 12 */ {EXT_CH_MODE_DISABLED, NULL},
		/* 13 */ {EXT_CH_MODE_DISABLED, NULL},
		/* 14 */ {EXT_CH_MODE_DISABLED, NULL},
		/* 15 */ {EXT_CH_MODE_RISING_EDGE | EXT_CH_MODE_AUTOSTART | EXT_MODE_GPIOC, extCB_ChargePin},
		/* 16 */ {EXT_CH_MODE_DISABLED, NULL},
		/* 17 */ {EXT_CH_MODE_DISABLED, NULL},
		/* 18 */ {EXT_CH_MODE_DISABLED, NULL},
		/* 19 */ {EXT_CH_MODE_DISABLED, NULL},
		/* 20 */ {EXT_CH_MODE_DISABLED, NULL},
		/* 21 */ {EXT_CH_MODE_DISABLED, NULL},
		/* 22 */ {EXT_CH_MODE_DISABLED, NULL}
	}
};

















	
const accel_settings_type accel_off = {
  .ctrl_reg1 = 0,
  .ctrl_reg2 = 0,
  .ctrl_reg3 = 0,
  .ctrl_reg4 = 0,
  .ctrl_reg5 = 0
};
const accel_settings_type accel_run = {
  .ctrl_reg1 = MMA8653FC_CTRL_REG1_ASLP_RATE_50Hz | MMA8653FC_CTRL_REG1_DR_50Hz | MMA8653FC_CTRL_REG1_ACTIVE,
  .ctrl_reg2 = MMA8653FC_CTRL_REG2_SMODS_LOWPWR | MMA8653FC_CTRL_REG2_MODS_LOWPWR,
  .ctrl_reg3 = MMA8653FC_CTRL_REG3_PP_OD,
  .ctrl_reg4 = MMA8653FC_CTRL_REG4_INT_EN_DRDY,
  .ctrl_reg5 = 0xFF
};



u8 fade_level;
s16 turnoff_timeout;

u16 average;
u32 smoothed_noise_level;

u16 mag_list_immediate[16]; // about 3 seconds
u16 mag_list_immediate_write_point = 0;
u16 mag_list_transfer[16];
u16 mag_list_transfer_write_point = 0;
u16 mag_list_long[16]; // about 256 samples (20.5 seconds)
u16 mag_list_long_write_point = 0;

#define MAG_IIR_LENGTH 4
u32 mag_IIR_filter;
s32 mag_average;

void add_value(u16 mag) {
	int i;
	u32 average;
	mag_list_transfer[mag_list_transfer_write_point] = mag_list_immediate[mag_list_immediate_write_point];
	mag_list_transfer_write_point++;
	if (mag_list_transfer_write_point >= 16) {
		average = 0;
		for (i = 15; i >= 0; i--) average += mag_list_transfer[i];
		average >>= 4;
		mag_list_long[mag_list_long_write_point] = average;
		mag_list_long_write_point++;
		if (mag_list_long_write_point >= 16) mag_list_long_write_point = 0;
		mag_list_transfer_write_point = 0;
	}
	
	mag_list_immediate[mag_list_immediate_write_point] = mag;
	mag_list_immediate_write_point++;
	if (mag_list_immediate_write_point >= 16) mag_list_immediate_write_point = 0;
	
	mag_IIR_filter = mag_IIR_filter - (mag_IIR_filter >> MAG_IIR_LENGTH) + mag;
	mag_average = mag_IIR_filter >> MAG_IIR_LENGTH;
}

void reset_filter(u16 mag) {
	int i;
	for (i = 0; i < 16; i++) {
		mag_list_immediate[i] = mag;
		mag_list_transfer[i]  = mag;
		mag_list_long[i]      = mag;
	}
	mag_IIR_filter = mag << MAG_IIR_LENGTH;
	mag_average = mag;
}

typedef struct {
	s16 x, y, z;
} accel_info_type;
accel_info_type accel;

u16 get_mag(void) {
	return int_sqrt32((s32)accel.x*(s32)accel.x + (s32)accel.y*(s32)accel.y + (s32)accel.z*(s32)accel.z);
}



s32 down, horiz, rotation;
s32 inv_mag;


void show_mode(mode_type mode, colour* out) {
	switch (mode) {
	case MODE_STANDARD:  out->raw = 0x00003F00; break; // out->red = 0x00; out->green = 0x00; out->blue = 0xFF; out->white = 0x00; break;
	case MODE_RAINBOW:   out->raw = 0x3F000000; break; // out->red = 0x00; out->green = 0xFF; out->blue = 0x00; out->white = 0x00; break;
	case MODE_SHIMERING: out->raw = 0x00000000; break; // out->red = 0x00; out->green = 0x00; out->blue = 0x00; out->white = 0x00; break;
	case MODE_PULSING:   out->raw = 0x00000000; break; // out->red = 0x00; out->green = 0x00; out->blue = 0x00; out->white = 0x00; break;
	case MODE_XYZ:       out->raw = 0x003F3F00; break; // out->red = 0xFF; out->green = 0x00; out->blue = 0xFF; out->white = 0x00; break;
	case MODE_HUESAT:    out->raw = 0x003F0000; break; // out->red = 0xFF; out->green = 0x00; out->blue = 0x00; out->white = 0x00; break;
	case MODE_CONFIG_BRIGHTNESS: out->raw = 0x40404040; break;
	}
}
mode_type decode_mode(void) {
	if (accel.z >= 0x900) {
		return MODE_CONFIG_BRIGHTNESS;
	}
	else if (accel.z <= -0x900) {
		return MODE_CONFIG_BRIGHTNESS;
	}
	else if (accel.x >= 0) {
		if (accel.y >= 0) {
			return MODE_STANDARD;
		}
		else {
			return MODE_RAINBOW;
		}
	}
	else {
		if (accel.y >= 0) {
			return MODE_XYZ;
		}
		else {
			return MODE_HUESAT;
		}
	}
	return MODE_STANDARD;
}
u32 noise_level;
u8 red, green, blue;
u16 mag;

s16 colour_x, colour_y;
u8 hue, saturation, bright;
u8 update_hue_count;

void calculate_colours(void) {
	int i;
	u32 average_accum = 0;
	u16 abs_x, abs_y, abs_z;
	s32 temp_bright;
	
	// mag scale 0x4000 for 1g
	mag = get_mag(); // this just does sqrt(x*x+y*y+z*z)
	
	add_value(mag);  
	
	for (i = 16; i >= 0; i--) {
		// fast FIR
		average_accum += mag_list_immediate[i];
		if (mag_list_immediate[i] > average) noise_level += mag_list_immediate[i] - average;
		else                                 noise_level += average - mag_list_immediate[i];
		// slow FIR
		average_accum += mag_list_long[i];
		if (mag_list_long[i] > average) noise_level += mag_list_long[i] - average;
		else                            noise_level += average - mag_list_long[i];
	}
	average_accum >>= 5;
	average = average_accum;
	noise_level >>= 5;
	smoothed_noise_level = smoothed_noise_level - (smoothed_noise_level >> NOISE_FILTER_PERIOD) + noise_level;
	noise_level = smoothed_noise_level >> (NOISE_FILTER_PERIOD);
	if (noise_level > NOISE_MARGINE) noise_level -= NOISE_MARGINE;
	else noise_level = 0;
	noise_level >>= NOISE_SCALING;
	if (noise_level >= 0x4000) noise_level = 0x3FFF;
	
	
	if (accel.x >= 0) abs_x = accel.x;
	else              abs_x = -(accel.x);
	if (accel.y >= 0) abs_y = accel.y;
	else              abs_y = -(accel.y);
	if (accel.z >= 0) abs_z = accel.z;
	else              abs_z = -(accel.z);
	
	// z
	inv_mag = (0x4000*0x4000) / mag;
	//             ADJUST
	down    = 0x4000-((abs_z * inv_mag) >> (14));
	if (down >= 0x4000) down = 0x3FFF;
	if (down < 0)       down = 0;
	
	mag_average = mag_average - 0x4000;
	if (mag_average < 0) mag_average = -mag_average;
	mag_average *= ACCEL_SCALING;
	
	if (mag_average > ACCEL_MARGINE) mag_average -= ACCEL_MARGINE;
	else                             mag_average = 0;
	
	// DISABLE THE TURN OFF!!! --------------------------------------------------------------------------
	//fade_level = 255;
	
	// ---- this is where I was going to put the pulsing code
	
	if (sys_state.config_state != CONFIG_OFF) {
		if (sys_state.config_time <= SETTING_SHOW_TIME) {
			switch (sys_state.config_state) {
			default:
			case CONFIG_MODE:
				show_mode(sys_state.mode, &palette[0]);
				break;
			case CONFIG_BRIGHTNESS:
				palette[0].red   = ((u32)0xFF * (u32)sys_state.brightness) >> 16;
				palette[0].green = ((u32)0xFF * (u32)sys_state.brightness) >> 16;
				palette[0].blue  = ((u32)0xFF * (u32)sys_state.brightness) >> 16;
				palette[0].white = ((u32)0xFF * (u32)sys_state.brightness) >> 16;
				break;
			case CONFIG_DOWN_COLOUR:
				xy_to_rgb(sys_state.colour_down.x, sys_state.colour_down.y, &palette[0]);
				break;
			case CONFIG_SIDE_COLOUR:
				xy_to_rgb(sys_state.colour_side.x, sys_state.colour_side.y, &palette[0]);
				break;
			case CONFIG_ACTION_COLOUR:
				xy_to_rgb(sys_state.colour_action.x, sys_state.colour_action.y, &palette[0]);
				break;
			}
		}
		else {
			switch (sys_state.config_state) {
			default:
			case CONFIG_MODE:
				show_mode(decode_mode(), &palette[0]);
				break;
			case CONFIG_BRIGHTNESS:
				temp_bright = (((accel.z * inv_mag)>>16) + 0xE00) << 3;
				if (temp_bright < 0x300) temp_bright = 0x300;
				if (temp_bright > 0xFFFF) temp_bright = 0xFFFF;
				palette[0].red   = ((u32)0xFF * temp_bright) >> 16;
				palette[0].green = ((u32)0xFF * temp_bright) >> 16;
				palette[0].blue  = ((u32)0xFF * temp_bright) >> 16;
				palette[0].white = ((u32)0xFF * temp_bright) >> 16;
				break;
			case CONFIG_DOWN_COLOUR:
			case CONFIG_SIDE_COLOUR:
			case CONFIG_ACTION_COLOUR:
				xy_to_rgb(((accel.x * inv_mag) >> (14)), ((accel.y * inv_mag) >> (14)), &palette[0]);
				break;
			}
		}
	}
	else {
		//if (noise_level < TURNOFF_NOISE_MARGINE && (down > (0x4000-TURNOFF_ON_SIDE_MARGINE) && down < (0x4000+TURNOFF_ON_SIDE_MARGINE))) {
		//  turnoff_timeout--;
		//  if (turnoff_timeout < 0) {
		//    turnoff_timeout = 0;
		//    if (fade_level >= FADE_SPEED) fade_level -= FADE_SPEED;
		//    else fade_level = 0;
		//  }
		//}
		//else {
		//  turnoff_timeout++;
		//  if (turnoff_timeout > TURNOFF_TIME) turnoff_timeout = TURNOFF_TIME;
		//  if (fade_level <= (255-FADE_SPEED)) fade_level += FADE_SPEED;
		//  else fade_level = 255;
		//}
		
		//========================================= MODES ============================================
		switch (sys_state.mode) {
		case MODE_STANDARD:      
			colour_x = ((sys_state.colour_side.x * down) >> 14) + ((sys_state.colour_down.x * (0x3FFF-down)) >> 14);
			colour_y = ((sys_state.colour_side.y * down) >> 14) + ((sys_state.colour_down.y * (0x3FFF-down)) >> 14);
			
			colour_x = ((colour_x * (0x4000-noise_level)) >> 14) + ((sys_state.colour_action.x * noise_level) >> 14);
			colour_y = ((colour_y * (0x4000-noise_level)) >> 14) + ((sys_state.colour_action.y * noise_level) >> 14);
			
			xy_to_rgb(colour_x, colour_y, &palette[0]);
			break;
		case MODE_RAINBOW: {
			u8 offset = 0;
			hue += 1;
			saturation = 0xFF;
			for (i = 0; i < leds.palette_size; i++) {
				hs_to_xy(hue + offset, saturation, &colour_x, &colour_y); xy_to_rgb(colour_x, colour_y, &palette[i]);
				offset += 37;
			}
		}      
		break;
		case MODE_XYZ:
			palette[0].red   = ((u32)(abs_x >> 7) * (u32)sys_state.brightness) >> 16;
			palette[0].green = ((u32)(abs_y >> 7) * (u32)sys_state.brightness) >> 16;
			palette[0].blue  = ((u32)(abs_z >> 7) * (u32)sys_state.brightness) >> 16;
			palette[0].white = 0;
			break;
		case MODE_HUESAT:
			xy_to_rgb(((accel.x * inv_mag) >> (14)), ((accel.y * inv_mag) >> (14)), &palette[0]);
			break;      
		case MODE_SHIMERING:
		case MODE_PULSING:
			for (i = 0; i < leds.palette_size; i++) {
				palette[0].red   = 255;
				palette[0].green = 255;
				palette[0].blue  = 255;
				palette[0].white = 255;
			}
			break;
		}
		//============================================================================================
		if (sys_state.mode != MODE_XYZ) {
			temp_bright = MIN_BRIGHT + (mag_average >> 10) + (noise_level >> 2);
			temp_bright = ((u32)temp_bright * (u32)sys_state.brightness) >> 16;
			if (temp_bright > MAX_BRIGHT) temp_bright = MAX_BRIGHT;
			//temp_bright = (temp_bright * fade_level) >> 8;
			bright = temp_bright;
			
			//bright = 0x40;
			for (i = 0; i < leds.palette_size; i++) {
				palette[i].red   = (palette[i].red   * bright) >> 8;
				palette[i].green = (palette[i].green * bright) >> 8;
				palette[i].blue  = (palette[i].blue  * bright) >> 8;
				palette[i].white = (palette[i].white * bright) >> 8;
				//palette[i].intensity = 31;
			}
		}
	}
	
	//get_rgb(mag_average, noise_level, down, fade_level, 0x80, 0x80, &red, &green, &blue);
	
	//palette[0].red   = red   >> 1;
	//palette[0].green = green >> 1;
	//palette[0].blue  = blue  >> 1;
	//palette[0].intensity = 31;
	
	
	//palette[0].red   = ((u16)palette[0].red   * (u16)fade_level) >> (8+2);
	//palette[0].green = ((u16)palette[0].green * (u16)fade_level) >> (8+2);
	//palette[0].blue  = ((u16)palette[0].blue  * (u16)fade_level) >> (8+2);
	//palette[0].intensity = 31;
	
	
	////palette[0].red   = 0xFF >> 1;
	////palette[0].green = 0xFF >> 1;
	////palette[0].blue  = 0xFF >> 1;
	////palette[0].intensity = 31;
	////palette[0].red   = (palette[0].red   * bright) >> 8;
	////palette[0].green = (palette[0].green * bright) >> 8;
	////palette[0].blue  = (palette[0].blue  * bright) >> 8;
	////palette[0].intensity = 31;
	////for (i = 0; i < leds.palette_size; i++) {
	////  palette[i].raw = palette[0].raw;
	////}
	//
	////if (red < 128)   palette[2].red   = red<<1;
	////else             palette[2].red   = 255;
	////if (green < 128) palette[2].green = green<<1;
	////else             palette[2].green = 255;
	////if (blue < 128)  palette[2].blue  = blue<<1;
	////else             palette[2].blue  = 255;
	////palette[2].intensity = 31;
}

//=============================================================
//=============================================================
void do_apply_config(void) {
	int save_settings = 0;
	mode_type new_mode;
	uint32_t temp;
	int32_t temp_bright;
	if (sys_state.button_counter > BUTTON_OFF_TIME) {
		if (sys_state.config_time > (BUTTON_OFF_TIME + SETTING_SHOW_TIME)) save_settings = 1;
	}
	else {
		if (sys_state.config_time > SETTING_SHOW_TIME) save_settings = 1;
	}
	
	if (save_settings) {
		switch (sys_state.config_state) {
		case CONFIG_OFF:
			break;
		case CONFIG_MODE:
			new_mode = decode_mode();
			if (new_mode != MODE_CONFIG_BRIGHTNESS) {
				sys_state.mode = decode_mode();
				temp = sys_state.mode;
				eeprom_write(EEPROM_ADDR_MODE, 1, &temp);
			}
			else {
				sys_state.config_state = CONFIG_BRIGHTNESS;
				sys_state.config_brightness_enter = 1;
			}
			break;
		case CONFIG_DOWN_COLOUR:
			sys_state.colour_down.x = ((accel.x * inv_mag) >> (14));
			sys_state.colour_down.y = ((accel.y * inv_mag) >> (14));
			eeprom_write(EEPROM_ADDR_COLOUR_DOWN, 1, &sys_state.colour_down.raw);
			break;
		case CONFIG_SIDE_COLOUR:
			sys_state.colour_side.x = ((accel.x * inv_mag) >> (14));
			sys_state.colour_side.y = ((accel.y * inv_mag) >> (14));
			eeprom_write(EEPROM_ADDR_COLOUR_SIDE, 1, &sys_state.colour_side.raw);
			break;
		case CONFIG_ACTION_COLOUR:
			sys_state.colour_action.x = ((accel.x * inv_mag) >> (14));
			sys_state.colour_action.y = ((accel.y * inv_mag) >> (14));
			eeprom_write(EEPROM_ADDR_COLOUR_ACTION, 1, &sys_state.colour_action.raw);
			break;
		case CONFIG_BRIGHTNESS:
			temp_bright = (((accel.z * inv_mag)>>16) + 0xE00) << 3;
			if (temp_bright < 0x300) temp_bright = 0x300;
			if (temp_bright > 0xFFFF) temp_bright = 0xFFFF;
			sys_state.brightness = temp_bright;
			temp = sys_state.brightness;
			eeprom_write(EEPROM_ADDR_BRIGHTNESS, 1, &temp);
			break;
		case CONFIG_PULSING_SPEED:
		case CONFIG_PULSING_JITTER:
			break;
			//default:
		}		
	}
}


void do_config_long_press(void) {
	if (sys_state.config_state != CONFIG_OFF) {
		do_apply_config();
		sys_state.config_state = CONFIG_OFF;
	}
	else {
		sys_state.config_state = CONFIG_MODE;
	}
}
void do_config_short_press(void) {
	do_apply_config();
	switch (sys_state.config_state) {
	case CONFIG_OFF:
		break;
	case CONFIG_MODE:
		switch (sys_state.mode) {
		case MODE_STANDARD:
		case MODE_SHIMERING:
		case MODE_PULSING:
			sys_state.config_state = CONFIG_DOWN_COLOUR;
			break;
		default:
			sys_state.config_state = CONFIG_OFF;
		}
		break;
	case CONFIG_DOWN_COLOUR:
		sys_state.config_state = CONFIG_SIDE_COLOUR;
		break;
	case CONFIG_SIDE_COLOUR:
		sys_state.config_state = CONFIG_ACTION_COLOUR;
		break;
	case CONFIG_ACTION_COLOUR:
		//switch (sys_state.mode) {
		//case MODE_STANDARD:
		//case MODE_RAINBOW:
		//case MODE_SHIMERING:
		//case MODE_PULSING:
		//case MODE_XYZ:
		//case MODE_HUESAT:
		//  break;
		//}
		sys_state.config_state = CONFIG_OFF;
		break;
	case CONFIG_BRIGHTNESS:
		if (sys_state.config_brightness_enter) {
			sys_state.config_brightness_enter = 0;
		}
		else {
			sys_state.config_state = CONFIG_OFF;
		}
		break;
	case CONFIG_PULSING_SPEED:
		sys_state.config_state = CONFIG_OFF;
		break;
	case CONFIG_PULSING_JITTER:
		sys_state.config_state = CONFIG_OFF;
		break;
	}
}

#define check_interrupt() palReadPad(GPIOA, GPIOA_BUTTON)

void loadSettings(void) {
	uint32_t eeprom_buf[10];
	eeprom_read(0, 10, eeprom_buf);
	sys_state.colour_down.raw   = eeprom_buf[EEPROM_ADDR_COLOUR_DOWN];
	sys_state.colour_side.raw   = eeprom_buf[EEPROM_ADDR_COLOUR_SIDE];
	sys_state.colour_action.raw = eeprom_buf[EEPROM_ADDR_COLOUR_ACTION];
	sys_state.mode              = eeprom_buf[EEPROM_ADDR_MODE];
	sys_state.brightness        = eeprom_buf[EEPROM_ADDR_BRIGHTNESS];
}

THD_FUNCTION(controlThread, arg) {
	(void) arg;
	int i;
	
	accel_calibrate();
	accel_config(&accel_run);
	
#ifdef SIDEWAYS
	get_accel(&accel.z, &accel.x, &accel.y);
#else
	get_accel(&accel.x, &accel.y, &accel.z);
#endif
#ifdef ACCEL_12BIT
	accel.x >>= 2;
	accel.y >>= 2;
	accel.z >>= 2;
#endif
	
	// pre-load the filters
	mag = get_mag();
	for (i = 0; i < 16; i++) {
		mag_list_immediate[i] = mag;
		mag_list_transfer[i]  = mag;
		mag_list_long[i]      = mag;
	}
	mag_IIR_filter = mag << MAG_IIR_LENGTH;
	mag_average = mag;

	loadSettings();
	
	while (true) {
		//eventmask_t evt = chEvtWaitAnyTimeout(ALL_EVENTS, TIME_INFINITE);
		
		if (!button_power) {
#ifdef SIDEWAYS
			get_accel(&accel.z, &accel.x, &accel.y);
#else
			get_accel(&accel.x, &accel.y, &accel.z);
#endif
#ifdef ACCEL_12BIT
			accel.x >>= 2;
			accel.y >>= 2;
			accel.z >>= 2;
#endif
		}

		//-------------------------------
		// check button
		sys_state.charge_status_acknowledged = 0;
		sys_state.button_short_press = 0;
		sys_state.button_long_press = 0;
		sys_state.power_toggle = 0;
		if (sys_state.button_wait_til_release) {
			if (check_interrupt()) 
				sys_state.button_wait_til_release = 0;
			
			chThdSleepMilliseconds(100);
		}
		else {
			if (!check_interrupt()) {
				if (sys_state.button_counter < 255) sys_state.button_counter++;
			}
			else {
				if (sys_state.button_counter > BUTTON_PROGRAM_TIME) {
					sys_state.button_long_press = 1;
				}
				else if (sys_state.button_counter > BUTTON_OFF_TIME) {
					sys_state.button_short_press = 1;
				}
				sys_state.button_counter = 0;
			}
			
			if (button_power) {
				accel_config(&accel_off);

				while(button_power) {
					if (sys_state.charge_status_acknowledged) {
						sys_state.charge_status_acknowledged_received = 1;
					}
					
					if (pwr.batteryCharging && !sys_state.charge_status_acknowledged && charging_colours[pwr.batteryState].raw != 0) {
						palette[0].raw   = charging_colours[pwr.batteryState].raw;
						chEvtSignal(&nil.threads[NIL_THREAD_LED], LED_EVT_UPDATE);
						chEvtWaitAnyTimeout(ALL_EVENTS, MS2ST(500));
					}
					else {
						chEvtSignal(&nil.threads[NIL_THREAD_LED], LED_EVT_SHUTDOWN);
						chEvtWaitAnyTimeout(ALL_EVENTS, TIME_INFINITE);
					}
				}

				button_power = 0;
				sys_state.button_wait_til_release = 1;
				sys_state.active_time = 0;
				sys_state.charge_status_acknowledged_received = 0;
				
				accel_calibrate();
				accel_config(&accel_run);
			}
			else {
				
				// now decide what to do
				if (sys_state.button_long_press) {
					do_config_long_press();
					sys_state.config_time = 0;
				}
				if (sys_state.button_short_press) {
					if (sys_state.config_state != CONFIG_OFF) {
						do_config_short_press();
						sys_state.config_time = 0;
					}
					else {
						sys_state.power_toggle = 1;
					}
				}
				// should we turn off?
				if (sys_state.power_toggle) {
					//go_to_off_state();
					button_power = 1;
					chEvtSignal(&nil.threads[NIL_THREAD_LED], LED_EVT_SHUTDOWN);
					sys_state.active_time = 0;
				}
				else {
					
					if (sys_state.config_state != CONFIG_OFF) {
						if (sys_state.config_time < 255) sys_state.config_time++;
					}
					else {
						sys_state.config_time = 0;
					}

					if (sys_state.active_time < BATTERY_STATUS_TIME) {
						sys_state.active_time++;
						if (battery_status_colours[pwr.batteryState].raw != 0) {
							palette[0].raw   = battery_status_colours[pwr.batteryState].raw;
						}
					}
					else {
						calculate_colours();
					}
					
					chEvtSignal(&nil.threads[NIL_THREAD_LED], LED_EVT_UPDATE);
					
					chThdSleepMilliseconds(50);
				}
			}
		}
		
	}
}



/*
 * Application entry point.
 */
int main(void) {
	chSemSignal(&sleepDisabled);

	RCC->APB2ENR |= RCC_APB2ENR_DBGEN;
	*(uint32_t*)(0x40015804) = 7; // enable debugging in low power modes
	PWR->CR |= PWR_CR_FWU;
	
	/*
	 * System initializations.
	 * - HAL initialization, this also initializes the configured device drivers
	 *   and performs the board-specific initializations.
	 * - Kernel initialization, the main() function becomes a thread and the
	 *   RTOS is active.
	 */
	
	ledInit(&leds);
	pwrInit(&pwr);
		
	halInit();
	chSysInit();

	chSysLock();
	if (chSemGetCounterI(&sleepDisabled) > 0) chSemWaitTimeoutS(&sleepDisabled, TIME_IMMEDIATE);
	chSysUnlock();

	extStart(&EXTD1, &extcfg);
  
	/* This is now the idle thread loop, you may perform here a low priority
	   task but you must never try to sleep or wait in this loop. Note that
	   this tasks runs at the lowest priority level so any instruction added
	   here will be executed after all other tasks have been started.*/
	while (true) {
		enter_LPSleep();
	}
}


