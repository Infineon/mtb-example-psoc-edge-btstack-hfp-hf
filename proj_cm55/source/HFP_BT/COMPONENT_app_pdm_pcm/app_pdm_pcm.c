/******************************************************************************
* File Name : app_pdm_pcm.c
*
* Description : Source file for PDM PCM.
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
#include "cy_utils.h"
#include "app_pdm_pcm.h"
#include "audio_encoder_task.h"
#include "audio_input_configuration.h"

/*******************************************************************************
* Macros
********************************************************************************/
#ifdef ENABLE_STEREO_INPUT_FEED
#define MIC_MODE                         (2U)
#else
#define MIC_MODE                         (1U)
#endif /* ENABLE_STEREO_INPUT_FEED */

#define TX_PCM_FRAME_SIZE                (640U)
#define PDM_PCM_HW_FIFO_SIZE             (64U)
#define PDM_PCM_FRAME_INTR_CNT           (5U)
#define MIC_RX_SAMPLES_COUNT             (160U * MIC_MODE)

#define ZERO                             (0U)
#define ONE_BYTE_BITS                    (8U)
#define ONE_BYTE_MASK                    (0xFF)

/*******************************************************************************
* Global Variables
*******************************************************************************/
/* PDM/PCM interrupt configuration */
const cy_stc_sysint_t PDM_IRQ_cfg = {
    .intrSrc      = (IRQn_Type)PDM_IRQ,
    .intrPriority = PDM_PCM_ISR_PRIORITY
};

int16_t mic_audio_app_buffer_ping[MIC_RX_SAMPLES_COUNT] = {0};
int16_t mic_audio_app_buffer_pong[MIC_RX_SAMPLES_COUNT] = {0};

int16_t* ping_pong_local_pointer = NULL;
int16_t* ping_pong_buffer_pointer = NULL;
volatile uint8_t pdm_pcm_intr_cnt = 0;

uint8_t  tx_msbc_data[TX_SBC_BUFFER_SIZE]__attribute__ ((section(".cy_socmem_data"))) = {0};
uint8_t  tx_pcm_data[TX_PCM_BUFFER_SIZE]__attribute__ ((section(".cy_socmem_data"))) = {0};

uint32_t current_tx_pcm_index = 0;
uint32_t current_tx_sbc_index = 0;

/*******************************************************************************
 * Static Variables
 *******************************************************************************/
/* Variable to send commands to Encoder task queue. */
static audio_encoder_q_data_t encoder_q_data;

/*******************************************************************************
* Function Declarations
*******************************************************************************/
extern void audio_mic_data_feed_cm55(int16_t *audio_data);
static void pdm_pcm_isr_handler(void);

/*******************************************************************************
* Function Definitions
*******************************************************************************/
/******************************************************************************
 * Function Name: app_pdm_pcm_init
 ******************************************************************************
 * Summary:
 *          Initializes the PDM-PCM hardware module.

 *
 * Parameters:
 *          None
 *
 * Return:
 *          None
 *
 ******************************************************************************/
void app_pdm_pcm_init(void)
{
    cy_en_pdm_pcm_gain_sel_t gain_scale = ZERO;

    if (CY_PDM_PCM_SUCCESS != Cy_PDM_PCM_Init(PDM0, &CYBSP_PDM_config))
    {
        CY_ASSERT(0);
    }

    Cy_PDM_PCM_Channel_Enable(PDM0, LEFT_CH_INDEX);
    Cy_PDM_PCM_Channel_Enable(PDM0, RIGHT_CH_INDEX);

    Cy_PDM_PCM_Channel_Init(PDM0, &LEFT_CH_CONFIG,  (uint8_t)LEFT_CH_INDEX);
    Cy_PDM_PCM_Channel_Init(PDM0, &RIGHT_CH_CONFIG, (uint8_t)RIGHT_CH_INDEX);

    /* Set the gain for both left and right channels. */
    gain_scale = convert_db_to_pdm_scale((double)PDM_MIC_GAIN_VALUE);
    set_pdm_pcm_gain(gain_scale);

    Cy_PDM_PCM_Channel_ClearInterrupt(PDM0, RIGHT_CH_INDEX, CY_PDM_PCM_INTR_MASK);
    Cy_PDM_PCM_Channel_SetInterruptMask(PDM0, RIGHT_CH_INDEX, CY_PDM_PCM_INTR_MASK);

    if(CY_SYSINT_SUCCESS != Cy_SysInt_Init(&PDM_IRQ_cfg, &pdm_pcm_isr_handler))
    {
        CY_ASSERT(0);
    }
    NVIC_ClearPendingIRQ(PDM_IRQ_cfg.intrSrc);
    NVIC_EnableIRQ(PDM_IRQ_cfg.intrSrc);
}


