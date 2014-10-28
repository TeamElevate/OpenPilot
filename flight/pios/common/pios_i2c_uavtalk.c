/**
 ******************************************************************************
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @addtogroup PIOS_EDISON EDISON Functions
 * @brief Hardware functions to deal with the Edison interface
 * @{
 *
 * @file       pios_i2c_uavtalk.c
 * @author     Michael Christen & Team Elevate (UM), based off of
 * 				HMC5883 interface
 * @brief      Receive and Send UAVOBjects over I2C
 * @see        The GNU Public License (GPL) Version 3
 *
 ******************************************************************************/
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#include "pios.h"

#ifdef PIOS_INCLUDE_I2C_UAVTALK

void PIOS_I2C_UAVTALK_Init() {
	PIOS_I2C_SetupSlave(PIOS_I2C_MAIN_ADAPTER);
}

int32_t PIOS_I2C_UAVTALK_Write(uint8_t address, uint8_t buffer) {

	uint8_t data[] = {
        address,
        buffer,
    };

    const struct pios_i2c_txn txn_list[] = {
        {
            .info = __func__,
            .addr = PIOS_I2C_UAVTALK_ADDR,
            .rw   = PIOS_I2C_TXN_WRITE,
            .len  = sizeof(data),
            .buf  = data,
        }
        ,
    };

    return PIOS_I2C_Transfer(PIOS_I2C_MAIN_ADAPTER, txn_list, NELEMENTS(txn_list));
}

int32_t PIOS_I2C_UAVTALK_Read(uint8_t * buffer, uint8_t len) {
	 //*buffer = *(PIOS_I2C_GetLastSlaveTxn(PIOS_I2C_MAIN_ADAPTER).buf);
	return *buffer + len;
	/*
	const struct pios_i2c_txn txn_list[] = {
		{
			.info = __func__,
			.addr = PIOS_I2C_UAVTALK_ADDR,
			.rw   = PIOS_I2C_TXN_READ,
			.len  = len,
			.buf  = buffer,
		}
	};
	return PIOS_I2C_Transfer(PIOS_I2C_MAIN_ADAPTER, txn_list,
			NELEMENTS(txn_list));
			*/
}
/*

void PIOS_I2C_UAVTALK_Respond(uint8_t * data) {
	struct pios_i2c_txn i2c_response[] = {
		{
			.info = __func__,
			.addr = PIOS_I2C_UAVTALK_ADDR,
			.rw   = PIOS_I2C_TXN_WRITE,
			.len  = sizeof(data),
			.buf  = data
		}
		,
	};
	PIOS_I2C_LoadSlaveResponse(PIOS_I2C_FLEXI_ADAPTER,
			i2c_response);
}
*/

#ifndef PIOS_INCLUDE_EXTI
#error PIOS_EXTI must be included in the project
#endif /* PIOS_INCLUDE_EXTI */
#endif /*PIOS_INCLUDE_I2C_UAVTALK*/
