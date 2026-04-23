/*******************************************************************************
* File Name: audio_platform_common.c
*
* Description: This file contains the definitions of functions related to
*              PSOC EDGE specific audio.
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
#include "cy_pdl.h"
#include "cybsp.h"
#include "mtb_hal.h"
#include "audio_platform_common.h"
#include <stdio.h>
#include <stdlib.h>
#include "sbc_decoder.h"
#include "sbc_dec_func_declare.h"
#include "sbc_dct.h"
#include "sbc_types.h"
#include "wiced_bt_trace.h"
#include "sbc_encoder_api.h"
#include "audio_encoder_task.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "cy_utils.h"
#include "wiced_bt_types.h"
#include "app_i2s.h"
//#include "app_pdm_pcm.h"


/* Enable below block for verbose logging via printf  */
#if 0
   #ifdef WICED_BT_TRACE
       #undef WICED_BT_TRACE
   #endif
   #define WICED_BT_TRACE                              printf
#endif

/*******************************************************************************
* Macros
*******************************************************************************/
/* Size of persistent memory required by the mSBC decoder */
#define MSBC_STATIC_MEM_SIZE                          (1920U)
/* Size of scratch (temporary) memory required by the mSBC decoder */
#define MSBC_SCRATCH_MEM_SIZE                         (2048U)

#define mSBC_START_OFFSET                             (2U)

#define ZERO                                          (0U)
#define ONE                                           (1U)
#define TWO                                           (2U)

#define MSBC_SYNCWORD_SIZE_BYTES                      (4U)
#define MSBC_BITS_PER_NIBBLE                          (4U)
#define MSBC_BITS_PER_BYTE                            (8U)

#define MSBC_MAX_INPUT_LEN                            (60U)

/*******************************************************************************
* Global Variables
*******************************************************************************/
int pcm_bytes_per_frame;
uint32_t format;

/* Handle of the queue holding the Audio TX data for HFP. */
QueueHandle_t audio_tx_q;

i2s_playback_q_data_t i2s_playback_q_data_hfp;

bool mic_capture_active = false;

extern volatile uint32_t sco_audio_size;
extern volatile uint32_t sco_audio_read_ptr;
extern volatile uint32_t sco_audio_write_ptr;
extern int16_t sco_audio_buffer[SCO_PCM_BUFFER_SIZE];

/*******************************************************************************
* Static Variables
*******************************************************************************/
static SINT32 msbc_decoder_static_mem[MSBC_STATIC_MEM_SIZE / sizeof (SINT32)]
= {ZERO};

static SINT32 msbc_decoder_scratch_mem[MSBC_SCRATCH_MEM_SIZE / sizeof (SINT32)] 
= {ZERO};

static uint8_t rx_pcm_data[RX_PCM_BUFFER_SIZE] = {ZERO};

static uint8_t msbc_tx_second_byte_index = ZERO;
static uint16_t msbc_frame_len = ZERO;
static uint32_t recommended_in_size;
static uint32_t min_out_size;
static const uint8_t msbc_tx_first_byte = ONE;
static const uint8_t msbc_tx_second_byte[] = {0x08, 0x38, 0xC8, 0xF8};
static  SBC_DEC_PARAMS  msbc_decoder_config = { ZERO };
static audio_tx_q_data_t audio_tx_q_data;

static uint32_t next_rx_pcm_frame_num_to_be_sent = 0;
static uint32_t current_rx_pcm_index = 0;
static uint8_t *current_rx_pcm_ptr = NULL;

/* Variable to send commands to Encoder task queue. */
static audio_encoder_q_data_t encoder_q_data;

static sbc_encoder_params_t msbc_encoder_params = 
{
   .sample_rate = WBS_SAMPLE_RATE,
   .channels = ONE,
   .bit_width = ENCODER_BIT_WIDTH,
   .sbc_config = 
   {
       .channel_mode           = SBC_MONO,
       .allocation_method      = ZERO,
       .sub_bands              = ENCODER_SUB_BANDS,
       .sub_blocks             = ENCODER_SUB_BLOCKS,
       .bit_pool               = ENCODER_BIT_POOL,
       .sbc_mode               = SBC_DEC_MODE_WB,
       .uui_id                 = ENCODER_UUI_ID,
   },
};

/*******************************************************************************
* Function Declarations
*******************************************************************************/
static uint16_t get_msbc_frame_len(void);

