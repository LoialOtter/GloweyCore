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
#include "mma8653fc.h"
#include "string.h"
#include "board.h"
#include "sleep.h"



I2CConfig accel_i2c_conf = {
	.timingr = STM32_TIMINGR_PRESC(1U) | STM32_TIMINGR_SCLL(0x09U) | STM32_TIMINGR_SCLH(0x03U) | STM32_TIMINGR_SDADEL(2U) | STM32_TIMINGR_SCLDEL(3U),
	.cr1 = 0,
	.cr2 = 0
};

void I2C_Enable(void) {
	chSemSignal(&sleepDisabled);
	
	i2cStart(&I2CD1, &accel_i2c_conf);
	palSetPadMode(GPIOB, GPIOB_I2C_SCL, PAL_MODE_ALTERNATE(1));
	palSetPadMode(GPIOB, GPIOB_I2C_SDA, PAL_MODE_ALTERNATE(1));
	// Fix the IO
	GPIOB->PUPDR &= ~((3 << (GPIOB_I2C_SDA << 1)) | (3 << (GPIOB_I2C_SCL << 1)));
	GPIOB->OTYPER |= (1 << GPIOB_I2C_SDA) | (1 << GPIOB_I2C_SCL);
}
void I2C_Shutdown(void) {
	palSetPadMode(GPIOB, GPIOB_I2C_SCL, PAL_MODE_INPUT_PULLUP);
	palSetPadMode(GPIOB, GPIOB_I2C_SDA, PAL_MODE_INPUT_PULLUP);
	i2cStop(&I2CD1);
	chSysLock();
	if (chSemGetCounterI(&sleepDisabled) > 0) chSemWaitTimeoutS(&sleepDisabled, TIME_IMMEDIATE);
	chSysUnlock();
}


msg_t I2C_WriteRegister(uint8_t address, uint8_t regAddress, uint8_t value) {
	uint8_t cmdBuffer[2];
	cmdBuffer[0] = regAddress;
	cmdBuffer[1] = value;
	return i2cMasterTransmitTimeout(&I2CD1, address, cmdBuffer, 2, NULL, 0, MS2ST(20));
}

msg_t I2C_ReadMultiRegisters(uint8_t address, uint8_t startAddress, uint8_t length, uint8_t* buffer) {
	uint8_t cmdBuffer[1];
	cmdBuffer[0] = startAddress;
	return i2cMasterTransmitTimeout(&I2CD1, address, cmdBuffer, 1, buffer, length, MS2ST(20));
}
	

void accel_config(const accel_settings_type* settings) {
	I2C_Enable();
	I2C_WriteRegister(MMA8653FC_ADDRESS, MMA8653FC_REG_0E_XYZ_DATA_CFG, settings->xyz_data_cfg);
	I2C_WriteRegister(MMA8653FC_ADDRESS, MMA8653FC_REG_2E_CTRL_REG5, settings->ctrl_reg5);
	I2C_WriteRegister(MMA8653FC_ADDRESS, MMA8653FC_REG_2D_CTRL_REG4, settings->ctrl_reg4);
	I2C_WriteRegister(MMA8653FC_ADDRESS, MMA8653FC_REG_2C_CTRL_REG3, settings->ctrl_reg3);
	I2C_WriteRegister(MMA8653FC_ADDRESS, MMA8653FC_REG_2B_CTRL_REG2, settings->ctrl_reg2);
	I2C_WriteRegister(MMA8653FC_ADDRESS, MMA8653FC_REG_2A_CTRL_REG1, settings->ctrl_reg1);
	I2C_Shutdown();
}

void accel_calibrate(void) {
	I2C_Enable();
	I2C_WriteRegister(MMA8653FC_ADDRESS, MMA8653FC_REG_2B_CTRL_REG2, MMA8653FC_CTRL_REG2_RST);
	I2C_Shutdown();
	chThdSleepMilliseconds(1);
}



static uint8_t buffer[0x40];
void get_accel(int16_t* x, int16_t* y, int16_t* z) {
	memset(buffer, 0x55, 0x40);

	I2C_Enable();
	// get x/y/z values and status
	I2C_ReadMultiRegisters(MMA8653FC_ADDRESS, 0x00, 7, &buffer[0x00]);
	//I2C_ReadMultiRegisters(MMA8653FC_ADDRESS, 0x0B, 15, &buffer[0x0B]);
	//I2C_ReadMultiRegisters(MMA8653FC_ADDRESS, 0x29,  9, &buffer[0x29]);
	I2C_ReadMultiRegisters(MMA8653FC_ADDRESS, MMA8653FC_REG_2A_CTRL_REG1, 12, &buffer[10]);
	// get interrupt source
	I2C_Shutdown();

	*x = (buffer[1] << 8 | buffer[2]);
	*y = (buffer[3] << 8 | buffer[4]);
	*z = (buffer[5] << 8 | buffer[6]);
}
