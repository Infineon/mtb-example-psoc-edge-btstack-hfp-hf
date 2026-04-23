/*******************************************************************************
* File Name: handsfree_unit_main.h
*
* Description: This is the source code for Handsfree Unit CE.
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
* Include guard
*******************************************************************************/
#ifndef __APP_HANDSFREE_MAIN_H__
#define __APP_HANDSFREE_MAIN_H__

/*******************************************************************************
* Header file includes
*******************************************************************************/
#include <stdio.h>
#include "wiced_bt_cfg.h"
#include "wiced_bt_hfp_hf_int.h"
#include "wiced_bt_utils.h"
#include "handsfree_unit.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

/*******************************************************************************
* Macros
*******************************************************************************/

/*******************************************************************************
* Structures
*******************************************************************************/
/* Data-type for HFP task queue data */
typedef struct
{
    hfp_user_cmd_t user_cmd;
    uint32_t data_len;
    uint8_t* data;
} hfp_task_q_data_t;

/*******************************************************************************
* Variable Definitions
*******************************************************************************/
extern QueueHandle_t hfp_task_q;

/*******************************************************************************
* Function Prototypes
*******************************************************************************/
void bt_task_create(void);

#endif /* __APP_HANDSFREE_MAIN_H__ */
