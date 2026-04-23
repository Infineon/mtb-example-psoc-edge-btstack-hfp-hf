/******************************************************************************
* File Name : audio_feed_interface.c
*
* Description : Feeds data received via PDM ISR to the audio pipeline 
*               on CM55.
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
#include "audio_conv_utils.h"
#include "cybsp_types.h"
#include "audio_input_configuration.h"

#include "app_i2s.h"
#include "audio_enhancement_interface.h"
#include "audio_aec_utils.h"


/*******************************************************************************
* Macros
*******************************************************************************/
#define MAX_SIZE_FOR_NON_INTERLEAVE_STEREO_IN_BYTES             (160*2*2)
#define FRAME_SIZE_MS                                           (10U)
#ifndef FRAME_SIZE
    #define FRAME_SIZE                                          (320U)
#endif
#define PLAYBACK_DATA_FRAME_SIZE                                (2*FRAME_SIZE)
#define BULK_DELAY_10MS                             			(10u)

/*******************************************************************************
* Global Variables
*******************************************************************************/
int16_t non_interleaved_audio_ping[MAX_SIZE_FOR_NON_INTERLEAVE_STEREO_IN_BYTES] 
    = {0};
int16_t non_interleaved_audio_pong[MAX_SIZE_FOR_NON_INTERLEAVE_STEREO_IN_BYTES] 
    = {0};

int16_t swap_audio[MAX_SIZE_FOR_NON_INTERLEAVE_STEREO_IN_BYTES]= {0};

int16_t audio_tx_buffer[MONO_AUDIO_DATA_IN_BYTES*2] = {0};
int16_t aec_ref_buffer[MONO_AUDIO_DATA_IN_BYTES] = {0};

int16_t* actual_aec_ref = 0;

int8_t buff_toggle_flag = 0;
int16_t* non_interleaved_audio = NULL;

int16_t aec_ref_zero_buffer[FRAME_SIZE] = {0};

unsigned int bdm_aec_ref_sent_len = 0;
char stereo_pdm[PLAYBACK_DATA_FRAME_SIZE] = {0};

extern int8_t aec_ref_flag;
extern unsigned int bdm_aec_ref_len;
extern char *bdm_aec_ref_buffer;
uint8_t bulk_delay = AFE_CONFIG_BULK_DELAY;
uint8_t bulk_delay_equivalent_frames = 0;


/*******************************************************************************
* Function Name: audio_mic_data_feed_cm55
********************************************************************************
* Summary:
* Receive PDM mic frames and feed it to the audio pipeline.
*
* Parameters:
*  audio_data - Pointer to audio buffer.
* Return:
*  None
*
*******************************************************************************/
void audio_mic_data_feed_cm55(int16_t *audio_data)
{
    cy_rslt_t result = CY_RSLT_SUCCESS;
    int16_t* aec_reference = NULL;
   
    uint8_t bulk_delay_equivalent_frames = 0;
    
    uint16_t *stereo = (uint16_t*)&stereo_pdm[0];
    cy_rslt_t ret_val  = CY_RSLT_SUCCESS;


 #ifndef ENABLE_IFX_AEC
    aec_reference = NULL;
#endif /* ENABLE_IFX_AEC */
/* For Bulk Delay measurement via Calibrate option*/
    if(NULL != bdm_aec_ref_buffer)
    {
        aec_reference = (int16_t* )((char *)bdm_aec_ref_buffer+bdm_aec_ref_sent_len);
        convert_mono_to_stereo_interleaved(stereo,(uint16_t *)aec_reference);
        bdm_aec_ref_sent_len = bdm_aec_ref_sent_len + FRAME_SIZE;
        if(bdm_aec_ref_len == bdm_aec_ref_sent_len)
        {
             bdm_aec_ref_sent_len = 0;
        }
        ret_val=aec_pop(audio_tx_buffer,0);
        if (ret_val==CY_RSLT_SUCCESS)
        {
        /* Convert Stereo AEC to Mono AEC reference */
            convert_stereo_interleaved_to_mono((uint16_t*)audio_tx_buffer,(uint16_t*)aec_ref_buffer);
            aec_reference = aec_ref_buffer;
        } else {
            aec_reference = NULL;
        }
    }
 /* End of Bulk Delay measurement */   
    else 
    {
        bulk_delay_equivalent_frames =(bulk_delay/BULK_DELAY_10MS);
    
        ret_val=aec_pop(audio_tx_buffer,bulk_delay_equivalent_frames);
        if (ret_val==CY_RSLT_SUCCESS)
        {
        /* Convert Stereo AEC to Mono AEC reference */
            convert_stereo_interleaved_to_mono((uint16_t*)audio_tx_buffer,(uint16_t*)aec_ref_buffer);
            aec_reference = aec_ref_buffer;
        }
        else
        {
            aec_reference = NULL;
        }
    }

    if (buff_toggle_flag == 0)
    {
        non_interleaved_audio = non_interleaved_audio_ping;
        buff_toggle_flag = 1;

    } else if (buff_toggle_flag == 1)
    {
        non_interleaved_audio = non_interleaved_audio_pong;
        buff_toggle_flag = 0;
    }
    
    // actual_aec_ref = aec_ref_zero_buffer;

#ifdef ENABLE_STEREO_INPUT_FEED


    convert_interleaved_to_stereo_non_interleaved((uint16_t *)audio_data, 
        (uint16_t *)non_interleaved_audio);
    
    /* Feed the data to AFE */
    result = ae_interface_feed((int16_t*)non_interleaved_audio,
        aec_reference);
        
#else
    /* Feed the data to AFE */
    result = ae_interface_feed(audio_data, aec_reference);

#endif /* ENABLE_STEREO_INPUT_FEED */

    if(CY_RSLT_SUCCESS != result)
    {
        printf("Failed to feed audio frame to AFE \r\n ");
    }
}

/* [] END OF FILE */
