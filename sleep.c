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


#include "hal.h"
#include "stm32_rcc.h"
#include "ch.h"
#include "sleep.h"

SEMAPHORE_DECL(sleepDisabled, 0);



void enter_LPSleep(void) {
	if (chSemGetCounterI(&sleepDisabled) > 0) return;

	//if (RCC->APB2ENR & RCC_APB2ENR_ADCEN) return;
	//if (RCC->APB1ENR & RCC_APB1ENR_I2C1EN) return;

	if (LPTIM1->IER == 0) {
		while (1);
	}

	RCC->CFGR &= ~3;        /* Switches on the selected clock source.   */
	while (RCC->CFGR & RCC_CFGR_SWS);
	RCC->CR &= ~RCC_CR_HSION;
	
	// Clear PDDS and LPDS bits
	PWR->CR &= ~PWR_CR_PDDS;
	PWR->CR &= ~PWR_CR_LPDS;
	
	// Clear previous wakeup register
	PWR->CR |= PWR_CR_CWUF;
	
	//if (mode == STANDBY) {
	//PWR->CR |= PWR_CR_PDDS;
	//}
	
	PWR->CR |= PWR_CR_LPDS;

	// turn off flash
	//FLASH->ACR |= FLASH_ACR_SLEEP_PD;
	PWR->CR &= ~PWR_CR_FWU;

    
	//SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;
    PWR->CR |= PWR_CR_LPSDSR; // voltage regulator in low-power mode during sleep
    /* 5. Follow the steps described in Section 6.3.5: Entering low-power mode */
    //SCB->SCR &= ~( SCB_SCR_SLEEPDEEP_Msk ); // low-power mode = sleep mode
    SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;
    //SCB->SCR |= SCB_SCR_SLEEPONEXIT_Msk; // reenter low-power mode after ISR
    __WFE(); // enter low-power mode
    // Now go into stop mode, wake up on interrupt
    //asm("    wfe");
    
    exit_LPSleep();
}

void exit_LPSleep(void) {
#if (STM32_SW != STM32_SW_MSI)
	RCC->CR |= RCC_CR_HSION;
	RCC->CFGR |= STM32_SW;        /* Switches on the selected clock source.   */
	while ((RCC->CFGR & RCC_CFGR_SWS) != (STM32_SW << 2))
		;
#endif
    SCB->SCR &= ~SCB_SCR_SLEEPDEEP_Msk;
}
