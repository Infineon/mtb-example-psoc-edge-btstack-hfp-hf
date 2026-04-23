/*******************************************************************************
* File Name : app_i2s.c
*
* Description : Source file for Audio Playback via I2S.
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
#include "app_i2s.h"
#include "retarget_io_init.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "handsfree_unit_evt_handler.h"
#include "circular_buffer.h"
#include "audio_conv_utils.h"
#include "audio_aec_utils.h"

/*******************************************************************************
* Macros
*******************************************************************************/
#define ZERO                            (0U)
#define SCO_FRAME_BYTES                 (320U)
#define QUEUE_LENGTH                    (50U)
#define I2S_PLAYBACK_TASK_PRIORITY      (6U)
#define I2S_PLAYBACK_TASK_STACK_SIZE    (4U * 256U)
#ifndef FRAME_SIZE
#define FRAME_SIZE                      (320u)
#endif
#define PLAYBACK_DATA_FRAME_SIZE        (2*FRAME_SIZE)

#define AEC_REF_FRAME_SIZE              (160u)

/*******************************************************************************
* Global Variables
*******************************************************************************/
extern bool mic_capture_active;

volatile uint32_t sco_audio_size = ZERO;
volatile uint32_t sco_audio_read_ptr = ZERO;
volatile uint32_t sco_audio_write_ptr = ZERO;
uint16_t zeros_data[HW_FIFO_HALF_SIZE/2] = {ZERO};

cy_stc_scb_i2c_context_t CYBSP_I2C_CONTROLLER_context;

int16_t sco_audio_buffer[SCO_PCM_BUFFER_SIZE] = {ZERO};

TaskHandle_t i2s_task_handle;

/* Queue handle for I2S playback */
QueueHandle_t i2s_playback_task_q;

/* Circular buffer variables */
circular_buffer_t aec_ref_cbuf;
int16_t* aec_ref_cb_ptr = NULL;

mtb_hal_i2c_cfg_t i2c_config = 
{
    .is_target = false,
    .address = I2C_ADDRESS,
    .frequency_hz = I2C_FREQUENCY_HZ,
    .address_mask = MTB_HAL_I2C_DEFAULT_ADDR_MASK,
    .enable_address_callback = false
};

/* I2S transmit interrupt configurations */
const cy_stc_sysint_t i2s_isr_txcfg = 
{
    .intrSrc = (IRQn_Type) tdm_0_interrupts_tx_0_IRQn,
    .intrPriority = I2S_ISR_PRIORITY
};


int16_t i2s_aec_buffer[FRAME_SIZE] = {0};
uint32_t i2s_tx_frame_count = 0;
int8_t i2s_write_flag = 0;
int8_t valid_audio_frame = 0;
extern int8_t bulk_delay_measurement;

/*******************************************************************************
* Static Variables
*******************************************************************************/
static mtb_hal_i2c_t CYBSP_I2C_CONTROLLER_hal_obj;

/*******************************************************************************
* Function prototypes
*******************************************************************************/    
static void create_i2s_playback_queue(void);

/*******************************************************************************
* Function Definitions
*******************************************************************************/
/*******************************************************************************
* Function Name: app_i2s_init
********************************************************************************
* Summary:
*  Initializes I2S and registers I2S interrupt handler
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
void app_i2s_init(void)
{   
    /* Initialize the MCLK Pin */
   (void)Cy_GPIO_Pin_FastInit(GPIO_PRT21, 3U, CY_GPIO_DM_STRONG_IN_OFF, 0x00U, 
        P21_3_TDM_TDM_TX_MCK0);

    /* Initialize the I2S interrupt */
    Cy_SysInt_Init(&i2s_isr_txcfg, i2s_tx_interrupt_handler);

   /* Initialize the I2S */
    cy_en_tdm_status_t volatile return_status = Cy_AudioTDM_Init(TDM_STRUCT0, 
                                            &CYBSP_TDM_CONTROLLER_0_config);
    if (CY_TDM_SUCCESS != return_status)
    {
        CY_ASSERT(ZERO);
    }

    /* Clear TX interrupts */
    Cy_AudioTDM_ClearTxInterrupt(TDM_STRUCT0_TX, CY_TDM_INTR_TX_MASK);
    Cy_AudioTDM_SetTxInterruptMask(TDM_STRUCT0_TX, CY_TDM_INTR_TX_MASK);

    create_i2s_playback_queue();
}

