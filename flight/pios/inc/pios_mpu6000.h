/**
 ******************************************************************************
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @addtogroup PIOS_MPU6000 MPU6000 Functions
 * @brief Deals with the hardware interface to the 3-axis gyro
 * @{
 *
 * @file       PIOS_MPU6000.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2012.
 * @brief      MPU6000 3-axis gyro function headers
 * @see        The GNU Public License (GPL) Version 3
 *
 ******************************************************************************
 */
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

#ifndef PIOS_MPU6000_H
#define PIOS_MPU6000_H

/* MPU6000 Addresses */
#define PIOS_MPU6000_SMPLRT_DIV_REG           0X19
#define PIOS_MPU6000_DLPF_CFG_REG             0X1A
#define PIOS_MPU6000_GYRO_CFG_REG             0X1B
#define PIOS_MPU6000_ACCEL_CFG_REG            0X1C
#define PIOS_MPU6000_FIFO_EN_REG              0x23
#define PIOS_MPU6000_INT_CFG_REG              0x37
#define PIOS_MPU6000_INT_EN_REG               0x38
#define PIOS_MPU6000_INT_STATUS_REG           0x3A
#define PIOS_MPU6000_ACCEL_X_OUT_MSB          0x3B
#define PIOS_MPU6000_ACCEL_X_OUT_LSB          0x3C
#define PIOS_MPU6000_ACCEL_Y_OUT_MSB          0x3D
#define PIOS_MPU6000_ACCEL_Y_OUT_LSB          0x3E
#define PIOS_MPU6000_ACCEL_Z_OUT_MSB          0x3F
#define PIOS_MPU6000_ACCEL_Z_OUT_LSB          0x40
#define PIOS_MPU6000_TEMP_OUT_MSB             0x41
#define PIOS_MPU6000_TEMP_OUT_LSB             0x42
#define PIOS_MPU6000_GYRO_X_OUT_MSB           0x43
#define PIOS_MPU6000_GYRO_X_OUT_LSB           0x44
#define PIOS_MPU6000_GYRO_Y_OUT_MSB           0x45
#define PIOS_MPU6000_GYRO_Y_OUT_LSB           0x46
#define PIOS_MPU6000_GYRO_Z_OUT_MSB           0x47
#define PIOS_MPU6000_GYRO_Z_OUT_LSB           0x48
#define PIOS_MPU6000_USER_CTRL_REG            0x6A
#define PIOS_MPU6000_PWR_MGMT_REG             0x6B
#define PIOS_MPU6000_FIFO_CNT_MSB             0x72
#define PIOS_MPU6000_FIFO_CNT_LSB             0x73
#define PIOS_MPU6000_FIFO_REG                 0x74
#define PIOS_MPU6000_WHOAMI                   0x75

/* FIFO enable for storing different values */
#define PIOS_MPU6000_FIFO_TEMP_OUT            0x80
#define PIOS_MPU6000_FIFO_GYRO_X_OUT          0x40
#define PIOS_MPU6000_FIFO_GYRO_Y_OUT          0x20
#define PIOS_MPU6000_FIFO_GYRO_Z_OUT          0x10
#define PIOS_MPU6000_ACCEL_OUT                0x08

/* Interrupt Configuration */
#define PIOS_MPU6000_INT_ACTL                 0x80
#define PIOS_MPU6000_INT_OPEN                 0x40
#define PIOS_MPU6000_INT_LATCH_EN             0x20
#define PIOS_MPU6000_INT_CLR_ANYRD            0x10

#define PIOS_MPU6000_INTEN_OVERFLOW           0x10
#define PIOS_MPU6000_INTEN_DATA_RDY           0x01

/* Interrupt status */
#define PIOS_MPU6000_INT_STATUS_FIFO_FULL     0x80
#define PIOS_MPU6000_INT_STATUS_FIFO_OVERFLOW 0x10
#define PIOS_MPU6000_INT_STATUS_IMU_RDY       0X04
#define PIOS_MPU6000_INT_STATUS_DATA_RDY      0X01

/* User control functionality */
#define PIOS_MPU6000_USERCTL_FIFO_EN          0X40
#define PIOS_MPU6000_USERCTL_I2C_MST_EN       0x20
#define PIOS_MPU6000_USERCTL_DIS_I2C          0X10
#define PIOS_MPU6000_USERCTL_FIFO_RST         0X04
#define PIOS_MPU6000_USERCTL_SIG_COND         0X02
#define PIOS_MPU6000_USERCTL_GYRO_RST         0X01

