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

#ifndef __LED_CONTROL_H__
#define __LED_CONTROL_H__

#include "stdint.h"
#include "common.h"
#include "board.h"
#include "hal.h"
#include "ch.h"

#define LED_EVT_SHUTDOWN (1<<0)
#define LED_EVT_UPDATE   (1<<1)

typedef struct {
	union {
		struct {
			uint8_t w;
			uint8_t b;
			uint8_t r;
			uint8_t g;
		};
		uint32_t raw;
	};
} led_colour_type;


typedef struct {
	uint32_t cycleCount;
	uint8_t enabled;

	uint8_t palette_size;
	uint8_t length;
	colour *palette;
	uint8_t *pixels;
} led_module_type;


#if !defined(_FROM_ASM_)
#ifdef __cplusplus
extern "C" {
#endif
	void ledInit(led_module_type* led);

	THD_FUNCTION(ledControlThread, arg);
	
#ifdef __cplusplus
}
#endif
#endif /* _FROM_ASM_ */


#endif
