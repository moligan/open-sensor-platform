/* Open Sensor Platform Project
 * https://github.com/sensorplatforms/open-sensor-platform
 *
 * Copyright (C) 2015 Audience Inc.
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
#include "common.h"
#include "hw_setup.h"

/*-------------------------------------------------------------------------------------------------*\
 |    E X T E R N A L   V A R I A B L E S   &   F U N C T I O N S
\*-------------------------------------------------------------------------------------------------*/
#ifdef ASF_PROFILING
  extern uint32_t gStackMem;
  extern uint32_t gStackSize;
  extern const char C_gStackPattern[8];
#endif

/*-------------------------------------------------------------------------------------------------*\
 |    P U B L I C   V A R I A B L E S   D E F I N I T I O N S
\*-------------------------------------------------------------------------------------------------*/
#ifdef DEBUG_BUILD
  char _errBuff[ERR_LOG_MSG_SZ];
#endif

/* Clock information */
RCC_ClocksTypeDef gRccClockInfo;

/*-------------------------------------------------------------------------------------------------*\
 |    P R I V A T E   C O N S T A N T S   &   M A C R O S
\*-------------------------------------------------------------------------------------------------*/

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
 * @fn      main
 *          Main entry point to the application firmware
 *
 * @param   none
 *
 * @return  none
 *
 ***************************************************************************************************/
int main( void )
{
#ifdef ASF_PROFILING
    register uint32_t *pStack = (uint32_t *)&gStackMem;
    register uint32_t stkSize = (uint32_t)&gStackSize;
    register uint32_t idx;

    /* main() is using the same stack that we are trying to initialize so we leave the last 32 bytes */
    for ( idx = 0; idx < ((stkSize-32)/sizeof(C_gStackPattern)); idx++)
    {
        *pStack++ = *((uint32_t *)C_gStackPattern);
        *pStack++ = *((uint32_t *)(C_gStackPattern+4));
    }
#endif
    /**
     Get the clocks going and setup UART1 as the basic debug port for now. Debug messages will be
     handled in a separate task when the RTOS takes over.
     */

    /* NVIC configuration */
    SystemInterruptConfig();

    /* System clock configuration for regular mode */
    //SystemClkConfig( false );
    /* Clock is set up in "system_stm32f30x.c" which is generated by an automatic clock configuration
      system and can be easily customized.
      To select different clock setup, use the "STM32Cube MX" utility that can be downloaded from 
      "http://www.st.com/content/st_com/en/products/development-tools/software-development-tools/stm32-software-development-tools/stm32-configurators-and-code-generators/stm32cubemx.html"
    */
    SystemCoreClockUpdate();

    /* Configure the GPIO ports (non module specific) */
    SystemGPIOConfig();

    /* Set startup state of LEDs */
    LED_Init();                   /* Initialize Debug LEDs */
    LED_On(FRONT_LED); //Visual indication that we powered up

    /* Configure RTC */
    RTC_Configuration();

    /* Configure debug UART port - we do it here to enable assert messages early in the system */
    DebugUARTConfig( DBG_UART_BAUD, USART_WordLength_8b, USART_StopBits_1, USART_Parity_No );
    DebugPortInit();

    /* Print version number */
    printf("\r\n### RTX Hello World Application Example for PurpleHaze Rev-F board: Date: %s - %s ###\r\n",
        __DATE__, __TIME__);

    /* Display System clock information */
    RCC_GetClocksFreq( &gRccClockInfo );
    D0_printf("System Clocks:\r\n");
    D0_printf("\tSYSCLK   - %ld\r\n", gRccClockInfo.SYSCLK_Frequency);
    D0_printf("\tHCLK     - %ld\r\n", gRccClockInfo.HCLK_Frequency);
    D0_printf("\tPCLK     - %ld\r\n", gRccClockInfo.PCLK_Frequency);
    D0_printf("\tADCCLK   - %ld\r\n", gRccClockInfo.ADCCLK_Frequency);
    D0_printf("\tCECCLK   - %ld\r\n", gRccClockInfo.CECCLK_Frequency);
    D0_printf("\tI2C1CLK  - %ld\r\n", gRccClockInfo.I2C1CLK_Frequency);
    D0_printf("\tUSART1   - %ld\r\n", gRccClockInfo.USART1CLK_Frequency);
    D0_printf("Device SNo.: %08X-%08X-%08X\r\n", gDevUniqueId->uidWords[2],
        gDevUniqueId->uidWords[1], gDevUniqueId->uidWords[0]);
    D0_printf("\t%02X.%02X.%02X.%02X.%02X.%02X.%02X.%02X.%02X.%02X.%02X.%02X\r\n",
        gDevUniqueId->uidBytes[11], gDevUniqueId->uidBytes[10], gDevUniqueId->uidBytes[9],
        gDevUniqueId->uidBytes[8], gDevUniqueId->uidBytes[7], gDevUniqueId->uidBytes[6],
        gDevUniqueId->uidBytes[5], gDevUniqueId->uidBytes[4], gDevUniqueId->uidBytes[3],
        gDevUniqueId->uidBytes[2], gDevUniqueId->uidBytes[1], gDevUniqueId->uidBytes[0]);

    /* Get the OS going - This must be the last call */
    AsfInitialiseTasks();

    /* If it got here something bad happened */
    ASF_assert_fatal(false);
}


extern AsfTaskHandle asfTaskHandleTable[];
/*----------------------------------------------------------------------------
  Task 1 'ledOn': switches the LED on
 *---------------------------------------------------------------------------*/
ASF_TASK void LED_On_Task( ASF_TASK_ARG )
{
    uint8_t ledID = 0;
    for (;;) {
        LED_On(ledID);                      /* Turn LED On                  */
        os_evt_set (0x0001, asfTaskHandleTable[LED_OFF_TASK_ID].handle);
        os_dly_wait (MSEC_TO_TICS(200));    /* delay 500ms                  */
        ledID = (ledID + 1) % NUM_LEDS;
    }
}

/*----------------------------------------------------------------------------
  Task 2 'ledOff': switches the LED off
 *---------------------------------------------------------------------------*/
ASF_TASK void LED_Off_Task( ASF_TASK_ARG )
{
    uint8_t ledID = 0;
    for (;;) {
        os_evt_wait_and (0x0001, 0xffff);   /* wait for an event flag 0x0001    */
        os_dly_wait(MSEC_TO_TICS(100));     /* delay 100ms                       */
        LED_Off(ledID);                     /* Turn LED Off                     */
        ledID = (ledID + 1) % NUM_LEDS;
    }
}


/*-------------------------------------------------------------------------------------------------*\
 |    E N D   O F   F I L E
\*-------------------------------------------------------------------------------------------------*/