/* Power management and clock selection */
#define PIOS_MPU6000_PWRMGMT_IMU_RST          0X80
#define PIOS_MPU6000_PWRMGMT_INTERN_CLK       0X00
#define PIOS_MPU6000_PWRMGMT_PLL_X_CLK        0X01
#define PIOS_MPU6000_PWRMGMT_PLL_Y_CLK        0X02
#define PIOS_MPU6000_PWRMGMT_PLL_Z_CLK        0X03
#define PIOS_MPU6000_PWRMGMT_STOP_CLK         0X07

/* Master I2C Communication on MPU6000 */
#if defined(PIOS_MPU6000_AUXI2C)

#define PIOS_MPU6000_I2C_MASTER_CTRL         0x24
	#define MPU6000_MST_CTRL_MULT_MST_EN     0x7
	#define MPU6000_MST_CTRL_WAIT_FOR_ES     0x6
	#define MPU6000_MST_CTRL_SLV3_FIFO_EN    0x5
	#define MPU6000_MST_CTRL_MST_P_NSR       0x4
	//Rest is clock
    #define MPU6000_MST_CLK_400KHZ           0xD

#define PIOS_MPU6000_I2C_MASTER_DELAY_CTRL   0x67
#define PIOS_MPU6000_I2C_MASTER_STATUS       0x36

/*
uint8_t PIOS_MPU6000_SLAVE_ADDR[] = {
	0x25,
	0x28,
	0x2B,
	0x2E,
};
*/

#define PIOS_MPU6000_I2C_SLAVE_ADDR_0        0x25
#define PIOS_MPU6000_I2C_SLAVE_REG_0         0x26
#define PIOS_MPU6000_I2C_SLAVE_CTRL_0        0x27
	#define MPU6000_SLV_CTRL_EN              0x7
	#define MPU6000_SLV_BYTE_SW              0x6
	#define MPU6000_SLV_REG_DIS              0x5
	#define MPU6000_SLV_ORD_GRP              0x4
	
#define PIOS_MPU6000_I2C_SLAVE_DO_0          0x63
#define PIOS_MPU6000_I2C_SLAVE_ADDR_1        0x28
#define PIOS_MPU6000_I2C_SLAVE_REG_1         0x29
#define PIOS_MPU6000_I2C_SLAVE_CTRL_1        0x2A
#define PIOS_MPU6000_I2C_SLAVE_DO_1          0x64
#define PIOS_MPU6000_I2C_SLAVE_ADDR_2        0x2B
#define PIOS_MPU6000_I2C_SLAVE_REG_2         0x2C
#define PIOS_MPU6000_I2C_SLAVE_CTRL_2        0x2D
#define PIOS_MPU6000_I2C_SLAVE_DO_2          0x65
#define PIOS_MPU6000_I2C_SLAVE_ADDR_3        0x2E
#define PIOS_MPU6000_I2C_SLAVE_REG_3         0x2F
#define PIOS_MPU6000_I2C_SLAVE_CTRL_3        0x30
#define PIOS_MPU6000_I2C_SLAVE_DO_3          0x66

#define PIOS_MPU6000_I2C_SLAVE_ADDR_OFF      0x0
#define PIOS_MPU6000_I2C_SLAVE_REG_OFF       0x1
#define PIOS_MPU6000_I2C_SLAVE_CTRL_OFF      0x2

#define PIOS_MPU6000_EXT_SENSE_BASE          0x49
//Actual last byte
#define PIOS_MPU6000_EXT_SENSE_END           0x60

#define PIOS_MPU6000_EXT_SENSE_SIZE (1 + PIOS_MPU6000_EXT_SENSE_END - PIOS_MPU6000_EXT_SENSE_BASE)

#define PIOS_MPU6000_

#endif /* PIOS_MPU6000_AUXI2C */

enum pios_mpu6000_range {
    PIOS_MPU6000_SCALE_250_DEG  = 0x00,
    PIOS_MPU6000_SCALE_500_DEG  = 0x08,
    PIOS_MPU6000_SCALE_1000_DEG = 0x10,
    PIOS_MPU6000_SCALE_2000_DEG = 0x18
};

enum pios_mpu6000_filter {
    PIOS_MPU6000_LOWPASS_256_HZ = 0x00,
    PIOS_MPU6000_LOWPASS_188_HZ = 0x01,
    PIOS_MPU6000_LOWPASS_98_HZ  = 0x02,
    PIOS_MPU6000_LOWPASS_42_HZ  = 0x03,
    PIOS_MPU6000_LOWPASS_20_HZ  = 0x04,
    PIOS_MPU6000_LOWPASS_10_HZ  = 0x05,
    PIOS_MPU6000_LOWPASS_5_HZ   = 0x06
};