/*******************************************************************************
 * Function Name: app_i2s_activate
 *******************************************************************************
* Summary: Activate I2S Tx interrupt and enable I2S transmission
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
void app_pdm_pcm_activate(void)
{
    Cy_PDM_PCM_Activate_Channel(PDM0, LEFT_CH_INDEX);
    Cy_PDM_PCM_Activate_Channel(PDM0, RIGHT_CH_INDEX);
}


/*******************************************************************************
* Function Name: pdm_pcm_isr_handler
********************************************************************************
* Summary:
*  PDM/PCM ISR handler. Set a flag to be processed in the main loop.
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
static void pdm_pcm_isr_handler(void)
{
    static bool ping_pong = false;
    volatile uint32_t int_stat;

    /* Choose the ping pong buffer */
    if(ZERO == pdm_pcm_intr_cnt)
    {
        if (ping_pong)
        {
            ping_pong_local_pointer = mic_audio_app_buffer_ping;
        }
        else
        {
            ping_pong_local_pointer = mic_audio_app_buffer_pong;
        }
        ping_pong_buffer_pointer = ping_pong_local_pointer;
    }

    int_stat = Cy_PDM_PCM_Channel_GetInterruptStatusMasked(PDM0, 
        RIGHT_CH_INDEX);
    if(CY_PDM_PCM_INTR_RX_TRIGGER & int_stat)
    {
        for(uint8_t i=0; i < RX_FIFO_TRIG_LEVEL; i++)
        {
#ifdef ENABLE_STEREO_INPUT_FEED
            int32_t data = (int32_t)Cy_PDM_PCM_Channel_ReadFifo(PDM0, 
                LEFT_CH_INDEX);
            
            *(ping_pong_buffer_pointer) = (int16_t)(data);
            ping_pong_buffer_pointer++;

            data = (int32_t)Cy_PDM_PCM_Channel_ReadFifo(PDM0, RIGHT_CH_INDEX);
            
            *(ping_pong_buffer_pointer) = (int16_t)(data);
            ping_pong_buffer_pointer++;
#else
            int32_t data = (int32_t)Cy_PDM_PCM_Channel_ReadFifo(PDM0, 
                LEFT_CH_INDEX);
            data = (int32_t)Cy_PDM_PCM_Channel_ReadFifo(PDM0, RIGHT_CH_INDEX);
            *(ping_pong_buffer_pointer) = (int16_t)(data);
            ping_pong_buffer_pointer++;
#endif
        }

        if (pdm_pcm_intr_cnt < PDM_PCM_FRAME_INTR_CNT)
        {
            pdm_pcm_intr_cnt++;
        }

        if (PDM_PCM_FRAME_INTR_CNT == pdm_pcm_intr_cnt)
        {
            pdm_pcm_intr_cnt = 0;

            audio_mic_data_feed_cm55(ping_pong_local_pointer);

            ping_pong = !ping_pong;
            if (ping_pong)
            {
                ping_pong_local_pointer = mic_audio_app_buffer_ping;
            }
            else
            {
                ping_pong_local_pointer = mic_audio_app_buffer_pong;
            }
            ping_pong_buffer_pointer = ping_pong_local_pointer;
        }

        Cy_PDM_PCM_Channel_ClearInterrupt(PDM0, RIGHT_CH_INDEX, 
            CY_PDM_PCM_INTR_RX_TRIGGER);
    }

    if((CY_PDM_PCM_INTR_RX_FIR_OVERFLOW | CY_PDM_PCM_INTR_RX_OVERFLOW |
        CY_PDM_PCM_INTR_RX_IF_OVERFLOW | CY_PDM_PCM_INTR_RX_UNDERFLOW) & 
        int_stat)
    {
        Cy_PDM_PCM_Channel_ClearInterrupt(PDM0, RIGHT_CH_INDEX, 
            CY_PDM_PCM_INTR_MASK);
    }
}

