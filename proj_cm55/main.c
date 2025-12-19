/*******************************************************************************
 * File Name: main.c
 *
 * Description: This is the source code for the Bluetooth HFP Hands Free
 *              Example for ModusToolbox.
 *
 * Related Document: See README.md
 *
********************************************************************************
 * (c) 2025, Infineon Technologies AG, or an affiliate of Infineon
 * Technologies AG. All rights reserved.
 * This software, associated documentation and materials ("Software") is
 * owned by Infineon Technologies AG or one of its affiliates ("Infineon")
 * and is protected by and subject to worldwide patent protection, worldwide
 * copyright laws, and international treaty provisions. Therefore, you may use
 * this Software only as provided in the license agreement accompanying the
 * software package from which you obtained this Software. If no license
 * agreement applies, then any use, reproduction, modification, translation, or
 * compilation of this Software is prohibited without the express written
 * permission of Infineon.
 *
 * Disclaimer: UNLESS OTHERWISE EXPRESSLY AGREED WITH INFINEON, THIS SOFTWARE
 * IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 * INCLUDING, BUT NOT LIMITED TO, ALL WARRANTIES OF NON-INFRINGEMENT OF
 * THIRD-PARTY RIGHTS AND IMPLIED WARRANTIES SUCH AS WARRANTIES OF FITNESS FOR A
 * SPECIFIC USE/PURPOSE OR MERCHANTABILITY.
 * Infineon reserves the right to make changes to the Software without notice.
 * You are responsible for properly designing, programming, and testing the
 * functionality and safety of your intended application of the Software, as
 * well as complying with any legal requirements related to its use. Infineon
 * does not guarantee that the Software will be free from intrusion, data theft
 * or loss, or other breaches ("Security Breaches"), and Infineon shall have
 * no liability arising out of any Security Breaches. Unless otherwise
 * explicitly approved by Infineon, the Software may not be used in any
 * application where a failure of the Product or any consequences of the use
 * thereof can reasonably be expected to result in personal injury.
*******************************************************************************/
/*******************************************************************************
* Includes
*******************************************************************************/
#include "cybsp.h"
#include "cy_time.h"

#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "wiced_bt_trace.h"
#include "wiced_bt_cfg.h"
#include "app_bt_utils.h"

#include "handsfree_unit.h"
#include "handsfree_unit_main.h"
#include "handsfree_unit_api.h"
#include "handsfree_unit_evt_handler.h"
#include "retarget_io_init.h"
#include "app_bt_bonding.h"
#include "app_i2s.h"
#include "audio_encoder_task.h"
#include "board.h"

/*******************************************************************************
* Macros
*******************************************************************************/
#define ZERO                                (0U)


/*******************************************************************************
* Global variables
*******************************************************************************/
/* RTC HAL object */
static mtb_hal_rtc_t rtc_obj;

/*******************************************************************************
* Function Definitions
*******************************************************************************/
/*******************************************************************************
* Function Name: setup_clib_support
********************************************************************************
* Summary:
*   1. This function configures and initializes the Real-Time Clock (RTC).
*   2. It then initializes the RTC HAL object to enable CLIB support library 
*      to work with the provided Real-Time Clock (RTC) module.
*
* Parameters:
*  void
*
* Return:
*  void
*
*******************************************************************************/
static void setup_clib_support(void)
{
    /* RTC Initialization */
    Cy_RTC_Init(&CYBSP_RTC_config);
    Cy_RTC_SetDateAndTime(&CYBSP_RTC_config);

    /* Initialize the ModusToolbox CLIB support library */
    mtb_clib_support_init(&rtc_obj);
}

/*****************************************************************************
* Function Name: main
******************************************************************************
* Summary:
*  1. Entry point to the application on CM55.
*  2. Set device configuration and start BT application RTOS tasks
*
* Parameters:
*  void
*
* Return:
*  int
*
*****************************************************************************/
int main(void)
{
    cy_rslt_t result;

    /* Initialize the device and board peripherals */
    result = cybsp_init();

    /* Board init failed. Stop program execution */
    if (CY_RSLT_SUCCESS != result)
    {
        CY_ASSERT(ZERO);
    }

    /* Setup CLIB support library. */
    setup_clib_support();

    /* Enable global interrupts */
    __enable_irq();

    /* retarget-io init failed. Stop program execution */
    if (CY_RSLT_SUCCESS != result)
    {
        CY_ASSERT(ZERO);
    }

    /* Initialize retarget-io middleware */
    init_retarget_io();

    /* \x1b[2J\x1b[;H - ANSI ESC sequence to clear screen */
    printf("\x1b[2J\x1b[;H");
    printf("*************************************************************************\r\n");
    printf("***** PSOC EDGE MCU: Handsfree Profile (HFP) Unit - Hands free (HF) *****\r\n");
    printf("*************************************************************************\r\n\r\n");

    /* Initialize USER Button and USER LED */
    board_init();
    board_led_set_state(LED_OFF);

    /* Initialize the device used by kv-store for performing
     * read/write operations to the NVM
     */
    app_kv_store_init();

    /* TLV codec initiailization */
    app_tlv_codec_init();

    /* I2S initialization */
    app_i2s_init();

    bt_task_create();

    /* Create the Audio Encoder task */
    result = create_audio_encoder_task();
    CY_ASSERT(CY_RSLT_SUCCESS == result);

    /* Start the FreeRTOS scheduler */
    vTaskStartScheduler();

    return ZERO;
}

/* [] END OF FILE */