/*******************************************************************************
* Function Name: app_tlv_codec_init
********************************************************************************
* Summary:
*  Initializes the I2C and TLV codec. 
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
void app_tlv_codec_init(void)
{
    /* Initialize I2C used to configure TLV codec */
    tlv_codec_i2c_init();
    /* TLV codec/ MW init */
    mtb_tlv320dac3100_init(&CYBSP_I2C_CONTROLLER_hal_obj);
    /* Configure internal clock dividers to achieve desired sample rate */
    mtb_tlv320dac3100_configure_clocking(MCLK_HZ, PLAYBACK_SAMPLE_RATE_HZ, 
        I2S_WORD_LENGTH, TLV320DAC3100_SPK_AUDIO_OUTPUT);

    
    mtb_tlv320dac3100_adjust_speaker_output_volume(I2S_TLV_CODEC_VOLUME);
/* Activate TLV320DAC3100 */
    mtb_tlv320dac3100_activate();
}

/*******************************************************************************
* Function Name: tlv_codec_i2c_init
********************************************************************************
* Summary:
*  Initialize I2C used for the TLV codec configurations
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/

void tlv_codec_i2c_init(void)
{
    cy_en_scb_i2c_status_t result;
    cy_rslt_t hal_result;

    /* Initialize and enable the I2C*/
    result = Cy_SCB_I2C_Init(CYBSP_I2C_CONTROLLER_HW, 
        &CYBSP_I2C_CONTROLLER_config, &CYBSP_I2C_CONTROLLER_context);
    if(CY_SCB_I2C_SUCCESS != result)
    {
        CY_ASSERT(0);
    }
    /* Enable I2C hardware. */
    Cy_SCB_I2C_Enable(CYBSP_I2C_CONTROLLER_HW);

    /* I2C HAL init */
    hal_result = mtb_hal_i2c_setup(&CYBSP_I2C_CONTROLLER_hal_obj, 
        &CYBSP_I2C_CONTROLLER_hal_config, &CYBSP_I2C_CONTROLLER_context, NULL);
    if (CY_RSLT_SUCCESS != hal_result)
    {
        CY_ASSERT(0);
    }

    /* Configure the I2C block.*/
    hal_result = mtb_hal_i2c_configure(&CYBSP_I2C_CONTROLLER_hal_obj, 
        &i2c_config);
    if (CY_RSLT_SUCCESS != hal_result)
    {
        CY_ASSERT(0);
    }
}
/*******************************************************************************
* Function Name: i2s_tx_interrupt_handler
********************************************************************************
* Summary:
*  I2S transmit interrupt handler function.
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/

void i2s_tx_interrupt_handler(void)
{
    int16_t* i2s_tx_ptr = NULL;
    i2s_playback_q_data_t i2s_playback_q_data;

    /* Get interrupt status and check for tigger interrupt and errors */
    uint32_t intr_status = 
    Cy_AudioTDM_GetTxInterruptStatusMasked(TDM_STRUCT0_TX);

    if(CY_TDM_INTR_TX_FIFO_TRIGGER & intr_status)
    {
        i2s_write_flag = 1;

        if (ZERO == i2s_tx_frame_count)
        {
            if (pdTRUE != 
                xQueueReceive(i2s_playback_task_q, &i2s_playback_q_data,0))
            {
                memset(i2s_aec_buffer, 0, sizeof(i2s_aec_buffer));
                valid_audio_frame = 0;
            }
            else
            {
                valid_audio_frame = 1;
                convert_mono_to_stereo_interleaved((uint16_t*)i2s_aec_buffer,
                    (uint16_t*)i2s_playback_q_data.data);
            }
        }

        /* Write the data from the buffer to I2S */
        i2s_tx_ptr = i2s_aec_buffer + (i2s_tx_frame_count * HW_FIFO_SIZE);
        for (uint32_t i = 0; i < HW_FIFO_SIZE; i++)
        {
            Cy_AudioTDM_WriteTxData(TDM_STRUCT0_TX, (uint32_t) i2s_tx_ptr[i]);
        }

        i2s_tx_frame_count ++;
        if (i2s_tx_frame_count >= NUM_I2S_BUFFERS_IN_AUDIO_FRAME)
        {
            i2s_tx_frame_count = 0;
            if (valid_audio_frame == 1)
            {
                aec_push(i2s_aec_buffer);
                valid_audio_frame = 0;
            }
        }

    }
    else if(CY_TDM_INTR_TX_FIFO_UNDERFLOW & intr_status)
    {
        printf("Error: I2S transmit underflowed");
    }

    /* Clear all Tx I2S Interrupt */
    Cy_AudioTDM_ClearTxInterrupt(TDM_STRUCT0_TX, CY_TDM_INTR_TX_MASK);
}
 
