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


#include "powerControl.h"
#include "ledControl.h"
#include "sleep.h"
#include "common.h"

//#define DEBUG_NO_BATTERY

#define VOLTAGE_INVERSE 0x001F0000
#define VOLTAGE_OFFSET  0



void extCB_ChargePin(EXTDriver *extp, expchannel_t channel) {
  (void)extp;
  (void)channel;
  chSysLockFromISR();
  chEvtSignalI(&nil.threads[NIL_THREAD_PWR_CTRL], CTRL_EVT_WAKEUP);
  chSysUnlockFromISR();
}

//                                0   1    2    3    4    5    6    7    8    9    A    B    C    D    E    F
const uint16_t stateVoltages[] = { 0, 250, 285, 300, 311, 315, 318, 321, 323, 325, 327, 330, 333, 335, 345, 360 };
void pwrSetChargeState(power_module_type* pwr) {
	int i;
	for (i = 15; i > 0; i--)
		if (pwr->batteryVoltage > stateVoltages[i]) break;
	pwr->batteryState = i;
}


void pwrInit(power_module_type* pwr) {
	pwr->batteryState = PWR_BATTERY_EMPTY;

	pwr->voltageInverse = VOLTAGE_INVERSE;
	pwr->voltageOffset  = VOLTAGE_OFFSET;
}



#define TEMP130_CAL_ADDR ((uint16_t*) ((uint32_t) 0x1FF8007E))
#define TEMP30_CAL_ADDR ((uint16_t*) ((uint32_t) 0x1FF8007A))
#define VDD_CALIB ((uint16_t) (300))
int32_t ComputeTemperature(power_module_type* pwr, uint32_t measure)
{
	int32_t temperature;
	temperature = ((measure * pwr->batteryVoltage / VDD_CALIB)
	               - (int32_t) *TEMP30_CAL_ADDR );
	temperature = temperature * (int32_t)(130 - 30);
	temperature = temperature / (int32_t)(*TEMP130_CAL_ADDR -
	                                      *TEMP30_CAL_ADDR);
	temperature = temperature + 30;
	return(temperature);
}

static const ADCConversionGroup adcgrpcfg1 = {
  .circular     = FALSE,
  .num_channels = ADC_GRP1_NUM_CHANNELS,
  .end_cb       = NULL,
  .error_cb     = NULL,
  .cfgr1        = ADC_CFGR1_CONT | ADC_CFGR1_RES_12BIT,
  .cfgr2        = 0,
  .tr           = ADC_TR(0, 0),
  .smpr         = ADC_SMPR_SMP_28P5,
  .chselr       = ADC_CHSELR_CHSEL17 | ADC_CHSELR_CHSEL18
};


