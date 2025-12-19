/*******************************************************************************
* File Name: handsfree_unit.h
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
 ******************************************************************************/
#ifndef __APP_HANDSFREE_H__
#define __APP_HANDSFREE_H__

/*******************************************************************************
* Header file includes
*******************************************************************************/
#include <stdio.h>
#include "wiced_bt_cfg.h"
#include "wiced_bt_hfp_hf_int.h"
#include "wiced_bt_utils.h"

/* Enable below block for verbose logging via printf  */
#if 0
    #ifdef WICED_BT_TRACE
        #undef WICED_BT_TRACE
    #endif
    #define WICED_BT_TRACE          printf
#endif

/*******************************************************************************
* Macros
*******************************************************************************/

/* Change this to your HF AG BD address */
#define HFP_AG_BD_ADDRESS              {0x11, 0x22, 0x33, 0x44, 0x55, 0x66}

#define HF_AT_CMD_VGS                  (0x00)    /* Update speaker volume */
#define HF_AT_CMD_VGM                  (0x01)    /* Update microphone volume */
#define HF_AT_CMD_ATA                  (0x02)    /* Answer incoming call */
#define HF_AT_CMD_BINP                 (0x03)    /* Retrieve number from voice tag */
#define HF_AT_CMD_BVRA                 (0x04)    /* Enable/Disable voice recognition */
#define HF_AT_CMD_BLDN                 (0x05)    /* Last Number redial */
#define HF_AT_CMD_CHLD                 (0x06)    /* Call hold command */
#define HF_AT_CMD_CHUP                 (0x07)    /* Call hang up command */
#define HF_AT_CMD_CIND                 (0x08)    /* Read Indicator Status */
#define HF_AT_CMD_CNUM                 (0x09)    /* Retrieve Subscriber number */
#define HF_AT_CMD_D                    (0x0A)    /* Place a call using a number or memory dial */
#define HF_AT_CMD_NREC                 (0x0B)    /* Disable Noise reduction and echo canceling in AG */
#define HF_AT_CMD_VTS                  (0x0C)    /* Transmit DTMF tone */
#define HF_AT_CMD_BTRH                 (0x0D)    /* CCAP incoming call hold */
#define HF_AT_CMD_COPS                 (0x0E)    /* Query operator selection */
#define HF_AT_CMD_CMEE                 (0x0F)    /* Enable/disable extended AG result codes */
#define HF_AT_CMD_CLCC                 (0x10)    /* Query list of current calls in AG */
#define HF_AT_CMD_BIA                  (0x11)    /* Activate/Deactivate indicators */
#define HF_AT_CMD_BIEV                 (0x12)    /* Send HF indicator value to peer */
#define HF_AT_CMD_BCC                  (0x13)    /* Initiate Codec Connection */
#define HF_AT_CMD_BCS                  (0x14)    /* Codec Selection */
#define HF_AT_CMD_BAC                  (0x15)    /* Updating Available Codec */
#define HF_AT_CMD_MAX                  (0x16)    /* For Command validation */

/* SDP Record for Hands-Free Unit */
#define HDLR_HANDS_FREE_UNIT            (0x10001)
#define HDLR_HEADSET_UNIT               (0x10002)
#define HANDS_FREE_SCN                  (0x01)
#define HEADSET_SCN                     (0x02)
#define HANDS_FREE_DEVICE_NAME          ("Hands-Free Unit")
#define BT_AUDIO_INVALID_SCO_INDEX      (0xFFFF)

#define WICED_HS_EIR_BUF_MAX_SIZE       (264U)

#ifndef BTM_SCO_PKT_TYPES_MASK_HV1
#define BTM_SCO_PKT_TYPES_MASK_HV3      (0x0004)
#define BTM_SCO_PKT_TYPES_MASK_EV3      (0x0008)
#define BTM_SCO_PKT_TYPES_MASK_EV4      (0x0010)
#define BTM_SCO_PKT_TYPES_MASK_EV5      (0x0020)
#define BTM_SCO_PKT_TYPES_MASK_NO_3_EV3 (0x0080)
#define BTM_SCO_PKT_TYPES_MASK_NO_3_EV5 (0x0200)
#define BTM_ESCO_RETRANS_POWER          (2U)       /* For S4 and T2 Retransmission_Effort=2 */
#endif

