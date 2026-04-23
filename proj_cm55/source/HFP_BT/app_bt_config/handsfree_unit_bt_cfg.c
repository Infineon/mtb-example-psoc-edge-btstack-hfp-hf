/******************************************************************************
*
* File Name: handsfree_unit_bt_cfg.c
*
* Description: Runtime Bluetooth stack configuration parameters
*
* Related Document: See README.md
*
*******************************************************************************
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
******************************************************************************/

/*******************************************************************************
*      INCLUDES
*******************************************************************************/
#include "wiced_bt_dev.h"
#include "wiced_bt_ble.h"
#include "wiced_bt_gatt.h"
#include "wiced_bt_cfg.h"
#include "wiced_bt_sdp.h"
#include "wiced_memory.h"
#include "handsfree_unit.h"
#include "wiced_bt_hfp_hf.h"

/*****************************************************************************
 *      WICED_BT CORE STACK CONFIGURATION
 ****************************************************************************/

uint8_t uuid_list[] =
{
    0x08, 0x11, /* Headset */
    0x1E, 0x11, /* Handsfree */
};

/*****************************************************************************
 *  BLE SCAN Setting 
*****************************************************************************/
const wiced_bt_cfg_ble_scan_settings_t wiced_bt_cfg_scan_settings =
{
    /**< BLE scan mode (PASSIVE, ACTIVE, or NONE) */
    .scan_mode = BTM_BLE_SCAN_MODE_ACTIVE,

    /* Advertisement scan configuration */
    .high_duty_scan_interval = 96, /**< High duty scan interval */
    .high_duty_scan_window = 48,   /**< High duty scan window */
    .high_duty_scan_duration = 30, /**< High duty scan duration (sec) */

    .low_duty_scan_interval = 2048,/**< Low duty scan interval  */
    .low_duty_scan_window = 48,    /**< Low duty scan window */
    .low_duty_scan_duration = 30,  /**< Low duty scan duration (sec) */

    /* Connection scan configuration */
    /**< High duty cycle connection scan interval */
    .high_duty_conn_scan_interval = 96,
    /**< High duty cycle connection scan window */
    .high_duty_conn_scan_window = 48,
    /**< High duty cycle connection duration (sec) */
    .high_duty_conn_duration = 30,

    /**< Low duty cycle connection scan interval */
    .low_duty_conn_scan_interval = 2048,
    /**< Low duty cycle connection scan window */
    .low_duty_conn_scan_window = 48,
    /**< Low duty cycle connection duration (sec) */
    .low_duty_conn_duration = 30,

    /* Connection configuration */
    /**< Minimum connection interval */
    .conn_min_interval = WICED_BT_CFG_DEFAULT_CONN_MIN_INTERVAL,
    /**< Maximum connection interval */
    .conn_max_interval = WICED_BT_CFG_DEFAULT_CONN_MAX_INTERVAL,
    /**< Connection latency */
    .conn_latency = WICED_BT_CFG_DEFAULT_CONN_LATENCY,
    /**< Connection link supervision timeout */
    .conn_supervision_timeout = WICED_BT_CFG_DEFAULT_CONN_SUPERVISION_TIMEOUT,
};

