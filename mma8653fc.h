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


#ifndef __MMA8653FC_H__
#define __MMA8653FC_H__

typedef struct {
  uint8_t ctrl_reg1, ctrl_reg2, ctrl_reg3, ctrl_reg4, ctrl_reg5, xyz_data_cfg;
} accel_settings_type;



#define MMA8653FC_ADDRESS               0x1D
#define MMA8653FC_ADDRESS_READ          0x3B
#define MMA8653FC_ADDRESS_WRITE         0x3A


#define MMA8653FC_REG_00_STATUS            0x00
#define MMA8653FC_REG_01_OUT_X_MSB         0x01
#define MMA8653FC_REG_02_OUT_X_LSB         0x02
#define MMA8653FC_REG_03_OUT_Y_MSB         0x03
#define MMA8653FC_REG_04_OUT_Y_LSB         0x04
#define MMA8653FC_REG_05_OUT_Z_MSB         0x05
#define MMA8653FC_REG_06_OUT_Z_LSB         0x06

#define MMA8653FC_REG_0B_SYSMOD            0x0B
#define MMA8653FC_REG_0C_INT_SOURCE        0x0C
#define MMA8653FC_REG_0D_WHO_AM_I          0x0D
#define MMA8653FC_REG_0E_XYZ_DATA_CFG      0x0E

#define MMA8653FC_REG_10_PL_STATUS         0x10
#define MMA8653FC_REG_11_PL_CFG            0x11
#define MMA8653FC_REG_12_PL_COUNT          0x12
#define MMA8653FC_REG_13_PL_BF_ZCOMP       0x13
#define MMA8653FC_REG_14_PL_THS_REG        0x14

#define MMA8653FC_REG_15_FF_MT_CFG         0x15
#define MMA8653FC_REG_16_FF_MT_SRC         0x16
#define MMA8653FC_REG_17_FF_MT_THS         0x17
#define MMA8653FC_REG_18_FF_MT_COUNT       0x18

#define MMA8653FC_REG_29_ASLP_COUNT        0x29
#define MMA8653FC_REG_2A_CTRL_REG1         0x2A
#define MMA8653FC_REG_2B_CTRL_REG2         0x2B
#define MMA8653FC_REG_2C_CTRL_REG3         0x2C
#define MMA8653FC_REG_2D_CTRL_REG4         0x2D
#define MMA8653FC_REG_2E_CTRL_REG5         0x2E
#define MMA8653FC_REG_2F_OFF_X             0x2F
#define MMA8653FC_REG_30_OFF_Y             0x30
#define MMA8653FC_REG_31_OFF_Z             0x31

/* STATUS */
#define MMA8653FC_STATUS_ZYXOW       0x80
#define MMA8653FC_STATUS_ZOW         0x40
#define MMA8653FC_STATUS_YOW         0x20
#define MMA8653FC_STATUS_XOW         0x10
#define MMA8653FC_STATUS_ZYXDR       0x08
#define MMA8653FC_STATUS_ZDR         0x04
#define MMA8653FC_STATUS_YDR         0x02
#define MMA8653FC_STATUS_XDR         0x01

/* SYSMOD */
#define MMA8653FC_SYSMOD_MASK            0x03
#define MMA8653FC_SYSMOD_STANDBY         0x00
#define MMA8653FC_SYSMOD_WAKE            0x01
#define MMA8653FC_SYSMOD_SLEEP           0x02

/* INT_SOURCE */
#define MMA8653FC_INT_SOURCE_ASLP        0x80 /* auto-sleep interrupt */
#define MMA8653FC_INT_SOURCE_LNDPRT      0x10 /* landscape/portrait orientation intterupt */
#define MMA8653FC_INT_SOURCE_FF_MT       0x04 /* freefall/motion interrupt */
#define MMA8653FC_INT_SOURCE_DRDY        0x01 /* data ready interrupt */

/* XYZ_DATA_CFG */
#define MMA8653FC_XYZ_DATA_CFG_MASK      0x03
#define MMA8653FC_XYZ_DATA_CFG_2G        0x00
#define MMA8653FC_XYZ_DATA_CFG_4G        0x01
#define MMA8653FC_XYZ_DATA_CFG_8G        0x02


/* Freefall/Motion */
#define MMA8653FC_FF_MT_CFG_ELE   0x80 /* Event Latch Enable */
#define MMA8653FC_FF_MT_CFG_OAE   0x40 /* 0=Freefall 1=Motion */
#define MMA8653FC_FF_MT_CFG_ZEFE  0x20 /* Z-Event flag enable */
#define MMA8653FC_FF_MT_CFG_YEFE  0x10 /* Y-Event flag enable */
#define MMA8653FC_FF_MT_CFG_XEFE  0x08 /* X-Event flag enable */

#define MMA8653FC_FF_MT_SRC_EA    0x80 /* Event Active Flag */
#define MMA8653FC_FF_MT_SRC_ZHE   0x20 /* Z Motion Flag */
#define MMA8653FC_FF_MT_SRC_ZHP   0x10 /* Z Motion going negative */
#define MMA8653FC_FF_MT_SRC_YHE   0x08 /* Y Motion Flag */
#define MMA8653FC_FF_MT_SRC_YHP   0x04 /* Y Motion going negative */
#define MMA8653FC_FF_MT_SRC_XHE   0x02 /* X Motion Flag */
#define MMA8653FC_FF_MT_SRC_XHP   0x01 /* X Motion going negative */