#define HANDS_FREE_SCO_PKT_TYPES (BTM_SCO_PKT_TYPES_MASK_HV3 | \
                                 BTM_SCO_PKT_TYPES_MASK_EV3 | \
                                 BTM_SCO_PKT_TYPES_MASK_EV4 | \
                                 BTM_SCO_PKT_TYPES_MASK_EV5 | \
                                 BTM_SCO_PKT_TYPES_MASK_NO_3_EV3 | \
                                 BTM_SCO_PKT_TYPES_MASK_NO_3_EV5)

#if (WICED_BT_HFP_HF_WBS_INCLUDED == TRUE)
#define SUPPORTED_FEATURES_ATT            (WICED_BT_HFP_HF_SDP_FEATURE_ECNR | \
                                           WICED_BT_HFP_HF_SDP_FEATURE_3WAY_CALLING | \
                                           WICED_BT_HFP_HF_SDP_FEATURE_CLIP | \
                                           WICED_BT_HFP_HF_SDP_FEATURE_VRECG | \
                                           WICED_BT_HFP_HF_SDP_FEATURE_REMOTE_VOL_CTRL | \
                                           WICED_BT_HFP_HF_SDP_FEATURE_WIDEBAND_SPEECH)

#define BT_AUDIO_HFP_SUPPORTED_FEATURES   (WICED_BT_HFP_HF_FEATURE_3WAY_CALLING | \
                                           WICED_BT_HFP_HF_FEATURE_CLIP_CAPABILITY | \
                                           WICED_BT_HFP_HF_FEATURE_VOICE_RECOGNITION_ACTIVATION | \
                                           WICED_BT_HFP_HF_FEATURE_REMOTE_VOLUME_CONTROL| \
                                           WICED_BT_HFP_HF_FEATURE_ENHANCED_CALL_STATUS | \
                                           WICED_BT_HFP_HF_FEATURE_ENHANCED_CALL_CONTROL | \
                                           WICED_BT_HFP_HF_FEATURE_CODEC_NEGOTIATION | \
                                           WICED_BT_HFP_HF_FEATURE_HF_INDICATORS | \
                                           WICED_BT_HFP_HF_FEATURE_ECNR | \
                                           WICED_BT_HFP_HF_FEATURE_ENHANCED_VOICE_RECOGNITION)
#else
#define SUPPORTED_FEATURES_ATT            (WICED_BT_HFP_HF_SDP_FEATURE_ECNR | \
                                           WICED_BT_HFP_HF_SDP_FEATURE_3WAY_CALLING | \
                                           WICED_BT_HFP_HF_SDP_FEATURE_CLIP | \
                                           WICED_BT_HFP_HF_SDP_FEATURE_VRECG | \
                                           WICED_BT_HFP_HF_SDP_FEATURE_REMOTE_VOL_CTRL)

#define BT_AUDIO_HFP_SUPPORTED_FEATURES   (WICED_BT_HFP_HF_FEATURE_3WAY_CALLING | \
                                           WICED_BT_HFP_HF_FEATURE_CLIP_CAPABILITY | \
                                           WICED_BT_HFP_HF_FEATURE_VOICE_RECOGNITION_ACTIVATION | \
                                           WICED_BT_HFP_HF_FEATURE_REMOTE_VOLUME_CONTROL| \
                                           WICED_BT_HFP_HF_FEATURE_ENHANCED_CALL_STATUS | \
                                           WICED_BT_HFP_HF_FEATURE_ENHANCED_CALL_CONTROL | \
                                           WICED_BT_HFP_HF_FEATURE_HF_INDICATORS | \
                                           WICED_BT_HFP_HF_FEATURE_ECNR | \
                                           WICED_BT_HFP_HF_FEATURE_ENHANCED_VOICE_RECOGNITION)
#endif

