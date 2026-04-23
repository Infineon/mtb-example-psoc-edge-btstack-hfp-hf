/*******************************************************************************
* File Name:   audio_encoder_task.h
*
* Description: This file is the public interface of audio_encoder_task.c. 
*   This file also contains the Audio Task configuration parameters.
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
* Include guard
*******************************************************************************/
#ifndef AUDIO_ENC_TASK_H_
#define AUDIO_ENC_TASK_H_

/*******************************************************************************
* Header Files
*******************************************************************************/
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "cybsp.h"
#include "semphr.h"
#include "cy_audio_sw_codecs.h"
#include "cy_audio_sw_codec_utils.h"

/*******************************************************************************
* Macros
********************************************************************************/
/* Task priority and stack size for the Audio Encoder task */
#define TASK_AUDIO_ENCODER_TASK_PRIORITY            (3U)
#define TASK_AUDIO_ENCODER_TASK_STACK_SIZE          (5U * 1024U)
#define AUDIO_ENCODER_TASK_QUEUE_LENGTH             (20U)
#define AUDIO_ENCODER_OUTPUT_BUFFER_LENGTH          (90U)

/*******************************************************************************
* Data structure and enumeration
********************************************************************************/
/* Data-type of various commands and responses for the IPC communication */
typedef enum
{
    CMD_ENCODER_INIT,
    CMD_ENCODER_CONFIG_IN_SIZE,
    CMD_ENCODER_CONFIG_OUT_SIZE,
    CMD_ENCODE_DATA,
} audio_encoder_cmd_t;

/* Data-type for Audio Encoder task queue data */
typedef struct
{
    audio_encoder_cmd_t cmd;
    uint32_t in_data_len;       /* Size of the input buffer in bytes */
    uint8_t* in_data;           /* Pointer to the input buffer */
    uint32_t out_data_len;      /* Size of the output buffer in bytes */
    uint8_t* out_data;          /* Pointer to the output buffer */
} audio_encoder_q_data_t;

/*******************************************************************************
* Extern Variables
*******************************************************************************/
extern QueueHandle_t audio_encoder_task_q;
extern SemaphoreHandle_t audio_encoder_semaphore;

/*******************************************************************************
* Function Prototypes
*******************************************************************************/
cy_rslt_t create_audio_encoder_task(void);

#endif /* AUDIO_ENC_TASK_H_ */

/* [] END OF FILE */
