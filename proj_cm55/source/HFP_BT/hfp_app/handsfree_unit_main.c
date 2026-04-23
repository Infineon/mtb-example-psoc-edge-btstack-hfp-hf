/*******************************************************************************
* File Name: handsfree_unit_main.c
*
* Description: This is the source code for Handsfree Unit CE.
*
* Related Document: See README.md
*
********************************************************************************
* (c) 2025-2026, Infineon Technologies AG, or an affiliate of Infineon
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
* Header file includes
*******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <ctype.h>

#include "cybsp.h"
#include "wiced_bt_trace.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"

#include "wiced_memory.h"
#include "wiced_bt_cfg.h"

#include "board.h"
#include "audio_platform_common.h"
#include "handsfree_unit.h"
#include "handsfree_unit_api.h"
#include "handsfree_unit_main.h"
#include "handsfree_unit_evt_handler.h"

/*******************************************************************************
* Macros
*******************************************************************************/
#define MAX_PATH                        (256U)
#define BDA_LEN                         (6U)
#define IP_ADDR_LEN                     (16U)
#define MAX_PHONE_NUM_LEN               (16U)

#define BT_TASK_STACK_SIZE              (3 * 1024)
#define BT_TASK_PRIORITY                (configMAX_PRIORITIES - 5)

#define TASK_DELAY_1MS                  (1U)
#define TASK_DELAY_10MS                 (10U)

#define HFP_TASK_QUEUE_LENGTH           (50U)

#define CONSOLE_ENTER_CHAR              '\r'

#define ZERO                            (0U)
#define ONE                             (1U)

/*******************************************************************************
* Global Variables
*******************************************************************************/
int8_t connection_status;
QueueHandle_t hfp_task_q;

/*******************************************************************************
* Static variables
*******************************************************************************/
static TaskHandle_t bt_task_handle;
static hfp_task_q_data_t hfp_task_q_data;

static BD_ADDR hfp_ag_bd_address = HFP_AG_BD_ADDRESS;

/*******************************************************************************
* Function Declarations
*******************************************************************************/
static void bt_task(void *task_params);

/*******************************************************************************
* Function Definitions
*******************************************************************************/

/*******************************************************************************
* Function Name: bt_task_create()
********************************************************************************
* Summary:
*  BT stack initialization function wrapper
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
void bt_task_create(void)
{
    BaseType_t status;
    status = xTaskCreate(bt_task, "BT task", BT_TASK_STACK_SIZE, NULL, 
        BT_TASK_PRIORITY, &bt_task_handle);
    if (status != pdPASS)
    {
        printf("Error in starting BT task \n");
        CY_ASSERT(ZERO);
    }
}

/*******************************************************************************
* Function Name: bt_task()
********************************************************************************
* Summary:
*  Application start to initialize stack and other required modules
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
static void bt_task(void *task_params)
{
    wiced_result_t result = WICED_BT_SUCCESS;
    unsigned int handle;

    /* Create a message queue to communicate data/commands between various tasks
     * and callbacks.
     */
    hfp_task_q = xQueueCreate(HFP_TASK_QUEUE_LENGTH, sizeof(hfp_task_q_data_t));

    hfp_task_q_data.user_cmd = SET_VISIBILITY;
    if (pdTRUE != xQueueSend(hfp_task_q, &hfp_task_q_data, portMAX_DELAY))
    {
        printf(">>> Error:HFP Task Queue Send failed\r\n");
    }

    hfp_task_q_data.user_cmd = SET_PAIRING_MODE;
    if (pdTRUE != xQueueSend(hfp_task_q, &hfp_task_q_data, portMAX_DELAY))
    {
        printf(">>> Error:HFP Task Queue Send failed\r\n");
    }

    hfp_task_q_data.user_cmd = HF_CONNECT;
    hfp_task_q_data.data = hfp_ag_bd_address;
    hfp_task_q_data.data_len = sizeof(hfp_ag_bd_address);
    if (pdTRUE != xQueueSend(hfp_task_q, &hfp_task_q_data, portMAX_DELAY))
    {
        printf(">>> Error:HFP Task Queue Send failed\r\n");
    }

    handsfree_application_start();

    for (;;)
    {
        if (pdTRUE == xQueueReceive(hfp_task_q, &hfp_task_q_data, 
            pdMS_TO_TICKS(TASK_DELAY_10MS)))
        {
            if (handsfree_is_valid_state(hfp_task_q_data.user_cmd))
            {
                handle = handsfree_ctxt_data.rfcomm_handle;
                switch(hfp_task_q_data.user_cmd)
                {
                    case SET_VISIBILITY:
                    {
                        /* Make the device Discoverable and Connectable */
                        handsfree_handle_set_visibility(WICED_TRUE, WICED_TRUE);
                        break;
                    }

                    case SET_PAIRING_MODE:
                    {
                        /* Make the device pairable */
                        handsfree_handle_set_pairability(WICED_TRUE);
                        break;
                    }

                    case SET_INQUIRY:
                    {
                        /* Set inquiry */
                        handsfree_inquiry(WICED_TRUE);
                        break;
                    }

                    case HF_CONNECT:
                    {
                        BD_ADDR *peer_bd_addr = (BD_ADDR *)hfp_task_q_data.data;

                        /* Connect */
                        result = wiced_bt_hfp_hf_connect(*peer_bd_addr);

                        printf("HF connect status = %x\n", result);
                        break;
                    }

                    case ANSWER_CALL:
                    {
                        printf("Event: Accepting call!\r\n");

                        /* Answer Call */
                        handsfree_send_at_command((uint16_t)handle, 
                            HF_AT_CMD_ATA, ZERO, NULL);
                        break;
                    }

                    case HANGUP_CALL:
                    {
                        printf("Event: Ending call!\r\n");

                        /* Hang Up the Call */
                        handsfree_send_at_command((uint16_t)handle, 
                            HF_AT_CMD_CHUP, ZERO, NULL);
                        break;
                    }

                    case REDIAL:
                    {
                        printf("Event: Re-dialing call!\r\n");

                        handsfree_send_at_command((uint16_t)handle, HF_AT_CMD_D,
                            ZERO, NULL);
                        break;
                    }

                    default:
                    {
                        printf("Invalid HFP command\n");
                        break;
                    }
                }
            }
        }

        if (WICED_BT_HFP_HF_STATE_CONNECTED == handsfree_ctxt_data.connection_status)
        {
            board_led_set_state(LED_ON);
        }
        else if (WICED_BT_HFP_HF_STATE_DISCONNECTED == 
            handsfree_ctxt_data.connection_status)
        {
            board_led_set_blink();
        }
    }
}

/* [] END OF FILE */