/*****************************************************************************
 *  BLE ADV Setting 
*****************************************************************************/
const wiced_bt_cfg_ble_advert_settings_t wiced_bt_cfg_adv_settings =
{
    /**< Advertising channel map */
    .channel_map = BTM_BLE_ADVERT_CHNL_37 |
                   BTM_BLE_ADVERT_CHNL_38 |
                   BTM_BLE_ADVERT_CHNL_39,

    /**< High duty undirected min advertising interval */
    .high_duty_min_interval = 160,
    /**< High duty undirected max advertising interval */
    .high_duty_max_interval = 160,
    /**< High duty undirected advertising duration (sec) */
    .high_duty_duration = 0,

    /**< Low duty undirected min advertising interval */
    .low_duty_min_interval = 400,
    /**< Low duty undirected max advertising interval */
    .low_duty_max_interval = 400,
    /**< Low duty undirected advertising duration (sec) */
    .low_duty_duration = 0,

    /**< High duty directed min advertising interval */
    .high_duty_directed_min_interval =
        WICED_BT_CFG_DEFAULT_HIGH_DUTY_DIRECTED_ADV_MIN_INTERVAL,
    /**< High duty directed max advertising interval */
    .high_duty_directed_max_interval =
        WICED_BT_CFG_DEFAULT_HIGH_DUTY_DIRECTED_ADV_MAX_INTERVAL,

    /**< Low duty directed min advertising interval */
    .low_duty_directed_min_interval =
        WICED_BT_CFG_DEFAULT_LOW_DUTY_DIRECTED_ADV_MIN_INTERVAL,
    /**< Low duty directed max advertising interval */
    .low_duty_directed_max_interval =
        WICED_BT_CFG_DEFAULT_LOW_DUTY_DIRECTED_ADV_MAX_INTERVAL,
    /**< Low duty directed advertising duration (sec) */
    .low_duty_directed_duration = 30,

    /**< High duty non-connectable min advertising interval */
    .high_duty_nonconn_min_interval =
        WICED_BT_CFG_DEFAULT_HIGH_DUTY_NONCONN_ADV_MIN_INTERVAL,
    /**< High duty non-connectable max advertising interval */
    .high_duty_nonconn_max_interval =
        WICED_BT_CFG_DEFAULT_HIGH_DUTY_NONCONN_ADV_MAX_INTERVAL,
    /**< High duty non-connectable advertising duration (sec) */
    .high_duty_nonconn_duration = 30,

    /**< Low duty non-connectable min advertising interval */
    .low_duty_nonconn_min_interval =
        WICED_BT_CFG_DEFAULT_LOW_DUTY_NONCONN_ADV_MIN_INTERVAL,
    /**< Low duty non-connectable max advertising interval */
    .low_duty_nonconn_max_interval =
        WICED_BT_CFG_DEFAULT_LOW_DUTY_NONCONN_ADV_MAX_INTERVAL,
    /**< Low duty non-connectable advertising duration (sec) */
    .low_duty_nonconn_duration = 0,
};

/*****************************************************************************
 *  L2CAP Setting 
*****************************************************************************/
/**< Application managed l2cap protocol configuration */
const wiced_bt_cfg_l2cap_application_t wiced_bt_cfg_l2cap_app =
{
    /* BR EDR l2cap configuration */
    /**< Maximum number of application-managed BR/EDR PSMs */
    .max_app_l2cap_psms = 0,
    /**< Maximum number of application-managed BR/EDR channels  */
    .max_app_l2cap_channels = 0,

    /**< Maximum ERTM channels allowed */
    .max_app_l2cap_br_edr_ertm_chnls = 0,
    /**< Maximum ERTM TX Window allowed */
    .max_app_l2cap_br_edr_ertm_tx_win = 0,

    /* LE L2cap connection-oriented channels configuration */
    .max_app_l2cap_le_fixed_channels = 0,
};

/*****************************************************************************
 *  BR Setting 
*****************************************************************************/
const wiced_bt_cfg_br_t wiced_bt_cfg_br =
{
    .br_max_simultaneous_links = 1,
    .br_max_rx_pdu_size = 1024,
    .device_class = {0x24, 0x04, 0x18}, /**< Local device class */

    .rfcomm_cfg = /* RFCOMM configuration */
    {
        /**< Max connected remote devices */
        .max_links = WICED_BT_HFP_HF_MAX_CONN,
        /**< Max simultaneous RFCOMM ports */
        .max_ports = WICED_BT_HFP_HF_MAX_CONN,
    },
    .avdt_cfg = /* Audio/Video Distribution configuration */
    {
        .max_links = 1, /**< Maximum simultaneous audio/video links */
        .max_seps = 3,  /**< Maximum number of stream end points */
    },

    .avrc_cfg = /* Audio/Video Remote Control configuration */
    {
        .max_links = 1, /**< Maximum simultaneous remote control links */
    },
};

/*****************************************************************************
 *  ISOC Setting 
*****************************************************************************/
const wiced_bt_cfg_isoc_t wiced_bt_cfg_isoc =
{
    .max_cis_conn = 0,
    .max_cig_count = 0,
    .max_sdu_size = 0,
    .channel_count = 0,
    .max_buffers_per_cis = 0,
};

/*****************************************************************************
 *  BLE Setting 
*****************************************************************************/

const wiced_bt_cfg_ble_t wiced_bt_cfg_ble =
{
    .ble_max_simultaneous_links = 1,
    .ble_max_rx_pdu_size = 365,
    /**< GATT appearance (see gatt_appearance_e) */
    .appearance = APPEARANCE_GENERIC_TAG,
    /**< Interval of random address refreshing (sec) */
    .rpa_refresh_timeout = WICED_BT_CFG_DEFAULT_RANDOM_ADDRESS_NEVER_CHANGE,
    /**< LE Address Resolution DB settings */
    .host_addr_resolution_db_size = 5,
    .p_ble_scan_cfg = &wiced_bt_cfg_scan_settings,
    .p_ble_advert_cfg = &wiced_bt_cfg_adv_settings,
};

/*****************************************************************************
 *  GATT Setting 
*****************************************************************************/

