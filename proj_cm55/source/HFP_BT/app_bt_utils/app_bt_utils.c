/*******************************************************************************
* File Name:   app_bt_utils.c
*
* Description: This file consists of the utility functions that will help
*              debugging and developing the applications easier with much
*              more meaningful information.
*
* Related Document: See Readme.md
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
#include "app_bt_utils.h"
#include "wiced_bt_dev.h"

/*******************************************************************************
* Macros
*******************************************************************************/
#define CASE_DEFAULT_STR default: printf("%s: default case\n", __FUNCTION__); break;

#define ZERO                        (0U)

#define DECIMAL_TO_HEX_MODULO       (16U)

/*******************************************************************************
* Function Definitions
*******************************************************************************/
/*******************************************************************************
* Function Name: print_bd_address()
********************************************************************************
* Summary:
*  This is the utility function that prints the address of the Bluetooth device
*
* Parameters:
*  wiced_bt_device_address_t bdadr                : Bluetooth address
*
* Return:
*  None
*
*******************************************************************************/
void print_bd_address(wiced_bt_device_address_t bdadr)
{
    printf("%02X:%02X:%02X:%02X:%02X:%02X\r\n", bdadr[BD_ADDR_INDEX_ZERO], 
        bdadr[BD_ADDR_INDEX_ONE], bdadr[BD_ADDR_INDEX_TWO],
        bdadr[BD_ADDR_INDEX_THREE], bdadr[BD_ADDR_INDEX_FOUR], 
        bdadr[BD_ADDR_INDEX_FIVE]);
}

/*******************************************************************************
* Function Name: print_array()
********************************************************************************
* Summary:
*  This is a utility function that prints the specified number of values from
*  memory
*
* Parameters:
*  void* to_print              : Pointer to the location to print
*  uint16_t                    : Number of bytes to print
*
* Return:
*  None
*
*******************************************************************************/
void print_array(void * to_print, uint16_t len)
{
    uint16_t counter;
    if(NULL == to_print)
    {
        printf("Error : print_array recive NULL pointer\n");
        return;
    }
    for (counter = ZERO; counter < len; counter++)
    {
        if (ZERO == counter % DECIMAL_TO_HEX_MODULO)
        {
           printf("\n");
        }
        printf("%02X ", *(((uint8_t *)(to_print)) + counter));
    }
    printf("\n");

}

/*******************************************************************************
* Function Name: get_bt_event_name()
********************************************************************************
* Summary:
*  The function converts the wiced_bt_management_evt_t enum value to its
*  corresponding string literal. This will help the programmer to debug easily
*  with log traces without navigating through the source code.
*
* Parameters:
*  wiced_bt_management_evt_t event: Bluetooth management event type
*
* Return:
*  const char * [char string]
*
*******************************************************************************/
const char *get_bt_event_name(wiced_bt_management_evt_t event)
{
    switch ((int)event)
    {
        CASE_RETURN_STR(BTM_ENABLED_EVT)
        CASE_RETURN_STR(BTM_DISABLED_EVT)
        CASE_RETURN_STR(BTM_POWER_MANAGEMENT_STATUS_EVT)
        CASE_RETURN_STR(BTM_PIN_REQUEST_EVT)
        CASE_RETURN_STR(BTM_USER_CONFIRMATION_REQUEST_EVT)
        CASE_RETURN_STR(BTM_PASSKEY_NOTIFICATION_EVT)
        CASE_RETURN_STR(BTM_PASSKEY_REQUEST_EVT)
        CASE_RETURN_STR(BTM_KEYPRESS_NOTIFICATION_EVT)
        CASE_RETURN_STR(BTM_PAIRING_IO_CAPABILITIES_BR_EDR_REQUEST_EVT)
        CASE_RETURN_STR(BTM_PAIRING_IO_CAPABILITIES_BR_EDR_RESPONSE_EVT)
        CASE_RETURN_STR(BTM_PAIRING_IO_CAPABILITIES_BLE_REQUEST_EVT)
        CASE_RETURN_STR(BTM_PAIRING_COMPLETE_EVT)
        CASE_RETURN_STR(BTM_ENCRYPTION_STATUS_EVT)
        CASE_RETURN_STR(BTM_SECURITY_REQUEST_EVT)
        CASE_RETURN_STR(BTM_SECURITY_FAILED_EVT)
        CASE_RETURN_STR(BTM_SECURITY_ABORTED_EVT)
        CASE_RETURN_STR(BTM_READ_LOCAL_OOB_DATA_COMPLETE_EVT)
        CASE_RETURN_STR(BTM_REMOTE_OOB_DATA_REQUEST_EVT)
        CASE_RETURN_STR(BTM_PAIRED_DEVICE_LINK_KEYS_UPDATE_EVT)
        CASE_RETURN_STR(BTM_PAIRED_DEVICE_LINK_KEYS_REQUEST_EVT)
        CASE_RETURN_STR(BTM_LOCAL_IDENTITY_KEYS_UPDATE_EVT)
        CASE_RETURN_STR(BTM_LOCAL_IDENTITY_KEYS_REQUEST_EVT)
        CASE_RETURN_STR(BTM_BLE_SCAN_STATE_CHANGED_EVT)
        CASE_RETURN_STR(BTM_BLE_ADVERT_STATE_CHANGED_EVT)
        CASE_RETURN_STR(BTM_SMP_REMOTE_OOB_DATA_REQUEST_EVT)
        CASE_RETURN_STR(BTM_SMP_SC_REMOTE_OOB_DATA_REQUEST_EVT)
        CASE_RETURN_STR(BTM_SMP_SC_LOCAL_OOB_DATA_NOTIFICATION_EVT)
        CASE_RETURN_STR(BTM_SCO_CONNECTED_EVT)
        CASE_RETURN_STR(BTM_SCO_DISCONNECTED_EVT)
        CASE_RETURN_STR(BTM_SCO_CONNECTION_REQUEST_EVT)
        CASE_RETURN_STR(BTM_SCO_CONNECTION_CHANGE_EVT)
        CASE_RETURN_STR(BTM_BLE_CONNECTION_PARAM_UPDATE)
        CASE_RETURN_STR(BTM_BLE_PHY_UPDATE_EVT)
        CASE_DEFAULT_STR
    }

    return "UNKNOWN_EVENT";
}

/* END OF FILE [] */
