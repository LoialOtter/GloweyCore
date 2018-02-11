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

#include "ledControl.h"
#include "hal.h"
#include "board.h"
#include "string.h"

extern uint8_t lowPowerMode;

const uint8_t led_bit_patterns[] = { 0xE0, 0xFC };

#define WS2812_HIGH()   GPIOB->BSRR.H.set   = (1 << GPIOB_WS2812)
#define WS2812_LOW()    GPIOB->BSRR.H.clear = (1 << GPIOB_WS2812)
#define WS2812_SET(val) GPIOB->BSRR.W       = val ? 0x00000002 : 0x00020000

void ledUpdate(led_module_type* leds) {
	int lednum, bitnum;
	uint32_t value;
	leds->enabled = 1;
	palSetPad(GPIOC, GPIOC_EN);

	WS2812_LOW();
	palSetPadMode(GPIOB, GPIOB_WS2812, PAL_MODE_OUTPUT_PUSHPULL);

	Pause(100);
	
	chSysLock();
	for (lednum = 0; lednum < leds->length; lednum++) {
		value = leds->palette[leds->pixels[lednum]].raw;
		for (bitnum = 32; bitnum > 0; --bitnum) {
			if (value & 0x80000000) {
				WS2812_HIGH();
				__NOP(); __NOP(); __NOP(); __NOP(); __NOP(); __NOP();
				__NOP(); __NOP(); __NOP(); __NOP(); __NOP(); __NOP();
				WS2812_LOW();
				__NOP(); __NOP();
			}
			else {
				WS2812_HIGH();
				__NOP(); __NOP(); __NOP(); __NOP(); __NOP(); __NOP();
				WS2812_LOW();
				__NOP(); __NOP(); __NOP(); __NOP(); __NOP(); __NOP();
			}
			value <<= 1;
		}
	}
	chSysUnlock();
	palSetPadMode(GPIOB, GPIOB_WS2812, PAL_MODE_INPUT_PULLDOWN);

	RCC->APB2ENR &= ~RCC_APB2ENR_SPI1EN;
}

void ledShutdown(led_module_type* leds) {
	palSetPadMode(GPIOB, GPIOB_WS2812, PAL_MODE_INPUT_PULLUP);
	palClearPad(GPIOC, GPIOC_EN);
	leds->enabled = 0;
}

void ledInit(led_module_type* leds) {
	(void)leds;
}


/*
 * Thread 1.
 */
THD_FUNCTION(ledControlThread, arg) {
	led_module_type* leds = (led_module_type*)arg;
	
	palSetPadMode(GPIOC, GPIOC_EN, PAL_MODE_OUTPUT_PUSHPULL);
	palClearPad(GPIOC, 14);

	while (true) {
		eventmask_t evt = chEvtWaitAnyTimeout(ALL_EVENTS, TIME_INFINITE);
		
		if (evt & LED_EVT_SHUTDOWN) {
			ledShutdown(leds);
		}
		else if (evt & LED_EVT_UPDATE) {
			// if it was in standby before, run the update twice
			if (!leds->enabled) {
				ledUpdate(leds);
			}
			ledUpdate(leds);
		}
		
		leds->cycleCount++;
	}
}