/*******************************************************************************
* Function Name: app_i2s_disable
********************************************************************************
* Summary:
*  Clear Tx FIFO and disable I2S
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
void app_i2s_disable(void)
{
    NVIC_DisableIRQ(i2s_isr_txcfg.intrSrc);

    Cy_AudioTDM_DisableTx(TDM_STRUCT0_TX);
}

/*******************************************************************************
* Function Name: app_i2s_activate
********************************************************************************
* Summary:
*  Activate I2S Tx interrupt
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
void app_i2s_activate(void)
{
    /* Activate and enable I2S TX interrupts */
    Cy_AudioTDM_ActivateTx(TDM_STRUCT0_TX);
}

/*******************************************************************************
* Function Name: app_i2s_enable
********************************************************************************
* Summary:
*  Enable I2S and fill TX HW FIFO
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
void app_i2s_enable(void)
{
    /* Clear TX interrupts */
    Cy_AudioTDM_ClearTxInterrupt(TDM_STRUCT0_TX, CY_TDM_INTR_TX_MASK);
    Cy_AudioTDM_SetTxInterruptMask(TDM_STRUCT0_TX, CY_TDM_INTR_TX_MASK);

    /* Start the I2S TX */                                
    Cy_AudioTDM_EnableTx(TDM_STRUCT0_TX);

    /* Fill TX FIFO before it is activated with Zeros */
    for(int i = ZERO; i < HW_FIFO_HALF_SIZE / 2; i++)
    {
        /* Write data in FIFO */
        Cy_AudioTDM_WriteTxData(TDM0_TDM_STRUCT0_TDM_TX_STRUCT, 
            (uint32_t)(zeros_data[i]));
        Cy_AudioTDM_WriteTxData(TDM0_TDM_STRUCT0_TDM_TX_STRUCT, 
            (uint32_t)(zeros_data[i]));
    }

    NVIC_EnableIRQ(i2s_isr_txcfg.intrSrc);
}

/*******************************************************************************
* Function Name: app_i2s_deactivate
********************************************************************************
* Summary:
*  Deactivate I2S Tx interrupt
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
void app_i2s_deactivate(void)
{
    /* To clear FIFO there is no direct way so Disable and Enable Tx, FIFO will 
    *be cleared as a side effect 
    */
    Cy_AudioI2S_DisableTx(TDM_STRUCT0_TX);
    Cy_AudioI2S_EnableTx(TDM_STRUCT0_TX);
    /* Once FIFO is cleared Deacivate and Disable I2S */
    Cy_AudioTDM_DeActivateTx(TDM_STRUCT0_TX);
    Cy_AudioI2S_DisableTx(TDM_STRUCT0_TX);
}

void writeI2SData(int16_t *decoded_pcm, uint32_t length)
{
    sco_audio_read_ptr = sco_audio_write_ptr;

    /* Push decoded PCM samples into circular buffer for I2S playback */
    for (uint32_t i = 0; i < length; i++)
    {
        sco_audio_buffer[sco_audio_write_ptr++] = decoded_pcm[i];

        if (sco_audio_write_ptr >= SCO_PCM_BUFFER_SIZE)
        {
            sco_audio_write_ptr = ZERO;
        }
    }

    sco_audio_size += length;
}

/*******************************************************************************
* Function Name: create_i2s_playback_queue
********************************************************************************
* Summary:
* Creates i2s playback queue
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
static void create_i2s_playback_queue(void)
{
    /* Create a send message queue to communicate audio data to i2s playback 
    task */
    i2s_playback_task_q = xQueueCreate(I2S_PLAYBACK_TASK_QUEUE_LENGTH,
        sizeof(i2s_playback_q_data_t));
}