/*******************************************************************************
* Function Definitions
*******************************************************************************/
/*******************************************************************************
* Function Name: msbc_audio_init
********************************************************************************
* Summary:
*  Initializes the encoder and decoder functions for use with HF SCO
*
* Parameters:
*  audio_playback_config pb_config_params : Decoder configurations to be set
*
* Return:
*  bool : true: successful, false: failed
*
*******************************************************************************/
bool msbc_audio_init(audio_playback_config pb_config_params)
{
    SINT16 sbc_result = ZERO;
    uint16_t sample_rate = ZERO;

    memset(msbc_decoder_static_mem, ZERO, 
        sizeof (msbc_decoder_static_mem));
 
    memset(msbc_decoder_scratch_mem, ZERO, 
        sizeof (msbc_decoder_scratch_mem));
    
    memset(&msbc_decoder_config, ZERO,
        sizeof (msbc_decoder_config));

    msbc_decoder_config.s32StaticMem  = msbc_decoder_static_mem;
    msbc_decoder_config.s32ScratchMem = msbc_decoder_scratch_mem;

    msbc_decoder_config.channelMode = pb_config_params.channelMode;
    msbc_decoder_config.numOfChannels = pb_config_params.numOfChannels;
    msbc_decoder_config.allocationMethod = pb_config_params.allocationMethod;
    msbc_decoder_config.numOfSubBands =  pb_config_params.numOfSubBands;
    msbc_decoder_config.numOfBlocks = pb_config_params.numOfBlocks;
    msbc_decoder_config.bitPool = pb_config_params.bitPool;

    msbc_decoder_config.sbc_mode = pb_config_params.sbc_mode;
    msbc_decoder_config.uui_id = pb_config_params.uui_id;

    sample_rate = pb_config_params.samplingFreq;

    format = sizeof(int16_t);

    WICED_BT_TRACE("\r\n === msbc_audio_init === \r\n");

    sbc_result = SBC_Decoder_decode_Init (&msbc_decoder_config);

    if (ZERO >= sbc_result)
    {
        WICED_BT_TRACE ("msbc_audio_init(): SBC_Decoder_decode_Init error: %d\r\n",
            sbc_result);
        return false;
    }

    msbc_frame_len = get_msbc_frame_len();
    WICED_BT_TRACE("msbc_frame_len = %d\r\n", msbc_frame_len);

    WICED_BT_TRACE("nblocks %d nchannels %d nsubbands %d ameth %d freq %d format %u \r\n", 
        msbc_decoder_config.numOfBlocks , 
        msbc_decoder_config.numOfChannels, 
        msbc_decoder_config.numOfSubBands,
        msbc_decoder_config.allocationMethod, sample_rate, format);

    pcm_bytes_per_frame = msbc_decoder_config.numOfBlocks *
        msbc_decoder_config.numOfChannels *
        msbc_decoder_config.numOfSubBands * 
        TWO;

    WICED_BT_TRACE("pcm_bytes_per_frame = %d\r\n",pcm_bytes_per_frame);

    /*
     *  nblocks 15 nchannels 1 nsubbands 8 ameth ZERO freq 16000 format 2
     *  pcm_bytes_per_frame = 240
    */

    encoder_q_data.cmd = CMD_ENCODER_INIT;
    encoder_q_data.in_data = (uint8_t *) &msbc_encoder_params;
    encoder_q_data.in_data_len = sizeof(msbc_encoder_params);

    if (pdTRUE != xQueueSend(audio_encoder_task_q, 
        &encoder_q_data, portMAX_DELAY))
    {
        printf(" Error:Audio Encoder Init Queue Send failed**********\r\n");
    }
    xSemaphoreTake(audio_encoder_semaphore, portMAX_DELAY);

    encoder_q_data.cmd = CMD_ENCODER_CONFIG_IN_SIZE;
    encoder_q_data.in_data = (uint8_t *) &recommended_in_size;
    encoder_q_data.in_data_len = sizeof(recommended_in_size);

    if (pdTRUE != xQueueSend(audio_encoder_task_q, &encoder_q_data, portMAX_DELAY))
    {
        printf("Error:Audio Encoder Config In size Queue Send failed****\r\n");
    }
    xSemaphoreTake(audio_encoder_semaphore, portMAX_DELAY);

    encoder_q_data.cmd = CMD_ENCODER_CONFIG_OUT_SIZE;
    encoder_q_data.in_data = (uint8_t *) &min_out_size;
    encoder_q_data.in_data_len = sizeof(min_out_size);

    if (pdTRUE != xQueueSend(audio_encoder_task_q, &encoder_q_data, portMAX_DELAY))
    {
         printf("Error:Audio Encoder Config In size Queue Send failed***\r\n");
    }
    xSemaphoreTake(audio_encoder_semaphore, portMAX_DELAY);

    WICED_BT_TRACE("Audio Encoder initialized!\r\n");
    WICED_BT_TRACE("    recommended_in_size = %u\r\n", recommended_in_size);
    WICED_BT_TRACE("    min_out_size = %u\r\n\r\n", min_out_size);

    /* Create a message queue to communicate data / commands between various tasks
     * and callbacks.
     */
    audio_tx_q = xQueueCreate(AUDIO_TX_QUEUE_LENGTH, sizeof(audio_tx_q_data_t));
 
    current_rx_pcm_index = 0;
    next_rx_pcm_frame_num_to_be_sent = 0;
    current_rx_pcm_ptr = rx_pcm_data;

   return true;
}