/*******************************************************************************
 * Function Name: app_pdm_pcm_deactivate
 *******************************************************************************
* Summary: Deactivates PDM/PCM channels to stop audio capture
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
void app_pdm_pcm_deactivate(void)
{
    Cy_PDM_PCM_DeActivate_Channel(PDM0, LEFT_CH_INDEX);
    Cy_PDM_PCM_DeActivate_Channel(PDM0, RIGHT_CH_INDEX);
}

/*******************************************************************************
* Function Name: convert_db_to_pdm_scale
********************************************************************************
* Summary:
*  Converts dB to PDM scale (fixed scale from 0 to 31)
*  Refer
*
* Parameters:
*  gain  : gain in dB
*
* Return:
*  Scale value
*
*******************************************************************************/
cy_en_pdm_pcm_gain_sel_t convert_db_to_pdm_scale(double db)
{
    if (PDM_PCM_MIN_GAIN >= db)
    {
        return CY_PDM_PCM_SEL_GAIN_NEGATIVE_103DB;
    }
    else if (PDM_PCM_SEL_GAIN_NEGATIVE_103DB < db &&
        PDM_PCM_SEL_GAIN_NEGATIVE_97DB >= db)
    {
        return CY_PDM_PCM_SEL_GAIN_NEGATIVE_97DB;
    }
    else if (PDM_PCM_SEL_GAIN_NEGATIVE_97DB < db && 
        PDM_PCM_SEL_GAIN_NEGATIVE_91DB >= db)
    {
        return CY_PDM_PCM_SEL_GAIN_NEGATIVE_91DB;
    }
    else if (PDM_PCM_SEL_GAIN_NEGATIVE_91DB < db && 
        PDM_PCM_SEL_GAIN_NEGATIVE_85DB >= db)
    {
        return CY_PDM_PCM_SEL_GAIN_NEGATIVE_85DB;
    }
    else if (PDM_PCM_SEL_GAIN_NEGATIVE_85DB < db && 
        PDM_PCM_SEL_GAIN_NEGATIVE_79DB >= db)
    {
        return CY_PDM_PCM_SEL_GAIN_NEGATIVE_79DB;
    }
    else if (PDM_PCM_SEL_GAIN_NEGATIVE_79DB < db && 
        PDM_PCM_SEL_GAIN_NEGATIVE_73DB >= db)
    {
        return CY_PDM_PCM_SEL_GAIN_NEGATIVE_73DB;
    }
    else if (PDM_PCM_SEL_GAIN_NEGATIVE_73DB < db && 
        PDM_PCM_SEL_GAIN_NEGATIVE_67DB >= db)
    {
        return CY_PDM_PCM_SEL_GAIN_NEGATIVE_67DB;
    }
    else if (PDM_PCM_SEL_GAIN_NEGATIVE_67DB < db && 
        PDM_PCM_SEL_GAIN_NEGATIVE_61DB >= db)
    {
        return CY_PDM_PCM_SEL_GAIN_NEGATIVE_61DB;
    }
    else if (PDM_PCM_SEL_GAIN_NEGATIVE_61DB < db && 
        PDM_PCM_SEL_GAIN_NEGATIVE_55DB >= db)
    {
        return CY_PDM_PCM_SEL_GAIN_NEGATIVE_55DB;
    }
    else if (PDM_PCM_SEL_GAIN_NEGATIVE_55DB < db && 
        PDM_PCM_SEL_GAIN_NEGATIVE_49DB >= db)
    {
        return CY_PDM_PCM_SEL_GAIN_NEGATIVE_49DB;
    }
    else if (PDM_PCM_SEL_GAIN_NEGATIVE_49DB < db && 
        PDM_PCM_SEL_GAIN_NEGATIVE_43DB >= db)
    {
        return CY_PDM_PCM_SEL_GAIN_NEGATIVE_43DB;
    }
    else if (PDM_PCM_SEL_GAIN_NEGATIVE_43DB < db && 
        PDM_PCM_SEL_GAIN_NEGATIVE_37DB >= db)
    {
        return CY_PDM_PCM_SEL_GAIN_NEGATIVE_37DB;
    }
    else if (PDM_PCM_SEL_GAIN_NEGATIVE_37DB < db && 
        PDM_PCM_SEL_GAIN_NEGATIVE_31DB >= db)
    {
        return CY_PDM_PCM_SEL_GAIN_NEGATIVE_31DB;
    }
    else if (PDM_PCM_SEL_GAIN_NEGATIVE_31DB < db && 
        PDM_PCM_SEL_GAIN_NEGATIVE_25DB >= db)
    {
        return CY_PDM_PCM_SEL_GAIN_NEGATIVE_25DB;
    }
    else if (PDM_PCM_SEL_GAIN_NEGATIVE_25DB < db && 
        PDM_PCM_SEL_GAIN_NEGATIVE_19DB >= db)
    {
        return CY_PDM_PCM_SEL_GAIN_NEGATIVE_19DB;
    }
    else if (PDM_PCM_SEL_GAIN_NEGATIVE_19DB < db && 
        PDM_PCM_SEL_GAIN_NEGATIVE_13DB >= db)
    {
        return CY_PDM_PCM_SEL_GAIN_NEGATIVE_13DB;
    }
    else if (PDM_PCM_SEL_GAIN_NEGATIVE_13DB < db && 
        PDM_PCM_SEL_GAIN_NEGATIVE_7DB >= db)
    {
        return CY_PDM_PCM_SEL_GAIN_NEGATIVE_7DB;
    }
    else if (PDM_PCM_SEL_GAIN_NEGATIVE_7DB < db && 
        PDM_PCM_SEL_GAIN_NEGATIVE_1DB >= db)
    {
        return CY_PDM_PCM_SEL_GAIN_NEGATIVE_1DB;
    }
    else if (PDM_PCM_SEL_GAIN_NEGATIVE_1DB < db && 
        PDM_PCM_SEL_GAIN_5DB >= db)
    {
        return CY_PDM_PCM_SEL_GAIN_5DB;
    }
    else if (PDM_PCM_SEL_GAIN_5DB < db && 
        PDM_PCM_SEL_GAIN_11DB >= db)
    {
        return CY_PDM_PCM_SEL_GAIN_11DB;
    }
    else if (PDM_PCM_SEL_GAIN_11DB < db && 
        PDM_PCM_SEL_GAIN_17DB >= db)
    {
        return CY_PDM_PCM_SEL_GAIN_17DB;
    }
    else if (PDM_PCM_SEL_GAIN_17DB < db && 
        PDM_PCM_SEL_GAIN_23DB >= db)
    {
        return CY_PDM_PCM_SEL_GAIN_23DB;
    }
    else if (PDM_PCM_SEL_GAIN_23DB < db && 
        PDM_PCM_SEL_GAIN_29DB >= db)
    {
        return CY_PDM_PCM_SEL_GAIN_29DB;
    }
    else if (PDM_PCM_SEL_GAIN_29DB < db &&
        PDM_PCM_SEL_GAIN_35DB >= db)
    {
        return CY_PDM_PCM_SEL_GAIN_35DB;
    }
    else if (PDM_PCM_SEL_GAIN_35DB < db && 
        PDM_PCM_SEL_GAIN_41DB >= db)
    {
        return CY_PDM_PCM_SEL_GAIN_41DB;
    }
    else if (PDM_PCM_SEL_GAIN_41DB < db && 
        PDM_PCM_SEL_GAIN_47DB >= db)
    {
        return CY_PDM_PCM_SEL_GAIN_47DB;
    }
    else if (PDM_PCM_SEL_GAIN_47DB < db && 
        PDM_PCM_SEL_GAIN_53DB >= db)
    {
        return CY_PDM_PCM_SEL_GAIN_53DB;
    }
    else if (PDM_PCM_SEL_GAIN_53DB < db && 
        PDM_PCM_SEL_GAIN_59DB >= db)
    {
        return CY_PDM_PCM_SEL_GAIN_59DB;
    }
    else if (PDM_PCM_SEL_GAIN_59DB < db && 
        PDM_PCM_SEL_GAIN_65DB >= db)
    {
        return CY_PDM_PCM_SEL_GAIN_65DB;
    }
    else if (PDM_PCM_SEL_GAIN_65DB < db && 
        PDM_PCM_SEL_GAIN_71DB >= db)
    {
        return CY_PDM_PCM_SEL_GAIN_71DB;
    }
    else if (PDM_PCM_SEL_GAIN_71DB < db && 
        PDM_PCM_SEL_GAIN_77DB >= db)
    {
        return CY_PDM_PCM_SEL_GAIN_77DB;
    }
    else if (PDM_PCM_SEL_GAIN_77DB < db && 
        PDM_PCM_SEL_GAIN_83DB >= db)
    {
        return CY_PDM_PCM_SEL_GAIN_83DB;
    }
    else if (PDM_PCM_MAX_GAIN < db)
    {
        return CY_PDM_PCM_SEL_GAIN_83DB;
    }
    else 
    {
        return (cy_en_pdm_pcm_gain_sel_t) PDM_MIC_GAIN_VALUE;
    }
}

