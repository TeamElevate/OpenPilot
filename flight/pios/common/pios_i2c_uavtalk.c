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
#include <pios_i2c_priv.h>

#ifdef PIOS_INCLUDE_I2C_UAVTALK

void PIOS_I2C_UAVTALK_Init() {
	PIOS_I2C_SetupSlave(PIOS_I2C_MAIN_ADAPTER);
}

int32_t PIOS_I2C_UAVTALK_Write(uint8_t *buffer, uint32_t length) {
    const struct pios_i2c_txn txn_list[] = {
        {
            .info = __func__,
            .addr = PIOS_I2C_UAVTALK_ADDR,
            .rw   = PIOS_I2C_TXN_WRITE,
            .len  = length,
            .buf  = buffer,
        }
        ,
    };

    return PIOS_I2C_Transfer(PIOS_I2C_MAIN_ADAPTER, txn_list, NELEMENTS(txn_list));
}

int32_t PIOS_I2C_UAVTALK_Read(uint8_t * buffer, uint32_t len, uint32_t timeout) {
	 //*buffer = *(PIOS_I2C_GetLastSlaveTxn(PIOS_I2C_MAIN_ADAPTER).buf);
    struct pios_i2c_adapter *i2c_adapter;
    i2c_adapter = (struct pios_i2c_adapter *)PIOS_I2C_MAIN_ADAPTER;
	struct pios_i2c_txn *rx_txn;
	uint32_t rd_len = 0;
	bool read = 0;
	//Look at front, and only remove when have read entire contents
	while(xQueuePeek(i2c_adapter->i2cRxTxnQueue, &rx_txn, timeout) ==
			pdTRUE) {
		read = 1;
		while(rx_txn->rd_idx < rx_txn->len &&
				rd_len < len) {
			buffer[rd_len++] = rx_txn->buf[rx_txn->rd_idx++];
		}
		if(rx_txn->rd_idx == rx_txn->len) {
			pios_free(rx_txn->buf);
			pios_free(rx_txn);
			if(xQueueReceive(i2c_adapter->i2cRxTxnQueue, &rx_txn, 0)
					!= pdTRUE) {
				//YOU SHOULD NOT BE HERE
				//This removes from the final queue, and we 
				//know there is something on the front since we peeked
			}
		}
		//Done reading
		if(rd_len == len) {
			break;
		}
	}
	return read ? (int32_t)rd_len : -1;
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