/*******************************************************************************
* Function Name: msbc_write_pcm_data
********************************************************************************
* Summary:
*   Writes the supplied PCM frames to MSBC Audio driver
*
* Parameters:
*   p_rx_media: The PCM buffer to be written
*   media_len : Length of p_rx_media data
*
* Return:
*   NONE
*
*******************************************************************************/
void msbc_write_pcm_data(uint8_t* p_rx_media, uint16_t media_len)
{
   /* Typically we get media_len = 60 */
   if ((NULL == p_rx_media) || (ZERO == media_len))
   {
       return;
   }

   if (ONE == media_len)
   {
       return;
   }

   msbc_decode(p_rx_media, media_len);
}

/*******************************************************************************
* Function Name: capture_mic_data
********************************************************************************
* Summary:
*  Reads mic data from MSBC Audio driver
*
* Parameters:
*  uint8_t *mic_data : buffer to fill the mic data (Out Parameter)
*
* Return:
*  uint32_t : return the actual bytes read from mic
*
*******************************************************************************/
uint32_t capture_mic_data(uint8_t *mic_data, uint32_t length)
{
    if (!mic_capture_active)
    {
        return ZERO;
    }

    if (pdTRUE == xQueueReceive(audio_tx_q, &audio_tx_q_data, portMAX_DELAY))
    {
        if (audio_tx_q_data.data_len == min_out_size)
        {
            memset(mic_data, ZERO, SCO_OUT_FRAME_SIZE);

            *(mic_data ++) = msbc_tx_first_byte;
            *(mic_data ++) = msbc_tx_second_byte[msbc_tx_second_byte_index++];

            if (msbc_tx_second_byte_index >= sizeof(msbc_tx_second_byte))
            {
                msbc_tx_second_byte_index = ZERO;
            }

            memcpy((uint8_t *)mic_data, (uint8_t *) audio_tx_q_data.data, 
                   audio_tx_q_data.data_len);
        }
    }
    return SCO_OUT_FRAME_SIZE;
}

/*******************************************************************************
* Function Name: start_audio_capture
********************************************************************************
* Summary:
*  Start to capture audio from microphone
*
* Parameters:
*  void: none
*
* Return:
*  bool : true: successful, false: failed
*
*******************************************************************************/
bool start_audio_capture(void)
{
    xQueueReset(audio_tx_q);

    current_tx_pcm_index = 0;
    current_tx_sbc_index = 0;
    mic_capture_active = true;

    app_pdm_pcm_activate();

    return true;
}


/*******************************************************************************
* Function Name: stop_audio_capture
********************************************************************************
* Summary:
*  Stop to capture audio from microphone 
*
* Parameters:
*  void: none
*
* Return:
*  bool : true: successful, false: failed
*
*******************************************************************************/
bool stop_audio_capture(void)
{
    app_pdm_pcm_deactivate();

    current_tx_pcm_index = 0;
    current_tx_sbc_index = 0;

    xQueueReset(audio_tx_q);

    mic_capture_active = false;

    return true;
}


/*******************************************************************************
* Function Name: start_audio_playback
********************************************************************************
* Summary:
*  Start the audio playback 
*
* Parameters:
*  void: none
*
* Return:
*  None
*
*******************************************************************************/
void start_audio_playback(void)
{
    xQueueReset(i2s_playback_task_q);

    /* Enable I2S */
    app_i2s_enable();
    
    /* Activate and enable I2S TX interrupts */
    app_i2s_activate();

    current_rx_pcm_index = 0;
    next_rx_pcm_frame_num_to_be_sent = 0;
    current_rx_pcm_ptr = rx_pcm_data;
}

/*******************************************************************************
* Function Name: stop_audio_playback
********************************************************************************
* Summary:
*  Stop the audio playback 
*
* Parameters:
*  void: none
*
* Return:
*  None
*
*******************************************************************************/
void stop_audio_playback(void)
{
    xQueueReset(i2s_playback_task_q);

    app_i2s_deactivate();

    app_i2s_disable();

    current_rx_pcm_index = 0;
    next_rx_pcm_frame_num_to_be_sent = 0;
    current_rx_pcm_ptr = rx_pcm_data;
}

