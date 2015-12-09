/* Open Sensor Platform Project
 * https://github.com/sensorplatforms/open-sensor-platform
 *
 * Copyright (C) 2013 Sensor Platforms Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
/*-------------------------------------------------------------------------------------------------*\
 |    I N C L U D E   F I L E S
\*-------------------------------------------------------------------------------------------------*/
#include "Common.h"
#include "Driver_I2C.h"
#include "Bosch_I2C_Adapter.h"

#ifndef I2C_DRIVER
# error Needs I2C_DRIVER to be defined. Check Common.h
#endif

/*-------------------------------------------------------------------------------------------------*\
 |    E X T E R N A L   V A R I A B L E S   &   F U N C T I O N S
\*-------------------------------------------------------------------------------------------------*/
extern ARM_DRIVER_I2C Driver_I2C0;

/*-------------------------------------------------------------------------------------------------*\
 |    P U B L I C   V A R I A B L E S   D E F I N I T I O N S
\*-------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------*\
 |    P R I V A T E   C O N S T A N T S   &   M A C R O S
\*-------------------------------------------------------------------------------------------------*/
/* This define is used for writing the registers in the sensors, this can be increased based on the need */
#define MAX_REG_WRITE_SIZE          (64)
#define I2C_SEND_STOP_BITS          (0)
#define I2C_DO_NOT_SEND_STOP_BITS   (1)

/*-------------------------------------------------------------------------------------------------*\
 |    P R I V A T E   T Y P E   D E F I N I T I O N S
\*-------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------*\
 |    S T A T I C   V A R I A B L E S   D E F I N I T I O N S
\*-------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------*\
 |    F O R W A R D   F U N C T I O N   D E C L A R A T I O N S
\*-------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------*\
 |    P R I V A T E     F U N C T I O N S
\*-------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------*\
 |    P U B L I C     F U N C T I O N S
\*-------------------------------------------------------------------------------------------------*/

/****************************************************************************************************
 * @fn      dev_i2c_delay
 *          adaptation of the Bosch API functions to the BOARD specific function
 *
 ***************************************************************************************************/
void dev_i2c_delay(unsigned int msec)
{
    os_dly_wait(MSEC_TO_TICS(msec));
}


/****************************************************************************************************
 * @fn      dev_i2c_write
 *          adaptation of the Bosch API functions to the BOARD specific function
 *
 ***************************************************************************************************/
char dev_i2c_write( unsigned char dev_addr, unsigned char reg_addr, unsigned char *reg_data, unsigned char cnt )
{
    unsigned char data[MAX_REG_WRITE_SIZE];
    int8_t        ret_val   = ARM_DRIVER_ERROR;
    uint8_t       index     = 0;

    ASF_assert( cnt < MAX_REG_WRITE_SIZE );
    /* Send the register to be written followed by the byte(s) to be written */
    data[0] = reg_addr;
    for( index=1; index <= cnt; index++ )
    {
        /* Offsetting the data by 1 for the register address */
        data[index] = reg_data[index-1];
    }
    /* Data count is 1 for the register address prepended */
    ret_val = Driver_I2C0.MasterTransmit( dev_addr, data, (cnt+1), I2C_SEND_STOP_BITS );
    ASF_assert( ARM_DRIVER_OK == ret_val );

    return ret_val; /*< Return status */
}


/****************************************************************************************************
 * @fn      dev_i2c_read
 *          adaptation of the Bosch API functions to the BOARD specific function
 *
 ***************************************************************************************************/
char dev_i2c_read( unsigned char dev_addr, unsigned char reg_addr, unsigned char *reg_data, unsigned char cnt )
{
    int8_t ret_val_tx = ARM_DRIVER_ERROR;
    int8_t ret_val_rx = ARM_DRIVER_ERROR;

    /* Send the register address with transfer pending as it will be followed by the register data read  */
    ret_val_tx = Driver_I2C0.MasterTransmit( dev_addr, &reg_addr, 1, I2C_DO_NOT_SEND_STOP_BITS );
    ASF_assert( ARM_DRIVER_OK == ret_val_tx );
    /* Read the registers */
    ret_val_rx = Driver_I2C0.MasterReceive( dev_addr, reg_data, cnt, I2C_SEND_STOP_BITS );
    ASF_assert( ARM_DRIVER_OK == ret_val_rx );

    return ( ret_val_tx | ret_val_rx ); /*< Return status */
}

/*-------------------------------------------------------------------------------------------------*\
 |    E N D   O F   F I L E
\*-------------------------------------------------------------------------------------------------*/
