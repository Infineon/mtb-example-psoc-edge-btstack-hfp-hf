/*******************************************************************************
* File Name: handsfree_unit_api.c
*
* Description: This is the source code for the Handsfree Unit Example.
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
#include "handsfree_unit_api.h"
#include "handsfree_unit.h"
#include "handsfree_unit_evt_handler.h"
#include "wiced_bt_stack.h"
#include "cybt_platform_trace.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "cyabs_rtos.h"
#include <stdio.h>
#include <stdlib.h>

/*******************************************************************************
* Macros
*******************************************************************************/
#define BT_STACK_HEAP_SIZE              (0x18000)
#define HFU_INQUIRY_DURATION            (5)

#define BT_HFP_HF_AT_BUFF_LEN           (WICED_BT_HFP_HF_AT_MAX_LEN + 16U)
#define ZERO                            (0U)
#define ONE                             (1U)

/*******************************************************************************
* Global Variables
*******************************************************************************/
wiced_bt_heap_t *p_default_heap   = NULL;
extern bool incoming_call;
/*******************************************************************************
* Static Variables
*******************************************************************************/
/* Mutex for heap protection */
static SemaphoreHandle_t bt_default_heap_mutex;

/*******************************************************************************
* Function Declarations
*******************************************************************************/
static void handsfree_build_send_at_cmd( uint16_t handle, char *cmd, 
    uint8_t arg_type, uint8_t arg_format, const char *p_arg, int16_t int_arg);

/*******************************************************************************
* Function Definitions
*******************************************************************************/
/*******************************************************************************
* Function Name: bt_default_heap_mutex_init
********************************************************************************
* Summary:
*  Initialize the mutex.
*
* Parameters:
*  p_lock_context - mutex lock context
*
* Return:
*  None
*
*******************************************************************************/
void bt_default_heap_mutex_init(void)
{
    bt_default_heap_mutex = xSemaphoreCreateMutex();

    if (bt_default_heap_mutex == NULL)
    {
        /* Handle error */
        configASSERT(ZERO);
    }
}

/*******************************************************************************
* Function Name: bt_default_heap_mutex_lock
********************************************************************************
* Summary:
*  Get mutex lock
*
* Parameters:
*  p_lock_context - mutex lock context
*
* Return:
*  None
*
*******************************************************************************/
void bt_default_heap_mutex_lock(void *p_lock_context)
{
    /* Block indefinitely until mutex is available */
    xSemaphoreTake(bt_default_heap_mutex, portMAX_DELAY);
}

/*******************************************************************************
* Function Name: bt_default_heap_mutex_unlock
********************************************************************************
* Summary:
*  Set mutex / unlock mutex
*
* Parameters:
*  p_lock_context - mutex lock context
*
* Return:
*  None
*
*******************************************************************************/
void bt_default_heap_mutex_unlock(void *p_lock_context)
{
    xSemaphoreGive(bt_default_heap_mutex);
}

/*******************************************************************************
* Function Name: handsfree_application_start
********************************************************************************
* Summary:
*  Set device configuration and start BT stack initialization. The actual
*  application initialization will happen when stack reports that BT device
*  is ready.
*
* Parameters: NONE
*
* Return: NONE
*
*******************************************************************************/
void handsfree_application_start(void)
{
    wiced_result_t wiced_result;

    WICED_BT_TRACE(
        "\n\n\n*********** Handsfree Unit Application Start ***********\n");

    bt_semaphore = xSemaphoreCreateBinary();
    if (NULL == bt_semaphore)
    {
        printf(">>> BT semaphore creation failed!\r\n");
        return;
    }
    xSemaphoreGive(bt_semaphore);
    xSemaphoreTake(bt_semaphore, portMAX_DELAY);

    bt_default_heap_mutex_init();

    wiced_bt_lock_t lock = 
    {
        .p_lock_context = &bt_default_heap_mutex,
        .pf_lock_func = bt_default_heap_mutex_lock,
        .pf_unlock_func = bt_default_heap_mutex_unlock
    };

    /* Register call back and configuration with stack */
    wiced_result = wiced_bt_stack_init (handsfree_management_callback, 
        &handsfree_cfg_settings);

    /* Check if stack initialization was successful */
    if (WICED_BT_SUCCESS == wiced_result)
    {
        WICED_BT_TRACE("Bluetooth Stack Initialization Successful \n");

        /* Create default heap with mutex lock */
        p_default_heap = wiced_bt_create_heap("default_heap", NULL, 
            BT_STACK_HEAP_SIZE, &lock, WICED_TRUE);
        if (NULL == p_default_heap)
        {
            WICED_BT_TRACE("create default heap error: size %d\n", 
                BT_STACK_HEAP_SIZE);
            CY_ASSERT(ZERO);
        }
    }
    else
    {
       WICED_BT_TRACE("Bluetooth Stack Initialization failed!! \n");
       CY_ASSERT(ZERO);
    }

    cybt_platform_set_trace_level(CYBT_TRACE_ID_ALL, CYBT_TRACE_LEVEL_NONE);

    wait_init_done();
}

