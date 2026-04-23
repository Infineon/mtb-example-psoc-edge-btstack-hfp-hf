/******************************************************************************
* File Name : audio_aec_utils.h
*
* Description :
* Header file for AEC reference Queue management 
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
#ifndef AUDIO_AEC_UTILS_H
#define AUDIO_AEC_UTILS_H

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

/*******************************************************************************
* Header Files
*******************************************************************************/
#include "cyabs_rtos.h"


/*******************************************************************************
* Macros
*******************************************************************************/

#define AEC_QUEUE_FAILURE   (-1)

/*******************************************************************************
* AEC Reference Queue APIs
*******************************************************************************/

/**
 * @brief Initialize AEC reference queue.
 *
 * Must be called during system initialization.
 */
void audio_aec_ref_init(void);

/**
 * @brief Push 10 ms speaker reference frame to AEC queue.
 *
 * @param item_ptr Pointer to 10 ms mono PCM data
 * @return cy_rslt_t
 */
cy_rslt_t aec_push(short* item_ptr);

/**
 * @brief Pop 10 ms speaker reference frame for AEC processing.
 *
 * @param item_ptr   Output buffer
 * @param bulk_delay Number of frames to delay before popping
 * @return cy_rslt_t
 */
cy_rslt_t aec_pop(short* item_ptr, int bulk_delay);

/**
 * @brief Flush AEC reference queue.
 */
void aec_flush(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* AUDIO_AEC_UTILS_H */

/* [] END OF FILE */
