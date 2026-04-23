/*******************************************************************************
* File Name:   audio_encoder_task.c
*
* Description: This file contains the task that performs audio encoding for
*              HFP Unit example.
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
#include "audio_encoder_task.h"
#include "cy_pdl.h"
#include "cybsp.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "sbc_encoder_api.h"
#include "cy_audio_sw_codecs.h"
#include "cy_audio_sw_codec_utils.h"
#include "audio_platform_common.h"

/*******************************************************************************
* Macros
*******************************************************************************/
/* Macro to check if the result of an operation was successful. When it has
* failed, print the error message and suspend the task.
*/
#define CHECK_RESULT(result, error_message...)     \
    do                                             \
    {                                              \
        if(CY_RSLT_SUCCESS != (cy_rslt_t)result)   \
        {                                          \
            printf(error_message);                 \
            vTaskSuspend(NULL);                    \
        }                                          \
    } while(0)

#define ZERO                    (0U)

/*******************************************************************************
* Global Variables
********************************************************************************/
/* Audio encoder task handle */
TaskHandle_t audio_encoder_task_handle;

/* Handle of the queue holding the Audio encoder task commands. */
QueueHandle_t audio_encoder_task_q;

/* Handle of the semaphore for synchronizing with the other tasks */
SemaphoreHandle_t audio_encoder_semaphore;

/* Variable to receive the queue data for this task. */
static audio_encoder_q_data_t encoder_q_data;
static cy_audio_sw_codec_t enc_handle;

static uint32_t recommended_in_size;
static uint32_t min_out_size;
static audio_tx_q_data_t audio_tx_q_data;

/*******************************************************************************
* Function Prototypes
********************************************************************************/
static void audio_encoder_task(void* pvParameters);

/*******************************************************************************
* Function Definitions
*******************************************************************************/
/*******************************************************************************
* Function Name: create_audio_encoder_task
********************************************************************************
* Summary:
*  Function that creates the audio encodertask.
*
* Parameters:
*  None
*
* Return:
*  CY_RSLT_SUCCESS upon successful creation of the task, else a non-zero value
*   that indicates the error.
*
*******************************************************************************/
cy_rslt_t create_audio_encoder_task(void)
{
    BaseType_t status;
   
    status = xTaskCreate(audio_encoder_task, "Audio Enc Task", 
        TASK_AUDIO_ENCODER_TASK_STACK_SIZE, NULL, 
        TASK_AUDIO_ENCODER_TASK_PRIORITY, &audio_encoder_task_handle);
   
    return (pdPASS == status) ? CY_RSLT_SUCCESS : (cy_rslt_t) status;
}

/*******************************************************************************
* Function Name: audio_encoder_task
********************************************************************************
* Summary:
*  Function that creates Audio Encoder task
*
* Parameters:
*  void *pvParameters : Task parameter defined during task creation (unused)
*
* Return:
*  None
*
*******************************************************************************/
static void audio_encoder_task(void* pvParameters)
{
    /* Status variable to indicate the result of various operations */
    cy_rslt_t result;

    uint8_t *pSBC = NULL;
    uint8_t *encoder_output = NULL;
    uint32_t in_size = ZERO;
    uint32_t out_size = ZERO;
    uint32_t num_frames = ZERO;

    /* Remove warning for unused parameter */
    (void)pvParameters;

    printf("Audio Encoder task started \r\n");

    /* Create a message queue to communicate data/commands between various
    * tasks and callbacks.
    */
    audio_encoder_task_q = xQueueCreate(AUDIO_ENCODER_TASK_QUEUE_LENGTH,
        sizeof(audio_encoder_q_data_t));

    audio_encoder_semaphore = xSemaphoreCreateBinary();
    if (NULL == audio_encoder_semaphore)
    {
        printf("Error in semaphore creation\r\n");
        return;
    }

    for (;;)
    {
        if (pdTRUE == xQueueReceive(audio_encoder_task_q, &encoder_q_data,
            portMAX_DELAY))
        {
            switch(encoder_q_data.cmd)
            {
            case CMD_ENCODER_INIT:
            {
                result = sbc_encoder_app_init(&enc_handle, 
                    (sbc_encoder_params_t *)encoder_q_data.in_data);
                if (CY_RSLT_SUCCESS != result)
                {
                    printf(">> sbc_encoder_app_init failed: %0lX\r\n", 
                        (unsigned long)result);
                }

                xSemaphoreGive(audio_encoder_semaphore);
                break;
            }

            case CMD_ENCODER_CONFIG_IN_SIZE:
            {
                result = cy_audio_sw_codec_get_encoder_recommended_inbuf_size(
                    enc_handle, &recommended_in_size);
                if(CY_RSLT_SUCCESS != result)
                {
                    printf("Failed to get recommended input buffer size: \
                        %0lX\r\n", (unsigned long)result);
                    break;
                }

                if (encoder_q_data.in_data_len != sizeof(recommended_in_size))
                {
                    printf("Invalid data size for Encoder task command \
                        CMD_ENCODER_CONFIG_IN_SIZE: %lu\r\n",       \
                        (unsigned long)encoder_q_data.in_data_len);
                    break;
                }

                *((uint32_t *)encoder_q_data.in_data) = recommended_in_size;

                xSemaphoreGive(audio_encoder_semaphore);
                break;
            }

            case CMD_ENCODER_CONFIG_OUT_SIZE:
            {
                result = cy_audio_sw_codec_get_encoder_recommended_outbuf_size(
                    enc_handle, recommended_in_size, &min_out_size);
                if(CY_RSLT_SUCCESS != result)
                {
                    printf(">> Failed to get recommended output buffer size: \
                        %0lX\r\n", (unsigned long)result);
                    break;
                }

                if (encoder_q_data.in_data_len != sizeof(min_out_size))
                {
                    printf(">> Invalid data size for Encoder task command \
                        CMD_ENCODER_CONFIG_OUT_SIZE: %lu\r\n", 
                        (unsigned long)encoder_q_data.in_data_len);
                    break;
                }

                *((uint32_t *)encoder_q_data.in_data) = min_out_size;

                xSemaphoreGive(audio_encoder_semaphore);
                break;
            }

            case CMD_ENCODE_DATA:
            {
                num_frames = encoder_q_data.in_data_len / recommended_in_size;

                pSBC = ((uint8_t *) encoder_q_data.in_data);
                encoder_output = ((uint8_t *) encoder_q_data.out_data);

                for (uint32_t i = ZERO; i < num_frames; i++)
                {
                    in_size = recommended_in_size;
                    out_size = encoder_q_data.out_data_len / num_frames;

                    result = sbc_encoder_app_encode(enc_handle, pSBC, &in_size, 
                        encoder_output, &out_size);
                    if (CY_RSLT_SUCCESS != result)
                    {
                        printf(">> sbc_encoder_app_encode error: %0lX\r\n",
                            (unsigned long)result);
                    }

                    audio_tx_q_data.data = encoder_output;
                    audio_tx_q_data.data_len = out_size;
                    if (pdTRUE != xQueueSend(audio_tx_q, &audio_tx_q_data, 
                        portMAX_DELAY))
                    {
                        printf("Error:Audio TX Queue Send failed*****\r\n");
                    }

                    pSBC += recommended_in_size;
                    encoder_output += out_size;
                }
                break;
            }
            }
        }
    }
}

/* [] END OF FILE */