/*******************************************************************************
* Function Name: handsfree_handle_set_visibility
********************************************************************************
* Summary:
*  Sets the device discoverable and connectability mode
*
* Parameters:
*  unsigned int discoverability : 1-discoverable, 0-Non-Discoverable
*  unsigned int connectability : 1-Connectable, 0-Non-Connectable
*
* Return:
*  NONE
*
*******************************************************************************/
void handsfree_handle_set_visibility(unsigned int discoverability, 
    unsigned int connectability)
{
    wiced_result_t result = WICED_BT_SUCCESS;

    /* we cannot be discoverable and not connectable */
    if (((ZERO != discoverability) && (ZERO == connectability)) ||
        (ONE < discoverability) ||
        (ONE < connectability))
    {
        WICED_BT_TRACE(
            "%s: Invalid Arguements: we cannot be discoverable and not connectable\n", 
            __FUNCTION__);
    }
    else
    {
        result = wiced_bt_dev_set_discoverability(
            (discoverability != ZERO) ? BTM_GENERAL_DISCOVERABLE : 
            BTM_NON_DISCOVERABLE,
            BTM_DEFAULT_DISC_WINDOW,
            BTM_DEFAULT_DISC_INTERVAL);
        if (WICED_BT_SUCCESS != result)
        {
            WICED_BT_TRACE(
                "%s: wiced_bt_dev_set_discoverability failed. Status = %x\n", 
                __FUNCTION__, result);
        }

        result = wiced_bt_dev_set_connectability(
            (connectability != ZERO) ? WICED_TRUE : WICED_FALSE,
            BTM_DEFAULT_CONN_WINDOW,
            BTM_DEFAULT_CONN_INTERVAL);
        if (WICED_BT_SUCCESS != result)
        {
            WICED_BT_TRACE(
                "%s: wiced_bt_dev_set_connectability failed. Status = %x\n", 
                __FUNCTION__, result);
        }
    }
}

/*******************************************************************************
* Function Name: handsfree_handle_set_pairability
********************************************************************************
* Summary:
*  Enable or disable pairing
*
* Parameters:
*  unsigned int allowed : 1-pairable, 0-Not-Pairable
*
* Return:
*  NONE
*
*******************************************************************************/
void handsfree_handle_set_pairability (unsigned int allowed)
{
    if (ONE < allowed)
    {
        WICED_BT_TRACE("%s: Invalid Argument\n", __FUNCTION__);
    }
    else if (handsfree_app_states.pairing_allowed != allowed)
    {
        handsfree_app_states.pairing_allowed = allowed;
        wiced_bt_set_pairable_mode(handsfree_app_states.pairing_allowed, 
            WICED_FALSE);
        WICED_BT_TRACE(" Set the pairing allowed to %d \n", 
            handsfree_app_states.pairing_allowed);
    }
}

/*******************************************************************************
* Function Name: handsfree_inquiry
********************************************************************************
* Summary:
*  Starts or Stops Device Inquiry
*
* Parameters:
*  unsigned int enable : 1 - starts Inquiry, 0 - Stops Inquiry
*
* Return:
*  NONE
*
*******************************************************************************/
void handsfree_inquiry(unsigned int enable)
{
    wiced_result_t result;
    wiced_bt_dev_inq_parms_t params;

    if (ONE == enable)
    {
        memset(&params, ZERO, sizeof(params));

        params.mode             = BTM_GENERAL_INQUIRY;
        params.duration         = HFU_INQUIRY_DURATION;
        params.filter_cond_type = BTM_CLR_INQUIRY_FILTER;

        result = wiced_bt_start_inquiry(&params, 
            &handsfree_inquiry_result_cback);
        WICED_BT_TRACE("inquiry started:%d\n", result);
    }
    else if (ZERO == enable)
    {
        result = wiced_bt_cancel_inquiry();
        WICED_BT_TRACE("cancel inquiry:%d\n", result);
    }
    else
    {
        printf("%s: Invalid Argument\n", __FUNCTION__);
    }
}