/*******************************************************************************
* Function Name: get_msbc_frame_len
********************************************************************************
* Summary:
*  Calculates the frame length based on the CODEC parameters chosen
*
* Parameters:
*  None
*
* Return:
*  Frame Length
*
*******************************************************************************/
static uint16_t get_msbc_frame_len(void)
{
    uint32_t frame_len;
    uint32_t tmp;
    uint8_t join = (msbc_decoder_config.channelMode == SBC_JOINT_STEREO) ?
                    SBC_DUAL : SBC_MONO;

    frame_len = MSBC_SYNCWORD_SIZE_BYTES + 
        (MSBC_BITS_PER_NIBBLE * msbc_decoder_config.numOfSubBands * 
            msbc_decoder_config.numOfChannels) / MSBC_BITS_PER_BYTE;

    /* channel Mode is not Join */
    if (SBC_STEREO > msbc_decoder_config.channelMode) 
    {
        tmp = (msbc_decoder_config.numOfBlocks * 
            msbc_decoder_config.numOfChannels * 
            msbc_decoder_config.bitPool);
    }
    else
    {
        tmp = (join * msbc_decoder_config.numOfSubBands) + 
            (msbc_decoder_config.numOfBlocks *
            msbc_decoder_config.bitPool);
    }

    frame_len += tmp / MSBC_BITS_PER_BYTE + 
        ((tmp % MSBC_BITS_PER_BYTE) ? ONE : ZERO);

    return ((uint16_t)frame_len);
}

/*******************************************************************************
* Function Name: msbc_decode
********************************************************************************
* Summary:
*  decoder function that takes an mSBC frame and outputs PCM samples.
*          
* Parameters:
*  p_rx_media - media data
*  media_len - data length
*
* Return:
*  None
*
*******************************************************************************/
void msbc_decode(uint8_t* p_rx_media, uint32_t media_len)
{
    SINT16 sbc_result = 0;
    uint8_t *pIn = NULL;
    uint8_t *pcm_ptr_to_be_sent = NULL;
    uint32_t pcm_frame_num = 0;
    static uint32_t j = 0;

    if (msbc_frame_len > media_len || 60 < media_len)
    {
        WICED_BT_TRACE("msbc_decode(): invalid media_len=%u\n", media_len);
        return;
    }

    pIn = p_rx_media + mSBC_START_OFFSET;

    /* Decode one mSBC frame into PCM samples */
    sbc_result = SBC_Decoder_decoder(&msbc_decoder_config, pIn, msbc_frame_len,
        (SINT16 *)current_rx_pcm_ptr);

    if (0 >= sbc_result)
    {
        WICED_BT_TRACE("msbc_decode(): decode error=%d\n", sbc_result);
        return;
    }


    if (sbc_result != (pcm_bytes_per_frame / 2))
    {
        printf ("msbc_decode():SBC_Decoder_decoder output length error: sbc_result (%d) != (pcm_bytes_per_frame / 2) (%d)\r\n", 
            sbc_result, (pcm_bytes_per_frame / 2));
        return;
    }

    current_rx_pcm_ptr += pcm_bytes_per_frame;
    current_rx_pcm_index += pcm_bytes_per_frame;

    pcm_frame_num = (current_rx_pcm_index / RX_PCM_FRAME_SIZE);

    if (pcm_frame_num > next_rx_pcm_frame_num_to_be_sent)
    {
        for (uint32_t i = next_rx_pcm_frame_num_to_be_sent; i < pcm_frame_num; i++)
        {
            while( j <= (i * RX_PCM_FRAME_SIZE))
            {
                pcm_ptr_to_be_sent = &(rx_pcm_data[j]);
                i2s_playback_q_data_hfp.data = (uint8_t *) pcm_ptr_to_be_sent;
                i2s_playback_q_data_hfp.data_len = 160 * sizeof(int16_t); 
                if (pdTRUE != xQueueSend(i2s_playback_task_q, 
                    &i2s_playback_q_data_hfp, portMAX_DELAY))
                {
                    printf(">>> Error:I2S Task Queue Send failed\r\n");
                }
                j += RX_PCM_FRAME_SIZE_BYTES;
            }
        }
        next_rx_pcm_frame_num_to_be_sent = pcm_frame_num;
    }

    if (current_rx_pcm_index >= RX_PCM_BUFFER_SIZE)
    {
        current_rx_pcm_index = 0;
        next_rx_pcm_frame_num_to_be_sent = 0;
        j = 0;
        current_rx_pcm_ptr = rx_pcm_data;
    }
}

/* [] END OF FILE */
