/*******************************************************************************
* File Name:   sbc_encoder_api.h
*
* Description: Header file for SBC encoder.
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
 ******************************************************************************/
#ifndef SBC_ENCODER_API_H_
#define SBC_ENCODER_API_H_

/*******************************************************************************
 * Header file includes
 ******************************************************************************/
#include "cy_audio_sw_codecs.h"
#include "cy_audio_sw_codec_utils.h"
#include "cy_asc_config_sbc_encoder.h"
#include "cy_asc_config_sbc_decoder.h"

/*******************************************************************************
* Structures
*******************************************************************************/
typedef struct sbc_encoder_params
{
    uint32_t sample_rate;
    uint8_t  channels;
    uint8_t  bit_width;
    cy_asc_sbc_encode_config_t  sbc_config;

} sbc_encoder_params_t;

/*******************************************************************************
* Function prototypes
*******************************************************************************/
int sbc_encoder_app_init(cy_audio_sw_codec_t *enc_handle, 
    sbc_encoder_params_t *sbc_encoder_params);
int sbc_encoder_app_encode(cy_audio_sw_codec_t enc_handle, uint8_t* input_buff, 
    uint32_t* in_size, uint8_t * output_buff, uint32_t* out_size);
void sbc_encoder_app_deinit();

#endif /* SBC_ENCODER_API_H_ */

/* [] END OF FILE */