/*******************************************************************************
* Function Name: handsfree_build_send_at_cmd
********************************************************************************
* Summary:
*   Builds the AT command for the HF command
*
* Parameters:
*   uint16_t handle : Connection Handle
*   char *cmd : HF command to be sent
*   uint8_t arg_type : WICED_BT_HFP_HF_AT_SET - command for setting the value
*                      AT+<cmd>=<val>
*                      WICED_BT_HFP_HF_AT_READ - command for getting the value
*                      AT+<cmd>?
*   uint8_t arg_format : Format of the command value/data
*                       (WICED_BT_HFP_HF_AT_FMT_INT/WICED_BT_HFP_HF_AT_FMT_STR)
*   const char *p_arg : command value in case the arg_format is string
*   int16_t int_arg :  command value in case the arg_format is int
*
* Return:
*  None
*
*******************************************************************************/
static void handsfree_build_send_at_cmd(uint16_t handle, char *cmd, 
    uint8_t arg_type, uint8_t arg_format, const char *p_arg, int16_t int_arg)
{
    char buf[BT_HFP_HF_AT_BUFF_LEN];
    char *p = buf;

    if (NULL == cmd)
    {
        WICED_BT_TRACE("%s: error cmd to be sent is null\n", __FUNCTION__);
    }
    else
    {
        memset (buf, ZERO, BT_HFP_HF_AT_BUFF_LEN);

        *p++ = 'A';
        *p++ = 'T';

        /* copy result code string */
        memcpy(p,cmd, strlen(cmd));
        p += strlen(cmd);

        if (WICED_BT_HFP_HF_AT_SET == arg_type)
        {
            *p++ = '=';

        }
        else if (WICED_BT_HFP_HF_AT_READ == arg_type)
        {
            *p++ = '?';

        }
        else if (WICED_BT_HFP_HF_AT_TEST == arg_type)
        {
            *p++ = '=';
            *p++ = '?';

        }

        /* copy argument if any */
        if (WICED_BT_HFP_HF_AT_FMT_INT == arg_format)
        {
            p += utl_itoa((uint16_t) int_arg, p);
        }
        else if (WICED_BT_HFP_HF_AT_FMT_STR == arg_format)
        {
            utl_strcpy(p, (char *)p_arg);
            p += strlen(p_arg);
        }

        /* finish with \r*/
        *p++ = '\r';

        WICED_BT_TRACE("SENDING AT CMD << %s\n", buf);

        wiced_bt_hfp_hf_send_at_cmd(handle,buf);
    }
}

