/******************************************************************************
* File Name : audio_enhancement_interface.c
*
* Description :
* Wrapper for Audio Enhancement
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

#include "audio_enhancement_interface.h"

#include "audio_platform_common.h"
//#include "app_pdm_pcm.h"
#include "pdm_mic_interface.h"



/*******************************************************************************
* Macros
*******************************************************************************/
#define AE_TASK_PRIORITY                            (2)
#define AE_QUEUE_SIZE                               (MONO_AUDIO_DATA_IN_BYTES*7)
#define AE_QUEUE_ELEMENTS                           (10)

#define AE_STACK_SIZE                               (1024)
/*******************************************************************************
* Global Variables
*******************************************************************************/
volatile bool ae_toggle_flag;

int16_t output_buffer_temp[240];
int16_t output_buffer_temp2[240];
int16_t debug_output_buffer[22*160];
volatile uint8_t entry_tracker = 0 ,afe_ping_pong = 0, frame_counter = 0;

TaskHandle_t rtos_aec_task;
QueueHandle_t aec_queue_handle;

/*******************************************************************************
* Function Name: license_limitation_exit
********************************************************************************
* Summary:
*  This function is blocking call after Audio Front End license timeout
*
*******************************************************************************/
void license_limitation_exit()
{
    while(1){}
}

/*******************************************************************************
 * Function Name: audio_enhancement_process_output
 *******************************************************************************
 * Summary:
 * Use case API
 * Sends back AE data/tuning data.
 *
 * Parameters:
 *  output_buffer: pointer to the output audio data buffer.
 *
 * Return:
 *  void
 *
 *******************************************************************************/
 
void audio_enhancement_process_output(ae_buffer_info_t *ae_output_buffer)
{
    
    
    BaseType_t ret = pdTRUE;
    
	if (aec_queue_handle !=NULL)
    {
        ret = xQueueSend(aec_queue_handle, (void*)ae_output_buffer, 0);
    }
    if (pdTRUE != ret)
    {
        printf(">>> Send failed - Queue full \r\n");
    }  

    return;
}


/*******************************************************************************
* Function Name: ae_interface_init
********************************************************************************
* Summary:
* Initialize Audio Enhancement
*
* Parameters:
*  channels - number of input channels
* 
* Return:
*  Result of AE initialization.
*
*******************************************************************************/

int ae_interface_init(int channels)
{

    ae_rslt_t result = AE_RSLT_SUCCESS;
    BaseType_t rtos_task_status;

    result = audio_enhancement_init(channels);
    
    if (result != AE_RSLT_SUCCESS) 
    {
        printf("DEEPCRAFT Audio Enhancement initialization failed \r\n");
    }
    else
    {
        printf("DEEPCRAFT Audio Enhancement initialized \r\n");
    }
#if AE_APP_PROFILE
    cy_profiler_init();
    cy_afe_profile(AFE_PROFILE_CMD_ENABLE,NULL);
#endif /* AE_APP_PROFILE */

    aec_queue_handle = xQueueCreate(AE_QUEUE_ELEMENTS, AE_QUEUE_SIZE);
    if (aec_queue_handle == NULL)
    {
         printf("Init queue for AEC streamer queue failed \r\n");
         CY_ASSERT(0);
    }

    rtos_task_status = xTaskCreate(mains_powered_aec_task, "AEC_task",
                        AE_STACK_SIZE, NULL, AE_TASK_PRIORITY,
                        &rtos_aec_task);

    if (pdPASS != rtos_task_status)
    {
        printf("AEC streamer task create failed \r\n");
        CY_ASSERT(0);
    }
    
    return result;
}

/*******************************************************************************
* Function Name: ae_interface_feed
********************************************************************************
* Summary:
*  Feed mic audio data and AEC reference to Audio Enhancement.
*
*******************************************************************************/

int ae_interface_feed(void* audio_input, void* aec_buffer)
{
    cy_rslt_t result = CY_RSLT_SUCCESS;

    result = audio_enhancement_feed_input((int16_t*)audio_input, (int16_t*)aec_buffer);
    
    if (AE_RSLT_LICENSE_ERROR == result)
    {
        printf("CPU Halt: Audio Enhancement Restricted License Timeout - Reset the board \r\n");
        license_limitation_exit();
        return result;
    }

    if(AE_RSLT_SUCCESS != result)
    {
        printf("Failed to feed data to AE \r\n");
        return result;
    }
    return result;
}


/*******************************************************************************
* Function Name: mains_powered_aec_task
********************************************************************************
* Summary:
* AEC streamer task.
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/

void mains_powered_aec_task(void *arg)
{

    char aec_afe_data[AE_QUEUE_SIZE] = {0};
    ae_buffer_info_t *aec_afe_data_ptr= NULL;
    int16_t *output_buffer = NULL;
     
    while(1)
    {
        memset(aec_afe_data, 0, AE_QUEUE_SIZE);
        if (pdTRUE == xQueueReceive(aec_queue_handle, (void*)aec_afe_data, portMAX_DELAY))
        {

            aec_afe_data_ptr = (ae_buffer_info_t*)aec_afe_data;
            output_buffer = (int16_t*)aec_afe_data_ptr->output_buf;
            
			if (afe_ping_pong == 0){
			    if (entry_tracker == 0){
			    	memcpy(&output_buffer_temp[0],(char*)output_buffer,320);
			    	entry_tracker = 1;
			    }else{
			    	memcpy(&output_buffer_temp[160],(char*)output_buffer,160);
			    	memcpy(&output_buffer_temp2[0],(char*)&output_buffer[80],160);
			    	pdm_pcm_data_passer((char*) output_buffer_temp);
			    	afe_ping_pong = 1;
			    }
			}else {
			    	memcpy(&output_buffer_temp2[80],(char*)output_buffer,320);
			    	pdm_pcm_data_passer((char*) output_buffer_temp2);
			    	afe_ping_pong = 0;
			    	entry_tracker = 0;
			}
   

        }
    }
}
/* [] END OF FILE */