/*******************************************************************************
* Function Name: set_pdm_pcm_gain
********************************************************************************
* Summary:
*  Set PDM PCM gain.
*
* Parameters:
*  gain  : gain in dB
*
* Return:
*  None
*
*******************************************************************************/
void set_pdm_pcm_gain(cy_en_pdm_pcm_gain_sel_t gain)
{
    Cy_PDM_PCM_SetGain(PDM0, RIGHT_CH_INDEX, gain);
    Cy_PDM_PCM_SetGain(PDM0, LEFT_CH_INDEX, gain);
}

/*******************************************************************************
* Function Name: pdm_pcm_data_passer
********************************************************************************
* Summary:
*  Send PDM PCM data to encoder.
*
* Parameters:
*  mono  : Mono data stream
*
* Return:
*  None
*
*******************************************************************************/
void pdm_pcm_data_passer(char *mono)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    if ((current_tx_pcm_index + TX_PCM_FRAME_SIZE) > TX_PCM_BUFFER_SIZE)
    {
        current_tx_pcm_index = 0;
    }
    
    if ((current_tx_sbc_index + TX_SBC_FRAME_SIZE) > TX_SBC_BUFFER_SIZE)
    {
        current_tx_sbc_index = 0;
    }

    memcpy((uint8_t *)&(tx_pcm_data[current_tx_pcm_index]), 
        (uint8_t *)&(mono[0]), TX_PCM_FRAME_SIZE);

    encoder_q_data.cmd = CMD_ENCODE_DATA;
    encoder_q_data.in_data = (uint8_t *) &(tx_pcm_data[current_tx_pcm_index]);
    encoder_q_data.in_data_len = TX_PCM_FRAME_SIZE;

    encoder_q_data.out_data = (uint8_t *) &(tx_msbc_data[current_tx_sbc_index]);
    encoder_q_data.out_data_len = TX_SBC_FRAME_SIZE;

    current_tx_pcm_index += TX_PCM_FRAME_SIZE;
    if (current_tx_pcm_index >= TX_PCM_BUFFER_SIZE)
    {
        current_tx_pcm_index = 0;
    }

    current_tx_sbc_index += TX_SBC_FRAME_SIZE;
    if (current_tx_sbc_index >= TX_SBC_BUFFER_SIZE)
    {
        current_tx_sbc_index = 0;
    }

    /* Send command to Audio encoder task */
    xQueueSendFromISR(audio_encoder_task_q, &encoder_q_data, 
        &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}


/* [] END OF FILE */