const wiced_bt_cfg_gatt_t wiced_bt_cfg_gatt =
{
    /**< Maximum number of service modules in the DB*/
    .max_db_service_modules = 0,
    /**< Maximum number of allowed gatt bearers */
    .max_eatt_bearers = 0,
};

/* wiced_bt core stack configuration */
const wiced_bt_cfg_settings_t handsfree_cfg_settings =
{
    /**< Local device name ( NULL terminated ) */
    .device_name = (uint8_t *)HANDS_FREE_DEVICE_NAME,
    /**< Security requirements mask */
    .security_required = BTM_SEC_BEST_EFFORT,

    .p_br_cfg = &wiced_bt_cfg_br,
    .p_ble_cfg = &wiced_bt_cfg_ble,
    .p_gatt_cfg = &wiced_bt_cfg_gatt,
    .p_isoc_cfg = &wiced_bt_cfg_isoc,
    .p_l2cap_app_cfg = &wiced_bt_cfg_l2cap_app,
};

/*****************************************************************************
 *      SDP DATABASE FOR THE HANDSFREE APPLICATION
 ****************************************************************************/

const uint8_t handsfree_sdp_db[] =
{
    SDP_ATTR_SEQUENCE_1(75 + 2                          /* HFP */
#ifdef  WICED_ENABLE_BT_HSP_PROFILE
              + 75 + 2                                  /* HSP */
#endif
             ),

    /* SDP Record for Hands-Free Unit */
    SDP_ATTR_SEQUENCE_1(75),
        SDP_ATTR_RECORD_HANDLE(HDLR_HANDS_FREE_UNIT),
        SDP_ATTR_ID(ATTR_ID_SERVICE_CLASS_ID_LIST),
            SDP_ATTR_SEQUENCE_1(6),
            SDP_ATTR_UUID16(UUID_SERVCLASS_HF_HANDSFREE),
            SDP_ATTR_UUID16(UUID_SERVCLASS_GENERIC_AUDIO),
        SDP_ATTR_RFCOMM_PROTOCOL_DESC_LIST(HANDS_FREE_SCN),
        SDP_ATTR_ID(ATTR_ID_BT_PROFILE_DESC_LIST),
            SDP_ATTR_SEQUENCE_1(8),
            SDP_ATTR_SEQUENCE_1(6),
                SDP_ATTR_UUID16(UUID_SERVCLASS_HF_HANDSFREE),
                SDP_ATTR_VALUE_UINT2(0x0108),
        SDP_ATTR_SERVICE_NAME(15),
            'W', 'I', 'C', 'E', 'D', ' ', 'H', 'F', ' ',
            'D', 'E', 'V', 'I', 'C', 'E',
        SDP_ATTR_UINT2(ATTR_ID_SUPPORTED_FEATURES, SUPPORTED_FEATURES_ATT),

#ifdef  WICED_ENABLE_BT_HSP_PROFILE
    /* SDP Record for Hands-Free Unit */
    SDP_ATTR_SEQUENCE_1(75),
        SDP_ATTR_RECORD_HANDLE(HDLR_HEADSET_UNIT),
        SDP_ATTR_ID(ATTR_ID_SERVICE_CLASS_ID_LIST),
            SDP_ATTR_SEQUENCE_1(6),
            SDP_ATTR_UUID16(UUID_SERVCLASS_HEADSET),
            SDP_ATTR_UUID16(UUID_SERVCLASS_GENERIC_AUDIO),
        SDP_ATTR_RFCOMM_PROTOCOL_DESC_LIST(HEADSET_SCN),
        SDP_ATTR_ID(ATTR_ID_BT_PROFILE_DESC_LIST),
            SDP_ATTR_SEQUENCE_1(8),
            SDP_ATTR_SEQUENCE_1(6),
                SDP_ATTR_UUID16(UUID_SERVCLASS_HEADSET),
                SDP_ATTR_VALUE_UINT2(0x0102),
        SDP_ATTR_SERVICE_NAME(15),
            'W', 'I', 'C', 'E', 'D', ' ', 'H', 'S', ' ',
            'D', 'E', 'V', 'I', 'C', 'E',
        SDP_ATTR_UINT2(ATTR_ID_SUPPORTED_FEATURES, 0x0016),
#endif
};

wiced_bt_device_address_t bt_device_address = {
    0x22, 0x11, 0x44, 0x33, 0x66, 0x56
};

/*
 * wiced_app_cfg_sdp_record_get_size
 */
uint16_t wiced_app_cfg_sdp_record_get_size(void)
{
    return (uint16_t)sizeof(handsfree_sdp_db);
}