enum pios_mpu6000_accel_range {
    PIOS_MPU6000_ACCEL_2G  = 0x00,
    PIOS_MPU6000_ACCEL_4G  = 0x08,
    PIOS_MPU6000_ACCEL_8G  = 0x10,
    PIOS_MPU6000_ACCEL_16G = 0x18
};

enum pios_mpu6000_orientation { // clockwise rotation from board forward
    PIOS_MPU6000_TOP_0DEG   = 0x00,
    PIOS_MPU6000_TOP_90DEG  = 0x01,
    PIOS_MPU6000_TOP_180DEG = 0x02,
    PIOS_MPU6000_TOP_270DEG = 0x03
};

struct pios_mpu6000_data {
    int16_t gyro_x;
    int16_t gyro_y;
    int16_t gyro_z;
#if defined(PIOS_MPU6000_ACCEL)
    int16_t accel_x;
    int16_t accel_y;
    int16_t accel_z;
#endif /* PIOS_MPU6000_ACCEL */

#if defined(PIOS_MPU6000_AUXI2C)
#endif /* PIOS_MPU6000_AUXI2C */
    int16_t temperature;
};

struct pios_mpu6000_cfg {
    const struct pios_exti_cfg *exti_cfg; /* Pointer to the EXTI configuration */

    uint8_t Fifo_store; /* FIFO storage of different readings (See datasheet page 31 for more details) */

    /* Sample rate divider to use (See datasheet page 32 for more details).*/
    uint8_t Smpl_rate_div_no_dlp; /* used when no dlp is applied (fs=8KHz)*/
    uint8_t Smpl_rate_div_dlp; /* used when dlp is on (fs=1kHz)*/
    uint8_t interrupt_cfg; /* Interrupt configuration (See datasheet page 35 for more details) */
    uint8_t interrupt_en; /* Interrupt configuration (See datasheet page 35 for more details) */
    uint8_t User_ctl; /* User control settings (See datasheet page 41 for more details)  */
    uint8_t Pwr_mgmt_clk; /* Power management and clock selection (See datasheet page 32 for more details) */
    enum pios_mpu6000_accel_range accel_range;
    enum pios_mpu6000_range gyro_range;
    enum pios_mpu6000_filter filter;
    enum pios_mpu6000_orientation orientation;
};

/* Public Functions */
extern int32_t PIOS_MPU6000_Init(uint32_t spi_id, uint32_t slave_num, const struct pios_mpu6000_cfg *new_cfg);
extern int32_t PIOS_MPU6000_ConfigureRanges(enum pios_mpu6000_range gyroRange, enum pios_mpu6000_accel_range accelRange, enum pios_mpu6000_filter filterSetting);
extern xQueueHandle PIOS_MPU6000_GetQueue();
extern int32_t PIOS_MPU6000_ReadGyros(struct pios_mpu6000_data *buffer);
extern int32_t PIOS_MPU6000_ReadID();
extern int32_t PIOS_MPU6000_Test();
extern float PIOS_MPU6000_GetScale();
extern float PIOS_MPU6000_GetAccelScale();
extern bool PIOS_MPU6000_IRQHandler(void);

#if defined(PIOS_MPU6000_AUXI2C)
#define MAX_PIOS_MPU6000_I2C_SLAVES 4 //0,1,2,3 - ignoring 4

struct pios_mpu6000_i2c_slave_cfg {
	uint8_t addr;
	uint8_t reg;
	bool using_reg;

	uint8_t slave_num;
};


extern int32_t PIOS_MPU6000_I2C_Init(struct pios_mpu6000_i2c_slave_cfg *cfg);

extern void PIOS_MPU6000_I2C_SLV_Stop(struct pios_mpu6000_i2c_slave_cfg *cfg);

//Read len bytes from MPU6000 aux device specified by addr,  
//if reg != 0 read from that reg from device, read into buf,
// return length read, -1 if err
extern int32_t PIOS_MPU6000_I2C_Read(struct pios_mpu6000_i2c_slave_cfg *cfg, 
		uint8_t len, uint8_t * buf, bool inTask);

//Write byte to MPU6000 aux device specified by addr,  
//if reg != 0 write to that reg from device,
// return non0 if err
extern int32_t PIOS_MPU6000_I2C_Write_Byte(struct pios_mpu6000_i2c_slave_cfg *cfg,
		uint8_t byte, bool inTask);

#endif /* PIOS_MPU6000_AUXI2C */

#endif /* PIOS_MPU6000_H */

/**
 * @}
 * @}
 */
