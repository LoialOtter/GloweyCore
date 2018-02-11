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

#ifndef __COMMON_H__
#define __COMMON_H__

#include "stdint.h"

#define NIL_THREAD_LED      0
#define NIL_THREAD_PWR_CTRL 1
#define NIL_THREAD_CTRL     2


#define CTRL_EVT_WAKEUP (1<<0)


// fill in some types I prefer having available
typedef uint8_t       u8;
typedef uint16_t     u16;
typedef uint32_t     u32;
typedef signed char   s8;
typedef signed short s16;
typedef signed int   s32;



uint32_t rand(uint16_t a, uint16_t b);

uint16_t int_sqrt32(uint32_t x);
uint32_t SquareRoot(uint32_t a_nInput);
void Pause(int number);

typedef union {
	u32 raw;
	struct {
		s16 x, y;
	};
} colour_coord;

typedef union {
  u32 raw;
	struct {
		uint8_t white;
		uint8_t blue;
		uint8_t red;
		uint8_t green;
	};
	struct {
		u16 bright;
		u8 hue, saturation;
	};
} colour;

#define COLOUR_RED    (0x00)
#define COLOUR_BLUE   (0x00)
#define COLOUR_GREEN  (0x00)
#define COLOUR_PINK   (0x00)
#define COLOUR_PURPLE (0x00)
#define COLOUR_YELLOW (0x00)
#define COLOUR_ORANGE (0x00)
#define COLOUR_CYAN   (0x00)

extern const colour coord_lookup[];
extern const colour_coord hue_lookup[];

void hs_to_xy(u8 hue, u8 saturation, s16* out_x, s16* out_y);
void xy_to_rgb(s16 x, s16 y, colour* out);


typedef enum { STATE_INIT, STATE_RUNNING, STATE_SLEEPING, STATE_SAMPLE, STATE_UNDERVOLTAGE } state_type;

typedef enum { MODE_STANDARD, MODE_RAINBOW, MODE_SHIMERING, MODE_PULSING, MODE_XYZ, MODE_HUESAT, MODE_CONFIG_BRIGHTNESS } mode_type;
typedef enum { CONFIG_OFF, CONFIG_MODE, CONFIG_DOWN_COLOUR, CONFIG_SIDE_COLOUR, CONFIG_ACTION_COLOUR, CONFIG_BRIGHTNESS, CONFIG_PULSING_SPEED, CONFIG_PULSING_JITTER } config_state_type;

#define EEPROM_ADDR_COLOUR_DOWN   0
#define EEPROM_ADDR_COLOUR_SIDE   1
#define EEPROM_ADDR_COLOUR_ACTION 2
#define EEPROM_ADDR_MODE          3
#define EEPROM_ADDR_BRIGHTNESS    4

typedef struct {
	mode_type mode;
	mode_type last_mode;
	config_state_type config_state;
	u8 config_time;
	u8 active_time;
	
	u8 button_counter;
	u16 voltage;
	
	colour_coord colour_down;
	colour_coord colour_side;
	colour_coord colour_action;
	u16 brightness;
	
	unsigned button_short_press : 1;
	unsigned button_long_press  : 1;
	unsigned power_toggle       : 1;
	unsigned button_wait_til_release : 1;
	unsigned charge_status_acknowledged : 1;
	unsigned charge_status_acknowledged_received : 1;

	unsigned config_brightness_enter : 1;
} sys_state_type;

extern sys_state_type sys_state;



#endif /* __COMMON_H__ */
