/*******************************************************************************
* File Name: handsfree_unit_evt_handler.h
*
* Description: This is the source code for Handsfree Unit CE.
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
#ifndef __APP_HANDSFREE_EVT_HANDLER_H__
#define __APP_HANDSFREE_EVT_HANDLER_H__

/*******************************************************************************
* Header file includes
*******************************************************************************/
#include <stdio.h>
#include "wiced_bt_cfg.h"
#include "wiced_bt_hfp_hf_int.h"
#include "wiced_bt_utils.h"
#include "handsfree_unit.h"
#include "FreeRTOS.h"
#include "semphr.h"

/*******************************************************************************
* Variable Definitions
*******************************************************************************/
extern bluetooth_hfp_context_t handsfree_ctxt_data;
extern handsfrees_app_globals handsfree_app_states;
extern SemaphoreHandle_t bt_semaphore;

#endif /* __APP_HANDSFREE_EVT_HANDLER_H__ */

/* [] END OF FILE */
