/*******************************************************************************
* File Name:   sbc_encoder_api.c
*
* Description: Source file for SBC encoder.
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
#include "sbc_encoder_api.h"

/*******************************************************************************
* Function definitions
*******************************************************************************/
/*******************************************************************************
* Function Name: sbc_encoder_app_init()
********************************************************************************
* Summary:
*  Initialize SBC encoder MW.
*
* Parameters:
*  SBC configurations
*  enc_handle - encoder handle pointer
*  sbc_encoder_params - encoder parameter structure
*
* Return:
*  Result of initialization.
*
*******************************************************************************/
int sbc_encoder_app_init(cy_audio_sw_codec_t *enc_handle, 
    sbc_encoder_params_t *sbc_encoder_params)
{
    cy_rslt_t                           result;
    cy_audio_sw_codec_encode_config_t   enc_config;
    cy_asc_sbc_encode_config_t          sbc_config;

    enc_config.bitwidth          = cy_audio_sw_codec_get_bitwidth_enum(
                                    sbc_encoder_params->bit_width);
    enc_config.channel           =  cy_audio_sw_codec_get_channel_enum(
                                    sbc_encoder_params->channels);
    enc_config.sampling_rate     = cy_audio_sw_codec_get_sampling_rate_enum(
                                    sbc_encoder_params->sample_rate);
    enc_config.bitrate           = CY_AUDIO_CODEC_BITRATE_UNKNOWN;
    enc_config.frame_ms          = CY_AUDIO_CODEC_FRAME_MS_UNKNOWN;
    enc_config.codec_type        = CY_AUDIO_CODEC_TYPE_SBC;

    sbc_config.allocation_method = 
        sbc_encoder_params->sbc_config.allocation_method;
    sbc_config.sbc_mode          = sbc_encoder_params->sbc_config.sbc_mode;
    sbc_config.sub_blocks        = sbc_encoder_params->sbc_config.sub_blocks;
    sbc_config.sub_bands         = sbc_encoder_params->sbc_config.sub_bands;
    sbc_config.bit_pool          = sbc_encoder_params->sbc_config.bit_pool;
    sbc_config.channel_mode      = sbc_encoder_params->sbc_config.channel_mode;
    sbc_config.uui_id            = sbc_encoder_params->sbc_config.uui_id;

    printf("***SBC config parameters*** \r\n");
    printf("Allocation method %d \r\n",sbc_config.allocation_method);
    printf("SBC mode %d \r\n",sbc_config.sbc_mode);
    printf("Sub blocks %d \r\n",sbc_config.sub_blocks);
    printf("Sub bands %d \r\n",sbc_config.sub_bands);
    printf("Bit Pool %d \r\n",sbc_config.bit_pool);
    printf("Channel Mode %d \r\n",sbc_config.channel_mode);

    enc_config.codec_specific_config.sbc = sbc_config;

    result = cy_audio_sw_codec_init(CY_AUDIO_CODEC_OPERATION_ENCODE, 
        (void*)&enc_config, enc_handle);

    if (CY_RSLT_SUCCESS != result)
    {
        cy_asc_log_err(result, "sw encoder init failed");
    } 
    else
    {
        cy_asc_log_dbg("sw encoder init success");
    }

    return result;

}

/*******************************************************************************
* Function Name: sbc_encoder_app_encode()
********************************************************************************
* Summary:
*  Encodes a buffer of PCM data to SBC.
*
* Parameters:
*  Buffers for input/output.
*  enc_handle - encoder handle
*  input_buff - input buffer
*  in_size - input size
*  output_buff - output buffer
*  out_size - output size
*
* Return:
*  Result of encoding.
*
*******************************************************************************/
int sbc_encoder_app_encode(cy_audio_sw_codec_t enc_handle, uint8_t* input_buff, 
    uint32_t* in_size, uint8_t * output_buff, uint32_t* out_size)
{
    cy_rslt_t   result;

    result = cy_audio_sw_codec_encode(enc_handle, (uint8_t *)input_buff, 
        (uint32_t*)in_size, (uint8_t*)output_buff, (uint32_t*)out_size);

    if(CY_RSLT_SUCCESS != result)
    {
        cy_asc_log_err(result, "Encoding failed");
    }

    return result;
}

/* [] END OF FILE */
