/******************************************************************************
* File Name : audio_aec_utils.c
*
* Description :
* AEC reference Queue management 
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
* Header Files
*******************************************************************************/
#include "FreeRTOS.h"
#include "cy_log.h"
#include "task.h"
#include "queue.h"
#include "cy_pdl.h"
#include "cybsp.h"
#include "cycfg.h"
#include "retarget_io_init.h"
#include "audio_aec_utils.h"

/*******************************************************************************
* Macros
*******************************************************************************/

#define AEC_REF_Q_LEN        (60)
#define AEC_REF_Q_SIZE   (640)

/*******************************************************************************
* Global Variables
*******************************************************************************/

QueueHandle_t aec_ref_queue;

/*******************************************************************************
* Internal helper functions (TASK CONTEXT ONLY)
*******************************************************************************/

/******************************************************************************
* Function Name: aec_queue_push
*
* Summary:
*   Pushes an item to the back of the specified FreeRTOS queue.
*
* Parameters:
*   queue     - Handle of the queue to which the item will be pushed.
*   item_ptr  - Pointer to the data item to be copied into the queue.
*
* Return:
*   cy_rslt_t - CY_RSLT_SUCCESS on success,
*               AEC_QUEUE_FAILURE if the queue is full.
******************************************************************************/

static cy_rslt_t aec_queue_push(QueueHandle_t queue, void* item_ptr)
{
    if (pdTRUE != xQueueSendToBack(queue, item_ptr, 0))
    {
        return AEC_QUEUE_FAILURE;
    }
    return CY_RSLT_SUCCESS;
}

/******************************************************************************
* Function Name: aec_queue_pop
*
* Summary:
*   Pops an item from the front of the specified FreeRTOS queue.
*
* Parameters:
*   queue     - Handle of the queue from which the item is popped.
*   item_ptr  - Pointer to the buffer where the popped data is stored.
*
* Return:
*   cy_rslt_t - CY_RSLT_SUCCESS on success,
*               AEC_QUEUE_FAILURE if the queue is empty.
******************************************************************************/

static cy_rslt_t aec_queue_pop(QueueHandle_t queue, void* item_ptr)
{
    if (pdTRUE != xQueueReceive(queue, item_ptr, 0))
    {
        return AEC_QUEUE_FAILURE;
    }
    return CY_RSLT_SUCCESS;
}

/******************************************************************************
* Function Name: aec_push
*
* Summary:
*   Pushes a 10 ms speaker reference audio frame into the AEC reference queue.
*
* Parameters:
*   item_ptr  - Pointer to the audio frame buffer to be pushed into the queue.
*
* Return:
*   cy_rslt_t - CY_RSLT_SUCCESS on successful push,
*               AEC_QUEUE_FAILURE if the queue is full or push fails.
******************************************************************************/

cy_rslt_t aec_push(short* item_ptr)
{
    return aec_queue_push(aec_ref_queue, item_ptr);
}

/******************************************************************************
* Function Name: aec_pop
*
* Summary:
*   Pops a speaker reference audio frame from the AEC reference queue
*   when sufficient buffered frames are available.
*
* Parameters:
*   item_ptr    - Pointer to the buffer where the popped audio frame
*                 will be copied.
*   bulk_delay  - Minimum number of audio frames that must be present
*                 in the queue before popping.
*
* Return:
*   cy_rslt_t   - CY_RSLT_SUCCESS on successful pop,
*                 AEC_QUEUE_FAILURE if insufficient frames are available.
******************************************************************************/
cy_rslt_t aec_pop(short* item_ptr, int bulk_delay)
{
    int queue_frames = uxQueueMessagesWaiting(aec_ref_queue);
    if (queue_frames >= bulk_delay)
    {
        return aec_queue_pop(aec_ref_queue, item_ptr);
    }

    return AEC_QUEUE_FAILURE;
}

/*******************************************************************************
* Function Name: aec_flush
*
* Summary:
*   Reset AEC reference queue.
* Parameters:
*  void
*
* Return:
*  void
*
*******************************************************************************/
void aec_flush(void)
{
    xQueueReset(aec_ref_queue);
}

/*******************************************************************************
* Function Name : void audio_aec_ref_init(void)

*
* Summary:
*   Initialize AEC reference queue.
* Parameters:
*  void
*
* Return:
*  void
*
*******************************************************************************/
void audio_aec_ref_init(void)
{
    aec_ref_queue = xQueueCreate(AEC_REF_Q_LEN , AEC_REF_Q_SIZE);
    if (NULL == aec_ref_queue)
    {
       cy_log_printf("AEC queue creation failed (queue handle is NULL)\r\n");
        CY_ASSERT(ZERO);
    }
}

/* [] END OF FILE */
