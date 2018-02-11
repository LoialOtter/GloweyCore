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

// we have a 512 byte eeprom internal to the chip - might as well use it :)
#include "stdint.h"
#include "stm32l011xx.h"

static void write_unlock(void) {
	FLASH->PEKEYR = 0x89ABCDEF;
	FLASH->PEKEYR = 0x02030405;
}

static void write_lock(void) {
	FLASH->PECR |= FLASH_PECR_PELOCK;
}

void eeprom_read(uint32_t offset, uint32_t length, uint32_t *dest) {
	uint32_t addr = DATA_EEPROM_BASE + (offset<<2);
	if (addr > DATA_EEPROM_END) return;
	uint32_t* data = (uint32_t *)(addr);
	
	for (uint32_t i = 0; i < length; i++) {
		dest[i] = data[i];
	}
}

void eeprom_write(uint32_t offset, uint32_t length, uint32_t *data) {
	uint32_t addr = DATA_EEPROM_BASE + (offset<<2);
	if (addr > DATA_EEPROM_END) return;
	uint32_t* dest = (uint32_t *)(addr);

	write_unlock();
	
	for (uint32_t i = 0; i < length; i++) {
		dest[i] = data[i];
	}

	write_lock();
}
