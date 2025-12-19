/*******************************************************************************
* File Name: audio_platform_common.h
*
* Description: This file contains the declarations of data types and functions
*              for PSOC EDGE specific audio.
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
* Include guard
*******************************************************************************/
#ifndef AUDIO_PLATFORM_COMMON_H_
#define AUDIO_PLATFORM_COMMON_H_

/*******************************************************************************
* Header file includes
*******************************************************************************/
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include <stdio.h>
#include <assert.h>
#include <stdbool.h>
#include "wiced_memory.h"
#include "cy_pdm_pcm_v2.h"

/*******************************************************************************
* Macros
*******************************************************************************/
#define NBS_SAMPLE_RATE                       (8000U)
#define WBS_SAMPLE_RATE                       (16000U)

#define ENCODER_BIT_WIDTH                     (16U)
#define ENCODER_SUB_BANDS                     (8U)
#define ENCODER_SUB_BLOCKS                    (15U)
#define ENCODER_BIT_POOL                      (26U)
/* UUI2: 2EV3, TeSCO 12: 57 Bytes */
#define ENCODER_UUI_ID                        (2U)

/* = 120 bytes of decoded data received over SCO */
#define RX_PCM_BUFFER_SIZE                    (120U)

/* For 16kHz, 30 samples (60 bytes) is 1.875 ms data. 
 * PDM PCM RX size is TX_PCM_FRAME_SIZE.
 * Total buffer size is TX_PCM_BUFFER_SIZE */

/* = 240 bytes = 7.5 ms of 16kHz sampled data */
#define SBC_ENC_IN_FRAME_SIZE                 (120 * 1 * sizeof(int16_t))               

/* = 60 bytes of encoded data to be sent over SCO */
#define SCO_OUT_FRAME_SIZE                    (60U)                                      

/* = 57 bytes of encoded output for every 240 byte PCM input */
#define SBC_ENC_OUT_FRAME_SIZE                (57U)                                      

/* = 15 ms of 16kHz sampled data = 2 SBC frames for encoding */
#define TX_PCM_FRAME_SIZE                     (2 * SBC_ENC_IN_FRAME_SIZE)               

/* = 120 ms of 16kHz sampled data */
#define TX_PCM_BUFFER_SIZE                    (8UL * TX_PCM_FRAME_SIZE)                 
#define TX_SBC_FRAME_SIZE                     (2 * SBC_ENC_OUT_FRAME_SIZE)
#define TX_SBC_BUFFER_SIZE                    (8UL * TX_SBC_FRAME_SIZE)

#define AUDIO_TX_QUEUE_LENGTH                 (50U)

/* Gain to Scale mapping */

#define PDM_PCM_SEL_GAIN_83DB                 (83.0)
#define PDM_PCM_SEL_GAIN_77DB                 (77.0)
#define PDM_PCM_SEL_GAIN_71DB                 (71.0)
#define PDM_PCM_SEL_GAIN_65DB                 (65.0)
#define PDM_PCM_SEL_GAIN_59DB                 (59.0)
#define PDM_PCM_SEL_GAIN_53DB                 (53.0)
#define PDM_PCM_SEL_GAIN_47DB                 (47.0)
#define PDM_PCM_SEL_GAIN_41DB                 (41.0)
#define PDM_PCM_SEL_GAIN_35DB                 (35.0)
#define PDM_PCM_SEL_GAIN_29DB                 (29.0)
#define PDM_PCM_SEL_GAIN_23DB                 (23.0)
#define PDM_PCM_SEL_GAIN_17DB                 (17.0)
#define PDM_PCM_SEL_GAIN_11DB                 (11.0)
#define PDM_PCM_SEL_GAIN_5DB                  (5.0)
#define PDM_PCM_SEL_GAIN_NEGATIVE_1DB         (-1.0)
#define PDM_PCM_SEL_GAIN_NEGATIVE_7DB         (-7.0)
#define PDM_PCM_SEL_GAIN_NEGATIVE_13DB        (-13.0)
#define PDM_PCM_SEL_GAIN_NEGATIVE_19DB        (-19.0)
#define PDM_PCM_SEL_GAIN_NEGATIVE_25DB        (-25.0)
#define PDM_PCM_SEL_GAIN_NEGATIVE_31DB        (-31.0)
#define PDM_PCM_SEL_GAIN_NEGATIVE_37DB        (-37.0)
#define PDM_PCM_SEL_GAIN_NEGATIVE_43DB        (-43.0)
#define PDM_PCM_SEL_GAIN_NEGATIVE_49DB        (-49.0)
#define PDM_PCM_SEL_GAIN_NEGATIVE_55DB        (-55.0)
#define PDM_PCM_SEL_GAIN_NEGATIVE_61DB        (-61.0)
#define PDM_PCM_SEL_GAIN_NEGATIVE_67DB        (-67.0)
#define PDM_PCM_SEL_GAIN_NEGATIVE_73DB        (-73.0)
#define PDM_PCM_SEL_GAIN_NEGATIVE_79DB        (-79.0)
#define PDM_PCM_SEL_GAIN_NEGATIVE_85DB        (-85.0)
#define PDM_PCM_SEL_GAIN_NEGATIVE_91DB        (-91.0)
#define PDM_PCM_SEL_GAIN_NEGATIVE_97DB        (-97.0)
#define PDM_PCM_SEL_GAIN_NEGATIVE_103DB       (-103.0)

/*******************************************************************************
* Structures and Enumerations
*******************************************************************************/
typedef struct audio_playback_config_TAG
{
    int16_t samplingFreq;        /* 16k, 32k, 44.1k or 48k */
    int16_t channelMode;         /* mono, dual, streo or joint streo */
    int16_t numOfSubBands;       /* 4 or 8 */
    int16_t numOfChannels;
    int16_t numOfBlocks;         /* 4, 8, 12 or 16 */
    int16_t allocationMethod;    /* loudness or SNR */
    int16_t bitPool;
    int16_t sbc_mode;           /* 0: A2DP mode, 1: WideBand mode */
    int16_t uui_id;
} audio_playback_config;

/* Data-type for Audio TX queue data */
typedef struct
{
    uint32_t data_len;  /* Number of Samples */
    uint8_t* data;     /* Pointer to the start of the buffer */
} audio_tx_q_data_t;

/*******************************************************************************
* Variable Definitions
*******************************************************************************/
extern QueueHandle_t audio_tx_q;

/*******************************************************************************
* Function prototypes
*******************************************************************************/
bool msbc_audio_init(audio_playback_config pb_config_params);
void msbc_write_pcm_data(uint8_t* p_rx_media, uint16_t media_len);
uint32_t capture_mic_data(uint8_t *mic_data, uint32_t _frames);
bool start_audio_capture(void);
bool stop_audio_capture(void);
void msbc_decode(uint8_t* p_rx_media, uint32_t media_len);
void start_audio_playback(void);
void stop_audio_playback(void);
void app_pdm_pcm_activate(void);
void app_pdm_pcm_deactivate(void);
cy_en_pdm_pcm_gain_sel_t convert_db_to_pdm_scale(double db);
void set_pdm_pcm_gain(cy_en_pdm_pcm_gain_sel_t gain);

#endif /* AUDIO_PLATFORM_COMMON_H_ */

/* [] END OF FILE */