THD_FUNCTION(pwrControlThread, arg) {
	power_module_type* pwr = (power_module_type*)arg;
	int i;

	palSetPad(GPIOC, GPIOC_CH_DET);
	palSetPadMode(GPIOC, GPIOC_CH_DET, PAL_MODE_OUTPUT_PUSHPULL);
	
	while (true) {
		chSysLock();
		if (chSemGetCounterI(&sleepDisabled) > 0) chSemWaitTimeoutS(&sleepDisabled, TIME_IMMEDIATE);
		chSysUnlock();

		if (pwr->batteryState <= PWR_BATTERY_LP_SD) {
			palSetPadMode(GPIOC, GPIOC_CH_DET, PAL_MODE_OUTPUT_PUSHPULL);
			palClearPad(GPIOC, GPIOC_CH_DET);
			for (i = 0; i < 10; i++) __NOP();
			
			palSetPadMode(GPIOC, GPIOC_CH_DET, PAL_MODE_INPUT_PULLDOWN);
			for (i = 0; i < 10; i++) __NOP();

			if (palReadPad(GPIOC, GPIOC_CH_DET)) {
				pwr->batteryCharging = TRUE;
				// this will let it turn on the ADC again
				pwr->batteryState = PWR_BATTERY_EMPTY;

				// reenable the output to limit the current through the ESD diode
				palSetPadMode(GPIOC, GPIOC_CH_DET, PAL_MODE_OUTPUT_PUSHPULL);
				palSetPad(GPIOC, GPIOC_CH_DET);
			}
			else {
				// if we're not charging, setting it as a pullup will probably
				// be the most efficient
				palSetPadMode(GPIOC, GPIOC_CH_DET, PAL_MODE_INPUT_PULLUP);
			}
		}
		else {
			chSemSignal(&sleepDisabled);

			adcStart(&ADCD1, NULL);
			adcSTM32EnableTS();
			adcSTM32EnableVREF();
			adcSTM32SetCCR(ADC_CCR_TSEN | ADC_CCR_VREFEN);
			
			osalSysLock();
			osalDbgAssert(ADCD1.thread == NULL, "already waiting");
			adcStartConversionI(&ADCD1, &adcgrpcfg1, pwr->samples, ADC_GRP1_BUF_DEPTH);
			chThdSuspendTimeoutS(&ADCD1.thread, MS2ST(100));
			osalSysUnlock();
			
			adcSTM32DisableVREF();
			adcSTM32DisableTS();
			adcStop(&ADCD1);

			chSysLock();
			if (chSemGetCounterI(&sleepDisabled) > 0) chSemWaitTimeoutS(&sleepDisabled, TIME_IMMEDIATE);
			chSysUnlock();
			
			// prepare for checking charger
			// first drive the pin low so that it's in a known state
			// this should turn off the power switch (there may be a reverse diode though, so this
			// might not work)
#ifndef DEBUG_NO_BATTERY
			palSetPadMode(GPIOC, GPIOC_CH_DET, PAL_MODE_OUTPUT_PUSHPULL);
			palClearPad(GPIOC, GPIOC_CH_DET);
#endif
			
			pwr->batteryVoltage = pwr->voltageInverse / (pwr->samples[0] + pwr->samples[2] + pwr->samples[4] + pwr->samples[6] - pwr->voltageOffset);
			pwr->temperature = ComputeTemperature(pwr, (pwr->samples[1] + pwr->samples[3] + pwr->samples[5] + pwr->samples[7]) >> 2);
			
			// prepare for checking if a charger is attached
			// there should be a small amount of time between this and when the input gets checked
#ifndef DEBUG_NO_BATTERY
			palSetPadMode(GPIOC, GPIOC_CH_DET, PAL_MODE_INPUT_PULLDOWN);
#endif
			
			pwrSetChargeState(pwr);
			
			if (pwr->batteryState == PWR_BATTERY_FULL) {
				// turn off charger
#ifndef DEBUG_NO_BATTERY
				palSetPadMode(GPIOC, GPIOC_CH_DET, PAL_MODE_OUTPUT_PUSHPULL);
				palClearPad(GPIOC, GPIOC_CH_DET);
#endif
			}
			else {
				if (palReadPad(GPIOC, GPIOC_CH_DET)) {
					pwr->batteryCharging = TRUE;
					
					// reenable the output to limit the current through the ESD diode
#ifndef DEBUG_NO_BATTERY
					palSetPadMode(GPIOC, GPIOC_CH_DET, PAL_MODE_OUTPUT_PUSHPULL);
					palSetPad(GPIOC, GPIOC_CH_DET);
#endif
				}
				else {
					pwr->batteryCharging = FALSE;
					
					// if we're not charging, setting it as a pullup will probably
					// be the most efficient
#ifndef DEBUG_NO_BATTERY
					palSetPadMode(GPIOC, GPIOC_CH_DET, PAL_MODE_INPUT_PULLUP);
#endif
				}
			}
		}

		// enable debugging while battery charger is attached
		if (pwr->batteryCharging) {
			// enable debugging in low power modes
			RCC->APB2ENR |= RCC_APB2ENR_DBGEN;
			*(uint32_t*)(0x40015804) = 7;
			PWR->CR |= PWR_CR_FWU;
		}
		else {
			// disable debugging in low power modes to keep the battery from dipping any deeper
			RCC->APB2ENR &= ~RCC_APB2ENR_DBGEN;
			*(uint32_t*)(0x40015804) = 0;
			PWR->CR &= ~PWR_CR_FWU;
		}
		
		if (pwr->batteryState <= PWR_BATTERY_LP_SD) {

			if (!pwr->batteryCharging) {
				// go to deep sleep (i.e. don't even poll until a charger is attached
				chEvtWaitAnyTimeout(ALL_EVENTS, TIME_INFINITE);
			}
			else {
				// poll every 10 seconds instead of the usual
				chEvtWaitAnyTimeout(ALL_EVENTS, MS2ST(5000));
			}
			
		}
		else {
			// enable debugging in low power modes
			RCC->APB2ENR |= RCC_APB2ENR_DBGEN;
			*(uint32_t*)(0x40015804) = 7;
			PWR->CR |= PWR_CR_FWU;

			// on wakeup also waking up the control thread so it can handle the charger change
			chEvtSignalI(&nil.threads[NIL_THREAD_CTRL], CTRL_EVT_WAKEUP);
				
			chThdSleepMilliseconds(5000);
		}



		pwr->cycleCount++;
	}
}
