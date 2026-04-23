/*******************************************************************************
* File Name : app_i2s.h
*
* Description : Header file for i2s containing  function prototypes.
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
#ifndef __APP_I2S_H__
#define __APP_I2S_H__

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */

/*******************************************************************************
* Header Files
*******************************************************************************/
#include "cy_pdl.h"
#include "mtb_hal.h"
#include "cybsp.h"
#include "retarget_io_init.h"
#include "mtb_tlv320dac3100.h"
#include "audio_platform_common.h"

/*******************************************************************************
* Macros
*******************************************************************************/
/* Sampling rate in KHz */
#define PLAYBACK_SAMPLE_RATE_HZ          (TLV320DAC3100_DAC_SAMPLE_RATE_16_KHZ)
/* MLCK Value for 16KHz playback */
#define MCLK_HZ                          (2048000U)
/* I2S word length parameter */
#define I2S_WORD_LENGTH                  (TLV320DAC3100_I2S_WORD_SIZE_16)

/* I2C controller address */
#define I2C_ADDRESS                      (0x18)
/* I2C frequency in Hz */
#define I2C_FREQUENCY_HZ                 (400000U)

/* I2S hardware FIFO size */
#define I2S_HW_FIFO_SIZE                 (128U)
/* I2S hardware half FIFO size is 64 */
#define HW_FIFO_HALF_SIZE                (I2S_HW_FIFO_SIZE/2)

#define HW_FIFO_SIZE                     (I2S_HW_FIFO_SIZE / 2)

/* Number of samples (for 2 channels) in the audio frame in the queue */
#define FRAME_SIZE                       (2 * 160u)

/* Number of I2S TX transactions per audio frame */
#define NUM_I2S_BUFFERS_IN_AUDIO_FRAME   (FRAME_SIZE / HW_FIFO_SIZE)

/* I2S interrupt priority */
#define I2S_ISR_PRIORITY                 (2U)

#define SCO_PCM_CHANNELS                 (1U)       /* Mono */
#define SCO_PCM_FRAME_MS                 (15U)
#define SCO_PCM_FRAME_SIZE               ((PLAYBACK_SAMPLE_RATE_HZ * \
                                           SCO_PCM_FRAME_MS) / 1000U)
#define SCO_PCM_NUM_FRAMES               (4U) /* 4 frames = 60 ms buffering */
#define SCO_PCM_BUFFER_SIZE              (SCO_PCM_FRAME_SIZE * \
                                           SCO_PCM_NUM_FRAMES * \
                                           SCO_PCM_CHANNELS)
#define SCO_PCM_SAMPLE_BYTES             (2U)
#define SCO_AUDIO_CHANNELS               (2U)       /* Dual mono for I2S */

#define I2S_PLAYBACK_TASK_QUEUE_LENGTH   (100U)

#define I2S_TLV_CODEC_VOLUME             (110)

/* Data-type for I2S playback task's queue data */
typedef struct
{
    uint32_t data_len;
    uint8_t* data;
} i2s_playback_q_data_t;

/*******************************************************************************
* Extern variables
*******************************************************************************/
extern QueueHandle_t i2s_playback_task_q;

/*******************************************************************************
* Functions Prototypes
*******************************************************************************/
void app_i2s_init(void);
void app_tlv_codec_init(void);
void i2s_tx_interrupt_handler(void);
void app_i2s_enable(void);
void app_i2s_disable(void);
void app_i2s_activate(void);
void app_i2s_deactivate(void);
void tlv_codec_i2c_init(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __APP_I2S_H__ */

/* [] END OF FILE */
