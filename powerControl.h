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


#ifndef __POWER_CONTROL_H__
#define __POWER_CONTROL_H__


#include "stdint.h"
#include "board.h"
#include "hal.h"
#include "ch.h"


#define PWR_EVT_SHUTDOWN EVT_MASK(0)
#define PWR_EVT_UPDATE   EVT_MASK(1)


#define PWR_BATTERY_FULL     0x0F
#define PWR_BATTERY_EMPTY    0x02
#define PWR_BATTERY_LP_SD    0x01
#define PWR_BATTERY_FAILED   0x00


#define ADC_GRP1_NUM_CHANNELS   2
#define ADC_GRP1_BUF_DEPTH      4

typedef struct {
	uint16_t batteryVoltage;
	uint16_t temperature;

	uint8_t batteryState;
	uint8_t batteryCharging;

	adcsample_t samples[ADC_GRP1_NUM_CHANNELS * ADC_GRP1_BUF_DEPTH];
	int32_t voltageInverse;
	int32_t voltageOffset;
	
	uint32_t cycleCount;
} power_module_type;



#if !defined(_FROM_ASM_)
#ifdef __cplusplus
extern "C" {
#endif
	void pwrInit(power_module_type* pwr);

	void extCB_ChargePin(EXTDriver *extp, expchannel_t channel);

	THD_FUNCTION(pwrControlThread, arg);
	
#ifdef __cplusplus
}
#endif
#endif /* _FROM_ASM_ */



#endif /* __POWER_CONTROL_H__ */
