/**
 ******************************************************************************
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @addtogroup PIOS_EDISON EDISON Functions
 * @brief Hardware functions to deal with the Edison interface
 * @{
 *
 * @file       pios_i2c_uavtalk.h
 * @author     Michael Christen & Team Elevate (UM), based off of
 * 				HMC5883 interface
 * @brief      Receive and Send UAVOBjects over I2C
 * @see        The GNU Public License (GPL) Version 3
 *
 *****************************************************************************/
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

#ifndef PIOS_I2C_UAVTALK_H
#define PIOS_I2C_UAVTALK_H

/* I2C_UAVTALK Addresses */
#define PIOS_I2C_UAVTALK_ADDR 0x00 //TODO: Define addressing scheme
#define PIOS_I2C_UAVTALK_SLAVE MODE 0
#define PIOS_I2C_UAVTALK_MASTER MODE 1
#define PIOS_UAVTALK_COM_TYPE PIOS_I2C_UAVTALK_SLAVE_MODE

/* Global type */
struct pios_i2c_uavtalk_cfg {
#if PIOS_I2C_UAVTALK_COM_TYPE == PIOS_I2C_UAVTALK_SLAVE_MODE
	uint8_t * receive_buffer;
	uint32_t receive_length;
	uint8_t * send_buffer;
	uint32_t send_length;
#elif PIOS_I2C_UAVTALK_COM_TYPE == PIOS_I2C_UAVTALK_MASTER_MODE
#else
#error PIOS_I2C_UAVTALK_COM_TYPE not MASTER or SLAVE
#endif /*PIOS_I2C_UAVTALK_COM_TYPE*/
};

/* Local Types */


/* Global Variables */
/*
#if defined(PIOS_INCLUDE_FREERTOS)
extern xSemaphoreHandle PIOS_BMP085_EOC;
#else
extern int32_t PIOS_BMP085_EOC;
#endif
*/

/* Public Functions */
extern void PIOS_I2C_UAVTALK_Init(void);
//extern void PIOS_I2C_UAVTALK_Read_UAV_Object(void);
//extern void PIOS_I2C_UAVTALK_Send_UAV_Object(void);
//extern void PIOS_I2C_UAVTALK_Bind_Callback(void);

extern int32_t PIOS_I2C_UAVTALK_Write(uint8_t address, uint8_t buffer);

extern int32_t PIOS_I2C_UAVTALK_Read(uint8_t * buffer,
		uint8_t len);
//extern void PIOS_I2C_UAVTALK_Respond(uint8_t * data);

#endif /* PIOS_I2C_UAVTALK_H */

/**
 * @}
 * @}
 */