/*******************************************************************************
* Function Name: handsfree_send_at_command
********************************************************************************
* Summary:
*  calls handsfree_build_send_at_cmd to build and sent the AT command
*
* Parameters:
*  uint16_t handle : connection handle
*  uint8_t command : command to be sent
*  int num : AT command arguement / value in case of integer
*  uint8_t* p_data : AT command arguement / value in case of string
*
* Return:
*  NONE
*
*******************************************************************************/
void handsfree_send_at_command (uint16_t handle, uint8_t command, int num, 
    uint8_t* p_data)
{
    WICED_BT_TRACE("\r\n\r\n >>>> handsfree_send_at_command: %d\r\n\r\n", 
        command);
    switch (command)
    {
    case HF_AT_CMD_VGS:
        wiced_bt_hfp_hf_notify_volume (handle,
            WICED_BT_HFP_HF_SPEAKER, num);
        break;

    case HF_AT_CMD_VGM:
        wiced_bt_hfp_hf_notify_volume (handle,
            WICED_BT_HFP_HF_MIC, num);
        break;

    case HF_AT_CMD_BINP:
        handsfree_build_send_at_cmd(handle, "+BINP",
            WICED_BT_HFP_HF_AT_SET, WICED_BT_HFP_HF_AT_FMT_INT, NULL, 1);
        break;

    case HF_AT_CMD_CHLD:
        wiced_bt_hfp_hf_perform_call_action(handle,
            (wiced_bt_hfp_hf_call_action_t)(WICED_BT_HFP_HF_CALL_ACTION_HOLD_0 
            + num), (char *)p_data);
        break;

    case HF_AT_CMD_BVRA:
        handsfree_build_send_at_cmd(handle, "+BVRA",
            WICED_BT_HFP_HF_AT_SET, WICED_BT_HFP_HF_AT_FMT_INT, NULL, num);
        break;

    case HF_AT_CMD_CMEE:
        handsfree_build_send_at_cmd(handle, "+CMEE",
            WICED_BT_HFP_HF_AT_SET, WICED_BT_HFP_HF_AT_FMT_INT, NULL, num);
        break;

    case HF_AT_CMD_ATA:
        wiced_bt_hfp_hf_perform_call_action(handle,
            WICED_BT_HFP_HF_CALL_ACTION_ANSWER,(char *)p_data);
        break;

    case HF_AT_CMD_CHUP:
        wiced_bt_hfp_hf_perform_call_action(handle,
            WICED_BT_HFP_HF_CALL_ACTION_HANGUP,(char *)p_data);
        break;

    case HF_AT_CMD_CNUM:
        handsfree_build_send_at_cmd(handle, "+CNUM",
            WICED_BT_HFP_HF_AT_NONE, WICED_BT_HFP_HF_AT_FMT_NONE, NULL, 
            ZERO);
        break;

    case HF_AT_CMD_CLCC:
        handsfree_build_send_at_cmd(handle, "+CLCC",
            WICED_BT_HFP_HF_AT_NONE, WICED_BT_HFP_HF_AT_FMT_NONE, NULL, 
            ZERO);
        break;

    case HF_AT_CMD_CIND:
        handsfree_build_send_at_cmd(handle, "+CIND",
            WICED_BT_HFP_HF_AT_READ, WICED_BT_HFP_HF_AT_FMT_NONE, NULL, 
            ZERO);
        break;

    case HF_AT_CMD_D:
    case HF_AT_CMD_BLDN:
        wiced_bt_hfp_hf_perform_call_action (handle ,
            WICED_BT_HFP_HF_CALL_ACTION_DIAL ,(char *)p_data);
        break;

    case HF_AT_CMD_NREC:
        handsfree_build_send_at_cmd(handle, "+NREC",
            WICED_BT_HFP_HF_AT_SET, WICED_BT_HFP_HF_AT_FMT_INT, NULL, 
            num);
        break;

    case HF_AT_CMD_VTS:
        handsfree_build_send_at_cmd(handle, "+VTS",
            WICED_BT_HFP_HF_AT_SET, WICED_BT_HFP_HF_AT_FMT_STR, (char *)p_data,
            ZERO);
        break;

    case HF_AT_CMD_BTRH:
        handsfree_build_send_at_cmd(handle, "+BTRH",
            WICED_BT_HFP_HF_AT_SET, WICED_BT_HFP_HF_AT_FMT_INT, NULL, num);
        break;

    case HF_AT_CMD_BIEV:
        handsfree_build_send_at_cmd(handle, "+BIEV",
            WICED_BT_HFP_HF_AT_SET, WICED_BT_HFP_HF_AT_FMT_STR, (char *)p_data,
            ZERO);
        break;

    case HF_AT_CMD_BIA:
        handsfree_build_send_at_cmd(handle, "+BIA",
            WICED_BT_HFP_HF_AT_SET, WICED_BT_HFP_HF_AT_FMT_STR, (char *)p_data,
            ZERO);
        break;

    case HF_AT_CMD_BCC:
        handsfree_build_send_at_cmd(handle, "+BCC",
            WICED_BT_HFP_HF_AT_NONE, WICED_BT_HFP_HF_AT_FMT_NONE, NULL, 
            ZERO);
        break;

    case HF_AT_CMD_BAC:
        handsfree_build_send_at_cmd(handle, "+BAC",
            WICED_BT_HFP_HF_AT_SET, WICED_BT_HFP_HF_AT_FMT_STR, (char *)p_data, 
            ZERO);
        break;
    }
}

/*******************************************************************************
* Function Name: handsfree_is_valid_state
********************************************************************************
* Summary:
*  This Function checks if the option can be handled in the current HF state
*
* Parameters:
*  int choice: user entered input
*
* Return:
*  uint8_t: returns if the user_input is valid in the current state
*  WICED_FALSE: for invalid state
*  WICED_TRUE: for valid state
*
*******************************************************************************/
uint8_t handsfree_is_valid_state (int user_input)
{
    uint8_t valid = WICED_FALSE;

    if( WICED_TRUE  == incoming_call)
    {
        handsfree_ctxt_data.is_sco_connected = WICED_TRUE;
    }


    switch(handsfree_ctxt_data.connection_status)
    {
    case WICED_BT_HFP_HF_STATE_DISCONNECTED:
        WICED_BT_TRACE("Current State: WICED_BT_HFP_HF_STATE_DISCONNECTED\n");
        switch(user_input)
        {
        case SET_VISIBILITY:
        case SET_PAIRING_MODE:
        case SET_INQUIRY:
        case HF_CONNECT:
        case REDIAL:
            valid = WICED_TRUE;
            break;
        }
        break;

    case WICED_BT_HFP_HF_STATE_CONNECTED:
        WICED_BT_TRACE("Current State: WICED_BT_HFP_HF_STATE_CONNECTED\n");
        switch(user_input)
        {
        case ANSWER_CALL:
        case HANGUP_CALL:
        case REDIAL:
            valid = WICED_TRUE;
            break;
        }
        break;
        
    case WICED_BT_HFP_HF_STATE_SLC_CONNECTED:
        WICED_BT_TRACE("Current State: WICED_BT_HFP_HF_STATE_SLC_CONNECTED\n");
        switch(user_input)
        {
        case ANSWER_CALL:
        case HANGUP_CALL:
        case REDIAL:
            valid = WICED_TRUE;
            break;
        }
        break;
    }
    return valid;
}

/* END OF FILE [] */