#define MMA8653FC_FF_MT_THS_DBCNTM    0x80 /* debounce counter mode: 0=inc/dec debounce, 1=inc/clear debounce */
#define MMA8653FC_FF_MT_THS_THRESH(x) (x & 0x7F) /* threshhold */
#define MMA8653FC_FF_MT_COUNT(x)      (x) /* debounce count */


/* CTRL_REG1 */
#define MMA8653FC_CTRL_REG1_ASLP_RATE_MASK   0xC0
#define MMA8653FC_CTRL_REG1_ASLP_RATE_50Hz   0x00
#define MMA8653FC_CTRL_REG1_ASLP_RATE_12_5Hz 0x40
#define MMA8653FC_CTRL_REG1_ASLP_RATE_6_25Hz 0x80
#define MMA8653FC_CTRL_REG1_ASLP_RATE_1_56Hz 0xC0

#define MMA8653FC_CTRL_REG1_DR_MASK   0x38
#define MMA8653FC_CTRL_REG1_DR_800Hz  0x00
#define MMA8653FC_CTRL_REG1_DR_400Hz  0x08
#define MMA8653FC_CTRL_REG1_DR_200Hz  0x10
#define MMA8653FC_CTRL_REG1_DR_100Hz  0x18
#define MMA8653FC_CTRL_REG1_DR_50Hz   0x20
#define MMA8653FC_CTRL_REG1_DR_12_5Hz 0x28
#define MMA8653FC_CTRL_REG1_DR_6_25Hz 0x30
#define MMA8653FC_CTRL_REG1_DR_1_56Hz 0x38

#define MMA8653FC_CTRL_REG1_F_READ 0x02 /* data format limited to single byte */
#define MMA8653FC_CTRL_REG1_ACTIVE 0x01 /* Standby/Active */

/* CTRL_REG2 */
#define MMA8653FC_CTRL_REG2_ST          0x80 /* Self Test */
#define MMA8653FC_CTRL_REG2_RST         0x40 /* Software Reset */
#define MMA8653FC_CTRL_REG2_SLPE        0x04 /* Auto-sleep enable */

// oversample mode - sleep
#define MMA8653FC_CTRL_REG2_SMODS_MASK     0x18
#define MMA8653FC_CTRL_REG2_SMODS_NORMAL   0x00 /* @1.56Hz 27uA  */
#define MMA8653FC_CTRL_REG2_SMODS_LOWNOISE 0x08 /* @1.56Hz 9uA   */
#define MMA8653FC_CTRL_REG2_SMODS_HIGHRES  0x10 /* @1.56Hz 184uA */
#define MMA8653FC_CTRL_REG2_SMODS_LOWPWR   0x18 /* @1.56Hz 6.5uA */

// oversample mode - awake
#define MMA8653FC_CTRL_REG2_MODS_MASK      0x03
#define MMA8653FC_CTRL_REG2_MODS_NORMAL    0x00
#define MMA8653FC_CTRL_REG2_MODS_LOWNOISE  0x01
#define MMA8653FC_CTRL_REG2_MODS_HIGHRES   0x02
#define MMA8653FC_CTRL_REG2_MODS_LOWPWR    0x03


/* CTRL_REG3 */
#define MMA8653FC_CTRL_REG3_WAKE_GATE    0x80 /* FIFO can wake */
#define MMA8653FC_CTRL_REG3_WAKE_TRANS   0x40 /* Transient interrupt */
#define MMA8653FC_CTRL_REG3_WAKE_LNDPRT  0x20 /* Orientation can wake */
#define MMA8653FC_CTRL_REG3_WAKE_PULSE   0x10 /* Pulse can wake */
#define MMA8653FC_CTRL_REG3_WAKE_FF_MT   0x08 /* Freefall/Motion can wake */
#define MMA8653FC_CTRL_REG3_IPOL         0x02 /* Interrupt Polarity */
#define MMA8653FC_CTRL_REG3_PP_OD        0x01 /* output type: 0=push-pull 1=open-drain */

/* CTRL_REG4 */
#define MMA8653FC_CTRL_REG4_INT_EN_ASLP   0x80 /* auto-sleep/wake interrupt enable */
#define MMA8653FC_CTRL_REG4_INT_EN_LNDPRT 0x10 /* orientation interrupt enable */
#define MMA8653FC_CTRL_REG4_INT_EN_FF_MT  0x04 /* freefall/motion interrupt enable */
#define MMA8653FC_CTRL_REG4_INT_EN_DRDY   0x01 /* data ready interrupt enable */

/* CTRL_REG5 */
#define MMA8653FC_CTRL_REG5_INT_CFG_ASLP_INT1   0x80
#define MMA8653FC_CTRL_REG5_INT_CFG_ASLP_INT2   0x00
#define MMA8653FC_CTRL_REG5_INT_CFG_LNDPRT_INT1 0x10
#define MMA8653FC_CTRL_REG5_INT_CFG_LNDPRT_INT2 0x00
#define MMA8653FC_CTRL_REG5_INT_CFG_FF_MT_INT1  0x04
#define MMA8653FC_CTRL_REG5_INT_CFG_FF_MT_INT2  0x00
#define MMA8653FC_CTRL_REG5_INT_CFG_DRDY_INT1   0x01
#define MMA8653FC_CTRL_REG5_INT_CFG_DRDY_INT2   0x00

void accel_config(const accel_settings_type* settings);
void accel_calibrate(void);
void get_accel(int16_t* x, int16_t* y, int16_t* z);


#endif /* __MMA8653FC_H__ */