/*******************************************************************************
* Structures and Enumerations
*******************************************************************************/
/** HFP User Commands */
typedef enum
{
    NONE                            = 0,
    SET_VISIBILITY                  = 1,
    SET_PAIRING_MODE                = 2,
    SET_INQUIRY                     = 3,
    HF_CONNECT                      = 4,
    ANSWER_CALL                     = 5,
    HANGUP_CALL                     = 6,
    REDIAL                          = 8,
} hfp_user_cmd_t;

/** HF device indicators. */
typedef enum
{
    WICED_BT_HFP_HF_SERVICE_IND     = 1,
    WICED_BT_HFP_HF_CALL_IND        = 2,
    WICED_BT_HFP_HF_CALL_SETUP_IND  = 3,
    WICED_BT_HFP_HF_CALL_HELD_IND   = 4,
    WICED_BT_HFP_HF_SIGNAL_IND      = 5,
    WICED_BT_HFP_HF_ROAM_IND        = 6,
    WICED_BT_HFP_HF_BATTERY_IND     = 7
} wiced_bt_hfp_hf_indicator_t;

typedef struct
{
    /* Connected AG device's BD address */
    wiced_bt_device_address_t               peer_bd_addr;
    /* Connected AG device's connection status */
    wiced_bt_hfp_hf_connection_state_t      connection_status;
    /* Call active/inactive */
    int                                     call_active;
    /* Call held */
    int                                     call_held;
    /* Call setup states */
    wiced_bt_hfp_hf_callsetup_state_t       call_setup;
    /* Call ring enabled/disabled state */
    wiced_bt_hfp_hf_inband_ring_state_t     inband_ring_status;
    /* Connected AG's microphone volume level */
    uint8_t                                 mic_volume;
    /* Connected AG's speaker volume level */
    uint8_t                                 spkr_volume;
    /* SCO index used for audio comms. */
    uint16_t                                sco_index;
    /* Handle used to send control commands to the connected AG device */
    uint16_t                                rfcomm_handle;
    /* HF initiation/wait for the SCO connection */
    wiced_bool_t                            init_sco_conn;
    /* SCO connection complete status */
    wiced_bool_t                            is_sco_connected;
    /* SCO connection voice config. path */
    wiced_bt_voice_path_setup_t             sco_voice_path;
} bluetooth_hfp_context_t;

/* Data associated with HF_OPEN_EVT */
typedef struct
{
    BD_ADDR             bd_addr;
    uint8_t             status;
} hci_control_hf_open_t;

/* Data associated with AT command response event */
typedef struct
{
    uint16_t            num;
    char                str[WICED_BT_HFP_HF_MAX_AT_CMD_LEN];
} hci_control_hf_value_t;

/* Data associated with HF_CONNECTED_EVT */
typedef struct
{
    uint32_t           peer_features;
    uint8_t            profile_selected;
} hci_control_hf_connect_t;

/* Union of data associated with HF event callback */
typedef union
{
    hci_control_hf_open_t    open;
    hci_control_hf_connect_t conn;
    hci_control_hf_value_t   val;
} hci_control_hf_event_t;

/* Data associated to connection parameters for the HF */
typedef struct
{
    uint8_t pairing_allowed;
    hci_control_hf_connect_t connect;
} handsfrees_app_globals;

/*******************************************************************************
* Variable Definitions
*******************************************************************************/
extern const wiced_bt_cfg_settings_t handsfree_cfg_settings;
extern const uint8_t handsfree_sdp_db[];
extern wiced_bt_sco_params_t handsfree_esco_params;

/*******************************************************************************
* Function Prototypes
*******************************************************************************/
/* External Function Definitions */
extern uint16_t wiced_app_cfg_sdp_record_get_size(void);

wiced_bt_dev_status_t handsfree_management_callback( 
    wiced_bt_management_evt_t event, 
    wiced_bt_management_evt_data_t *p_event_data);
void handsfree_inquiry_result_cback(
    wiced_bt_dev_inquiry_scan_result_t *p_inquiry_result, uint8_t *p_eir_data);
void handsfree_print_hfp_context(void);
void wait_init_done();
void notify_init_done();

#endif /* __APP_HANDSFREE_UNIT_H__ */

/* [] END OF FILE */
