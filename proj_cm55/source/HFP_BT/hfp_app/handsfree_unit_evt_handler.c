/*******************************************************************************
* File Name: handsfree_unit_evt_handler.c
*
* Description: This is the source code for the Handsfree Unit Code Example
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
#include "cybsp.h"
#include "handsfree_unit_evt_handler.h"
#include "FreeRTOS.h"
#include "app_bt_bonding.h"
#include "app_bt_utils.h"
#include "app_i2s.h"
#include "audio_platform_common.h"
#include "handsfree_unit.h"
#include "handsfree_unit_main.h"
#include "handsfree_unit_api.h"

#include "sbc_decoder.h"
#include "semphr.h"
#include "stdio.h"
#include "wiced_bt_dev.h"
#include "wiced_bt_stack.h"
#include "wiced_memory.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "pdm_mic_interface.h"

/*******************************************************************************
* Macros
*******************************************************************************/
#define ZERO                                (0U)
#define ONE                                 (1U)
#define TWO                                 (2U)

#define NREC_ENABLED                        (1U)

/*******************************************************************************
* Structures and Enumerations
*******************************************************************************/
wiced_bt_sco_params_t handsfree_esco_params = {
#if (WICED_BT_HFP_HF_WBS_INCLUDED == TRUE)
    0x000D, /* Latency: 13 ms ( HS/HF can use EV3, 2-EV3, 3-EV3 ) ( T2 ) */
#else
    0x000C, /* Latency: 12 ms ( HS/HF can use EV3, 2-EV3, 3-EV3 ) ( S4 ) */
#endif
    HANDS_FREE_SCO_PKT_TYPES,
    /* Retrans Effort ( At least one retrans, opt for power ) ( S4 ) */
    BTM_ESCO_RETRANS_POWER,
#if (WICED_BT_HFP_HF_WBS_INCLUDED == TRUE)
    WICED_TRUE
#else
    WICED_FALSE
#endif
};

#define ESCO_INPUT_DATA_SIZE                 (8U)
#define ESCO_OUTPUT_DATA_SIZE                (8U)

#define ONE_BYTE_BITS                        (8U)
#define ONE_BYTE_MASK                        (0xFF)

#define BT_HFP_HF_DEF_SPK_VOL                (8U)
#define BT_HFP_HF_DEF_MIC_VOL                (8U)
#define MIC_DATA_LENTH                       (480U)

/*******************************************************************************
* Global variables
*******************************************************************************/
extern wiced_bt_device_address_t bt_device_address;

SemaphoreHandle_t bt_semaphore;
uint8_t bondindex = ZERO;

bluetooth_hfp_context_t handsfree_ctxt_data;
handsfrees_app_globals handsfree_app_states;
bool incoming_call = false;
uint8_t mic_data[MIC_DATA_LENTH] = {ZERO};

/*******************************************************************************
* Function prototypes
*******************************************************************************/
void handsfree_post_bt_init(wiced_bt_management_evt_data_t *p_event_data);
void handsfree_hfp_init(void);
void handsfree_write_eir(void);
void handsfree_init_context_data(void);
void hf_sco_management_callback(wiced_bt_management_evt_t event,
    wiced_bt_management_evt_data_t *p_event_data);
static const char *handsfree_get_indicator_name(wiced_bt_hfp_hf_event_t event);
static void handsfree_call_setup_event_handler(
    wiced_bt_hfp_hf_call_data_t *call_data);
static void handsfree_event_callback(wiced_bt_hfp_hf_event_t event,
    wiced_bt_hfp_hf_event_data_t *p_data);
static void handsfree_connection_event_handler(
    wiced_bt_hfp_hf_event_data_t *p_data);
static void handsfree_sco_data_app_callback(uint16_t sco_channel,
    uint16_t length, uint8_t *p_data);

/*******************************************************************************
* Function definitions
*******************************************************************************/
/*******************************************************************************
* Function Name: handsfree_management_callback
********************************************************************************
* Summary:
*  This is a Bluetooth stack event handler function to receive management
*  events from the Bluetooth stack and process as per the application.
*
* Parameters:
*  wiced_bt_management_evt_t event : BT event code of one byte length
*  wiced_bt_management_evt_data_t *p_event_data : Pointer to BT management
*  event structures
*
* Return:
*  wiced_result_t: Error code from WICED_RESULT_LIST or BT_RESULT_LIST
*
*******************************************************************************/
wiced_result_t handsfree_management_callback(wiced_bt_management_evt_t event,
    wiced_bt_management_evt_data_t *p_event_data) 
{    
    wiced_bt_device_address_t bda = {ZERO};
    cy_rslt_t rslt;
    wiced_result_t result = WICED_BT_SUCCESS;
    wiced_bt_dev_pairing_cplt_t *p_pairing_cmpl;
    uint8_t pairing_result;
    wiced_bt_dev_encryption_status_t *p_encryption_status;

    WICED_BT_TRACE("handsfree_management_callback. Event: 0x%x %s\n", event,
        get_bt_event_name(event));

    switch (event) 
    {
    case BTM_ENABLED_EVT:
        /* Bluetooth Controller and Host Stack Enabled */
        if (WICED_BT_SUCCESS == p_event_data->enabled.status) 
        {
            wiced_bt_set_local_bdaddr((uint8_t *)bt_device_address,
                BLE_ADDR_PUBLIC);

            wiced_bt_dev_read_local_addr(bda);
            WICED_BT_TRACE("Local Bluetooth Address: ");
            print_bd_address(bda);

            /* Disable pairing, pairing has to be explicitly enabled by user */
            wiced_bt_set_pairable_mode(WICED_FALSE, WICED_FALSE);

            handsfree_post_bt_init(p_event_data);
            notify_init_done();    
        } 
        else 
        {
            WICED_BT_TRACE("Bluetooth Enable Failed \n");
        }
        break;

    case BTM_DISABLED_EVT:
        WICED_BT_TRACE("Bluetooth Disabled \n");
        break;

    case BTM_SCO_CONNECTED_EVT:
    case BTM_SCO_CONNECTION_REQUEST_EVT:
    case BTM_SCO_CONNECTION_CHANGE_EVT:
    case BTM_SCO_DISCONNECTED_EVT:
        hf_sco_management_callback(event, p_event_data);
        break;

    case BTM_SECURITY_REQUEST_EVT:
        if (handsfree_app_states.pairing_allowed) 
        {
            wiced_bt_ble_security_grant(p_event_data->security_request.bd_addr,
                WICED_BT_SUCCESS);
        } 
        else 
        {
            /* Pairing not allowed, return error */
            result = WICED_BT_ERROR;
        }
        break;

    case BTM_PAIRING_COMPLETE_EVT:
        p_pairing_cmpl = &p_event_data->pairing_complete;

        if (p_pairing_cmpl->transport == BT_TRANSPORT_BR_EDR) 
        {
            pairing_result =
                p_pairing_cmpl->pairing_complete_info.br_edr.status;
        }
        else 
        {
            pairing_result = p_pairing_cmpl->pairing_complete_info.ble.reason;
        }
        WICED_BT_TRACE("pairing_result = 0x%x\n", pairing_result);
        /* Update Num of bonded devices and next free slot in slot data*/
        rslt = app_bt_update_slot_data();

        if(CY_RSLT_TYPE_ERROR == rslt)
        {
            result = WICED_BT_ERROR;
            printf("Error updating number of bonded device!\n");
        }

        break;

    case BTM_PAIRED_DEVICE_LINK_KEYS_UPDATE_EVT:
        WICED_BT_TRACE("BTM_PAIRED_DEVICE_LINK_KEYS_UPDATE_EVT\n");

        /* This application supports a single paired host, we can save keys
         * under the same NVRAM ID overwriting previous pairing if any */
        if (p_event_data == NULL) 
        {
            WICED_BT_TRACE("Callback data pointer p_event_data is NULL \n");
            break;
        }
        /* save device keys to NVRAM */
        rslt = app_bt_save_device_link_keys(
            &(p_event_data->paired_device_link_keys_update));
        if (CY_RSLT_SUCCESS == rslt) 
        {
            printf("Successfully Bonded to ");
            print_bd_address(
                p_event_data->paired_device_link_keys_update.bd_addr);
        } 
        else 
        {
            result = WICED_BT_ERROR;
            printf("Failed to bond! \n");
        }
        break;

    case BTM_PAIRED_DEVICE_LINK_KEYS_REQUEST_EVT:
        /* read existing key from the NVRAM  */
        if (p_event_data == NULL) 
        {
            WICED_BT_TRACE("Callback data pointer p_event_data is NULL \n");
            break;
        }

        /* Paired Device Link Keys Request */
        printf("Paired Device Link keys Request Event for device ");
        print_bd_address(
            (uint8_t *)(p_event_data->paired_device_link_keys_request.bd_addr));

        /* Need to search to see if the BD_ADDR we are
         * looking for is in NVRAM. If not, we return WICED_BT_ERROR
         * and the stack will generate keys and will then call
         * BTM_PAIRED_DEVICE_LINK_KEYS_UPDATE_EVT so that they
         * can be stored
         */

        /* Assume the device won't be found.
         * If it is, we will set this back to WICED_BT_SUCCESS */
        result = WICED_BT_ERROR;
        bondindex = app_bt_find_device_in_nvm(
            p_event_data->paired_device_link_keys_request.bd_addr);

        if (BOND_INDEX_MAX > bondindex) 
        {
            /* Copy the keys to where the stack wants it */
            memcpy(&(p_event_data->paired_device_link_keys_request),
                &bond_info.link_keys[bondindex],
                sizeof(wiced_bt_device_link_keys_t));
            WICED_BT_TRACE(
                "BTM_PAIRED_DEVICE_LINK_KEYS_REQUEST_EVT: successfully read\n");
            result = WICED_BT_SUCCESS;
        } 
        else 
        {
            result = WICED_BT_ERROR;
            printf("Device Link Keys not found in the database! \n");
        }
        break;

    case BTM_PAIRING_IO_CAPABILITIES_BR_EDR_REQUEST_EVT:
        /* Use the default security for BR/EDR*/
        WICED_BT_TRACE(
            "BTM_PAIRING_IO_CAPABILITIES_BR_EDR_REQUEST_EVT bda %s\n",
            p_event_data->pairing_io_capabilities_br_edr_request.bd_addr);
        p_event_data->pairing_io_capabilities_br_edr_request.local_io_cap =
            BTM_IO_CAPABILITIES_NONE;
        p_event_data->pairing_io_capabilities_br_edr_request.auth_req =
            BTM_AUTH_SINGLE_PROFILE_GENERAL_BONDING_NO;
        p_event_data->pairing_io_capabilities_br_edr_request.oob_data =
            WICED_FALSE;
        break;

    case BTM_USER_CONFIRMATION_REQUEST_EVT:
        wiced_bt_dev_confirm_req_reply(
            WICED_BT_SUCCESS, p_event_data->user_confirmation_request.bd_addr);
        break;

    case BTM_ENCRYPTION_STATUS_EVT:
        p_encryption_status = &p_event_data->encryption_status;
        WICED_BT_TRACE("Encryption Status:(%s) res:%d\n",
            p_encryption_status->bd_addr,
            p_encryption_status->result);
        /* This call will return BOND_INDEX_MAX if the device is not found */
        bondindex = app_bt_find_device_in_nvm(
            p_event_data->encryption_status.bd_addr);

        if (bondindex < BOND_INDEX_MAX) {
            app_bt_restore_bond_data();
            app_bt_restore_cccd();
            printf("Bond info present in NVM for device: ");
            print_bd_address(p_event_data->encryption_status.bd_addr);
        } else {
            printf("No Bond info present in NVM for device: ");
            print_bd_address(p_event_data->encryption_status.bd_addr);
            bondindex = ZERO;
        }
        break;

    default:
        break;
    }

    return result;
}

/*******************************************************************************
* Function Name: handsfree_post_bt_init
********************************************************************************
* Summary:
*  Called after the stack init is success. Sets up EIR data, SDP database and
*  hfp init
*
* Parameters:
*  wiced_bt_management_evt_data_t *p_event_data : Pointer to BT management
*
* Return:
*  NONE
*
*******************************************************************************/
void handsfree_post_bt_init(wiced_bt_management_evt_data_t *p_event_data) 
{    
    if ((p_event_data) && (WICED_BT_SUCCESS == p_event_data->enabled.status)) 
    {
        WICED_BT_TRACE("Bluetooth stack initialized\n");

        handsfree_app_states.pairing_allowed = WICED_FALSE;

        /* Set-up EIR data */
        handsfree_write_eir();

        /* Set-up SDP database */
        wiced_bt_sdp_db_init((uint8_t *)handsfree_sdp_db,
            wiced_app_cfg_sdp_record_get_size());

        handsfree_hfp_init();
    } 
    else
    {
        WICED_BT_TRACE("Bluetooth stack initialization failure!!\n");
        return;
    }
}

/*******************************************************************************
* Function Name: handsfree_hfp_init
********************************************************************************
* Summary:
*  Configures the HF params, sets up voice data path, configures and
*  initiatizes MSBC module and microphone
*
* Parameters:
*  NONE
*
* Return:
*  NONE
*
*******************************************************************************/
void handsfree_hfp_init(void) 
{
    wiced_result_t result = WICED_BT_ERROR;
    wiced_bt_hfp_hf_config_data_t config;
    audio_playback_config pb_config_params;
    wiced_bt_enh_esco_params_t enh_esco_params;

    handsfree_init_context_data();

    config.feature_mask = BT_AUDIO_HFP_SUPPORTED_FEATURES;
    config.speaker_volume = handsfree_ctxt_data.spkr_volume;
    config.mic_volume = handsfree_ctxt_data.mic_volume;
    config.num_server = 1;
    config.scn[ZERO] = HANDS_FREE_SCN;
    config.uuid[ZERO] = UUID_SERVCLASS_HF_HANDSFREE;

    result = wiced_bt_hfp_hf_init(&config, handsfree_event_callback);

    /* Setup SCO PATH */
    handsfree_ctxt_data.sco_voice_path.path = WICED_BT_SCO_OVER_HCI;
    handsfree_ctxt_data.sco_voice_path.p_sco_data_cb =
        &handsfree_sco_data_app_callback;
    result = wiced_bt_sco_setup_voice_path(&handsfree_ctxt_data.sco_voice_path);

    WICED_BT_TRACE("[%s] SCO Setting up voice path = %d\n", __func__, result);

    wiced_bt_read_esco_parameters(WICED_BT_SCO_IDX_MSBC, &enh_esco_params);

    enh_esco_params.input_cf.coding_format = HCI_CODING_FORMAT_MSBC;
    enh_esco_params.output_cf.coding_format = HCI_CODING_FORMAT_MSBC;
    enh_esco_params.input_coded_data_size = ESCO_INPUT_DATA_SIZE;
    enh_esco_params.output_coded_data_size = ESCO_OUTPUT_DATA_SIZE;

    wiced_bt_config_esco_parameters(WICED_BT_SCO_IDX_MSBC, &enh_esco_params);

#if (WICED_BT_HFP_HF_WBS_INCLUDED == TRUE)
    WICED_BT_TRACE("%s WBS enabled\n",__func__);
    if (handsfree_esco_params.use_wbs == WICED_TRUE)
    {
        pb_config_params.samplingFreq = WBS_SAMPLE_RATE;
    }
    else
#endif
    {
        pb_config_params.samplingFreq = NBS_SAMPLE_RATE;
    }

    pb_config_params.channelMode        = SBC_MONO; /* Mono */
    pb_config_params.numOfChannels      = ONE;
    pb_config_params.allocationMethod   = ZERO; /* Loudness */
    pb_config_params.numOfSubBands      = ENCODER_SUB_BANDS;
    pb_config_params.numOfBlocks        = ENCODER_SUB_BLOCKS;
    pb_config_params.bitPool            = ENCODER_BIT_POOL;
    pb_config_params.sbc_mode           = SBC_DEC_MODE_WB;
    pb_config_params.uui_id             = ENCODER_UUI_ID;

    msbc_audio_init(pb_config_params);

    pdm_mic_interface_init();
}

/*******************************************************************************
* Function Name: handsfree_write_eir
********************************************************************************
* Summary:
*  Sets the EIR data to the stack
*
* Parameters:
*  NONE
*
* Return:
*  NONE
*
*******************************************************************************/
void handsfree_write_eir(void) 
{
    uint8_t *pBuf;
    uint8_t *p;
    uint8_t length;

    pBuf = (uint8_t *)wiced_bt_get_buffer(WICED_HS_EIR_BUF_MAX_SIZE);

    if (!pBuf) 
    {
        WICED_BT_TRACE("%s: wiced_bt_get_buffer returned NULL\n", __FUNCTION__);
    } 
    else 
    {
        p = pBuf;

        length = strlen((char *)handsfree_cfg_settings.device_name);

        *p++ = length + 1;

        /* EIR type full name */
        *p++ = 0x09;
        memcpy(p, handsfree_cfg_settings.device_name, length);
        p += length;

        /* length of services + 1 */
        *p++ = (1 * 2) + 1;

        /* EIR type full list of 16 bit service UUIDs */
        *p++ = 0x02;
        *p++ = UUID_SERVCLASS_HF_HANDSFREE & ONE_BYTE_MASK;
        *p++ = (UUID_SERVCLASS_HF_HANDSFREE >> ONE_BYTE_BITS) & ONE_BYTE_MASK;
        *p++ = UUID_SERVCLASS_GENERIC_AUDIO & ONE_BYTE_MASK;
        *p++ = (UUID_SERVCLASS_GENERIC_AUDIO >> ONE_BYTE_BITS) & ONE_BYTE_MASK;
        *p++ = ZERO;

        wiced_bt_dev_write_eir(pBuf, (uint16_t)(p - pBuf));
    }
    return;
}

/*******************************************************************************
* Function Name: handsfree_init_context_data
********************************************************************************
* Summary:
*   initializes the hanffree context data
*
* Parameters:
*   NONE
*
* Return:
*   NONE
*
*******************************************************************************/
void handsfree_init_context_data(void) 
{
    handsfree_ctxt_data.call_active = WICED_FALSE;
    handsfree_ctxt_data.call_held = WICED_FALSE;
    handsfree_ctxt_data.call_setup = WICED_BT_HFP_HF_CALLSETUP_STATE_IDLE;
    handsfree_ctxt_data.connection_status = WICED_BT_HFP_HF_STATE_DISCONNECTED;
    handsfree_ctxt_data.spkr_volume = BT_HFP_HF_DEF_SPK_VOL;
    handsfree_ctxt_data.mic_volume = BT_HFP_HF_DEF_MIC_VOL;
    handsfree_ctxt_data.sco_index = BT_AUDIO_INVALID_SCO_INDEX;
    handsfree_ctxt_data.init_sco_conn = WICED_FALSE;
}

/*******************************************************************************
* Function Name: handsfree_event_callback
********************************************************************************
* Summary:
*  Callback from the stack for Handsfree events
*
* Parameters:
*  wiced_bt_hfp_hf_event_t event : handsfree events
*  wiced_bt_hfp_hf_event_data_t* p_data : handsfree event data
*
* Return:
*  NONE
*
*******************************************************************************/
static void handsfree_event_callback(wiced_bt_hfp_hf_event_t event,
    wiced_bt_hfp_hf_event_data_t *p_data) 
{
    hci_control_hf_event_t p_val;

    memset(&p_val, ZERO, sizeof(hci_control_hf_event_t));

    switch (event) 
    {
    case WICED_BT_HFP_HF_CONNECTION_STATE_EVT:
        handsfree_connection_event_handler(p_data);
        break;

    case WICED_BT_HFP_HF_AG_FEATURE_SUPPORT_EVT:
        p_val.conn.peer_features = p_data->ag_feature_flags;

        if (p_data->ag_feature_flags &
            WICED_BT_HFP_AG_FEATURE_INBAND_RING_TONE_CAPABILITY) 
        {
            handsfree_ctxt_data.inband_ring_status =
                WICED_BT_HFP_HF_INBAND_RING_ENABLED;
        }
        else 
        {
            handsfree_ctxt_data.inband_ring_status =
                WICED_BT_HFP_HF_INBAND_RING_DISABLED;
        }
        WICED_BT_TRACE("%s : AG Feature Flags = 0x%X\n",
            handsfree_get_indicator_name(event),
            p_data->ag_feature_flags);
        break;

    case WICED_BT_HFP_HF_SERVICE_STATE_EVT:
        WICED_BT_TRACE("%s : Service = %d\n",
            handsfree_get_indicator_name(event),
            p_data->service_state);
        break;

    case WICED_BT_HFP_HF_RSSI_IND_EVT:
        WICED_BT_TRACE("%s : RSSI = %d\n", handsfree_get_indicator_name(event),
            p_data->rssi);
        break;

    case WICED_BT_HFP_HF_SERVICE_TYPE_EVT:
        WICED_BT_TRACE("%s : Service Type = %d\n",
            handsfree_get_indicator_name(event),
            p_data->service_type);
        break;

    case WICED_BT_HFP_HF_BATTERY_STATUS_IND_EVT:
        WICED_BT_TRACE("%s : Battery Level = %d\n",
            handsfree_get_indicator_name(event),
            p_data->battery_level);
        break;

    case WICED_BT_HFP_HF_CALL_SETUP_EVT:
        WICED_BT_TRACE("%s :", handsfree_get_indicator_name(event));
        handsfree_call_setup_event_handler(&p_data->call_data);
        break;

    case WICED_BT_HFP_HF_RING_EVT:
    case WICED_BT_HFP_HF_OK_EVT:
    case WICED_BT_HFP_HF_ERROR_EVT:
        WICED_BT_TRACE("%s\n", handsfree_get_indicator_name(event));
        break;

    case WICED_BT_HFP_HF_INBAND_RING_STATE_EVT:
        handsfree_ctxt_data.inband_ring_status = p_data->inband_ring;
        WICED_BT_TRACE("%s : %d\n", handsfree_get_indicator_name(event),
            p_data->inband_ring);
        break;

    case WICED_BT_HFP_HF_CME_ERROR_EVT:
        WICED_BT_TRACE("%s : Error = %d\n", handsfree_get_indicator_name(event),
            p_data->error_code);
        break;

    case WICED_BT_HFP_HF_CLIP_IND_EVT:
        WICED_BT_TRACE("%s : Number = %s, Type = %d\n",
            handsfree_get_indicator_name(event),
            p_data->clip.caller_num, p_data->clip.type);
        break;

    case WICED_BT_HFP_HF_BINP_EVT:
        WICED_BT_TRACE("%s : Number = %s, Type = %d\n",
            handsfree_get_indicator_name(event),
            p_data->binp_data.caller_num, p_data->binp_data.type);
        break;

    case WICED_BT_HFP_HF_VOLUME_CHANGE_EVT:
        WICED_BT_TRACE(
            "%s : %s VOLUME = %d \n", handsfree_get_indicator_name(event),
            (p_data->volume.type == WICED_BT_HFP_HF_SPEAKER) ? "SPK (+VGS)"
                                                             : "MIC (+VGS)",
            p_data->volume.level);
        break;

    case WICED_BT_HFP_HFP_CODEC_SET_EVT:
        if (p_data->selected_codec == WICED_BT_HFP_HF_MSBC_CODEC) 
        {
            handsfree_esco_params.use_wbs = WICED_TRUE;
        } 
        else 
        {
            handsfree_esco_params.use_wbs = WICED_FALSE;
        }
        WICED_BT_TRACE("%s : %d \n", handsfree_get_indicator_name(event),
            p_data->selected_codec);
        break;

    case WICED_BT_HFP_HFP_ACTIVE_CALL_EVT:
        WICED_BT_TRACE("%s : idx = %d, dir = %d, status = %d, mode = %d, "
            "is_conference = %d, num = %s, type = %d\n",
            handsfree_get_indicator_name(event),
            p_data->active_call.idx, p_data->active_call.dir,
            p_data->active_call.status, p_data->active_call.mode,
            p_data->active_call.is_conference,
            p_data->active_call.num, p_data->active_call.type);
        break;

    case WICED_BT_HFP_HF_CNUM_EVT:
        WICED_BT_TRACE("%s : cnum data = %s\n",
            handsfree_get_indicator_name(event), p_data->cnum_data);
        break;

    case WICED_BT_HFP_HF_BIND_EVT:
        WICED_BT_TRACE("%s : id = %d, val = %d\n",
            handsfree_get_indicator_name(event),
            p_data->bind_data.ind_id, p_data->bind_data.ind_value);
        break;

    case WICED_BT_HFP_HF_COPS_EVT:
        break;

    default:
        break;
    }
}

/*******************************************************************************
* Function Name: handsfree_connection_event_handler
********************************************************************************
* Summary:
*  Hander funtion for connection events (Connected/Disconnected/SLC connected)
*
* Parameters:
*  wiced_bt_hfp_hf_event_data_t* p_data : Connection Event data
*
* Return:
*  None
*
*******************************************************************************/
static void handsfree_connection_event_handler(
    wiced_bt_hfp_hf_event_data_t *p_data) 
{
    wiced_bt_dev_status_t status;

    if (NULL != p_data) 
    {
        handsfree_ctxt_data.connection_status = p_data->conn_data.conn_state;

        if (WICED_BT_HFP_HF_STATE_CONNECTED == p_data->conn_data.conn_state) 
        {
            hci_control_hf_open_t open;
            wiced_bt_hfp_hf_scb_t *p_scb = wiced_bt_hfp_hf_get_scb_by_bd_addr(
                p_data->conn_data.remote_address);
            memcpy(open.bd_addr, p_data->conn_data.remote_address, BD_ADDR_LEN);
            open.status = WICED_BT_SUCCESS;
            handsfree_ctxt_data.rfcomm_handle = p_scb->rfcomm_handle;

            if (WICED_BT_HFP_PROFILE == p_data->conn_data.connected_profile) 
            {
                handsfree_app_states.connect.profile_selected =
                    WICED_BT_HFP_PROFILE;
            } 
            else 
            {
                handsfree_app_states.connect.profile_selected =
                    WICED_BT_HSP_PROFILE;
                memcpy(handsfree_ctxt_data.peer_bd_addr,
                    p_data->conn_data.remote_address,
                    sizeof(wiced_bt_device_address_t));
            }

            status =
                wiced_bt_sco_create_as_acceptor(&handsfree_ctxt_data.sco_index);
            WICED_BT_TRACE("%s: Connection: status [%d] Connection Handle [%x] "
                "SCO INDEX [%d] \n",
                __func__, status, p_scb->rfcomm_handle,
                handsfree_ctxt_data.sco_index);
        } 
        else if (WICED_BT_HFP_HF_STATE_SLC_CONNECTED == 
            p_data->conn_data.conn_state) 
        {
            WICED_BT_TRACE("%s: SLC Connected : Peer BD Addr [%s]\n", __func__,
                p_data->conn_data.remote_address);

            memcpy(handsfree_ctxt_data.peer_bd_addr,
                p_data->conn_data.remote_address,
                sizeof(wiced_bt_device_address_t));
            handsfree_print_hfp_context();
        } 
        else if (WICED_BT_HFP_HF_STATE_DISCONNECTED == 
            p_data->conn_data.conn_state) 
        {
            memset(handsfree_ctxt_data.peer_bd_addr, ZERO,
                sizeof(wiced_bt_device_address_t));
            handsfree_ctxt_data.rfcomm_handle = ZERO;
            if (BT_AUDIO_INVALID_SCO_INDEX != handsfree_ctxt_data.sco_index) 
            {
                status = wiced_bt_sco_remove(handsfree_ctxt_data.sco_index);
                handsfree_ctxt_data.sco_index = BT_AUDIO_INVALID_SCO_INDEX;
                WICED_BT_TRACE("%s: remove sco status [%d] \n", __func__,
                    status);
            }
            handsfree_print_hfp_context();
        }
    } 
    else 
    {
        WICED_BT_TRACE("%s: p_data is NULL\n", __FUNCTION__);
    }
}

/*******************************************************************************
* Function Name: hf_sco_management_callback
********************************************************************************
* Summary:
*  Callback from Stack for SCO events
*
* Parameters:
*  wiced_bt_management_evt_t event : SCO events
*  wiced_bt_management_evt_data_t *p_event_data : SCO Event data
*
* Return:
*  NONE
*
*******************************************************************************/
void hf_sco_management_callback(wiced_bt_management_evt_t event,
    wiced_bt_management_evt_data_t *p_event_data) 
{
    int status;

    WICED_BT_TRACE("hf_sco_management_callback handsfree_management_callback. "
        "Event: 0x%x %s\n",
        event, get_bt_event_name(event));

    switch (event) 
    {
    /**< SCO connected event. Event data: #wiced_bt_sco_connected_t */
    case BTM_SCO_CONNECTED_EVT:
        WICED_BT_TRACE("%s: SCO Audio connected, sco_index = %d [in context "
            "sco index=%d]\n",
            __func__, p_event_data->sco_connected.sco_index,
            handsfree_ctxt_data.sco_index);

        handsfree_ctxt_data.is_sco_connected = WICED_TRUE;
        handsfree_ctxt_data.sco_index = p_event_data->sco_connected.sco_index;
        handsfree_print_hfp_context();
        break;

    /**< SCO disconnected event. Event data: #wiced_bt_sco_disconnected_t */
    case BTM_SCO_DISCONNECTED_EVT:
        WICED_BT_TRACE("%s: SCO disconnection change event handler\n",
            __func__);

        status =
            wiced_bt_sco_create_as_acceptor(&handsfree_ctxt_data.sco_index);
        WICED_BT_TRACE("%s: status [%d] SCO INDEX [%d] \n", __func__, status,
            handsfree_ctxt_data.sco_index);
        handsfree_ctxt_data.is_sco_connected = WICED_FALSE;
        handsfree_ctxt_data.sco_index = BT_AUDIO_INVALID_SCO_INDEX;
        handsfree_print_hfp_context();
        break;

    /**< SCO connection request event. Event data:
     * #wiced_bt_sco_connection_request_t */
    case BTM_SCO_CONNECTION_REQUEST_EVT:
        WICED_BT_TRACE("%s: SCO connection request event handler \n", __func__);
        
        if (WICED_BT_HFP_PROFILE == handsfree_app_states.connect.profile_selected)
        {
            WICED_BT_TRACE(
                "Accept SCO Conn with eSCO param: \n max_latency = %d; "
                "packet_types = %d; retrans_effort = %d; use_wbs = %d\n\n",
                handsfree_esco_params.max_latency,
                handsfree_esco_params.packet_types,
                handsfree_esco_params.retrans_effort,
                handsfree_esco_params.use_wbs);

            wiced_bt_sco_accept_connection(
                p_event_data->sco_connection_request.sco_index, HCI_SUCCESS,
                (wiced_bt_sco_params_t *)&handsfree_esco_params);
        }
        break;

    /**< SCO connection change event. Event data:
     * #wiced_bt_sco_connection_change_t */
    case BTM_SCO_CONNECTION_CHANGE_EVT:
        WICED_BT_TRACE("%s: SCO connection change event handler\n", __func__);
        break;
    }
}

/*******************************************************************************
* Function Name: handsfree_inquiry_result_cback
********************************************************************************
* Summary:
*  Callback function called from stack for Inquiry Results
*
* Parameters:
*  wiced_bt_dev_inquiry_scan_result_t *p_inquiry_result : Inquiry results
*  uint8_t *p_eir_data : EIR data
*
* Return:
*
*******************************************************************************/
void handsfree_inquiry_result_cback(
    wiced_bt_dev_inquiry_scan_result_t *p_inquiry_result, uint8_t *p_eir_data) 
{
    if (p_inquiry_result == NULL) {
        printf("Inquiry Complete \n");
    } else {
        printf("\n--------------------------------------------\n");
        printf("Inquiry Result: %02X %02X %02X %02X %02X %02X\n",
            p_inquiry_result->remote_bd_addr[BD_ADDR_INDEX_ZERO],
            p_inquiry_result->remote_bd_addr[BD_ADDR_INDEX_ONE],
            p_inquiry_result->remote_bd_addr[BD_ADDR_INDEX_TWO],
            p_inquiry_result->remote_bd_addr[BD_ADDR_INDEX_THREE],
            p_inquiry_result->remote_bd_addr[BD_ADDR_INDEX_FOUR],
            p_inquiry_result->remote_bd_addr[BD_ADDR_INDEX_FIVE]);
        printf("Clock Offset = 0x%x\n", p_inquiry_result->clock_offset);
        printf("RSSI = %d\n", p_inquiry_result->rssi);
        printf("--------------------------------------------\n");
    }
}

/*******************************************************************************
* Function Name: handsfree_print_hfp_context
********************************************************************************
* Summary:
*  This Function prints the current HFP connection status
*
* Parameters:
*  NONE
*
* Return:
*  NONE
*
*******************************************************************************/
void handsfree_print_hfp_context(void) 
{
    printf("\n----------------HF CONNECTION "
        "DETAILS----------------------------\n");
    printf("BD ADDRESS \t\t CONNECTION HANDLE \t SCO INDEX\n");
    printf("%02X %02X %02X %02X %02X %02X \t %X \t\t\t %X \n",
        handsfree_ctxt_data.peer_bd_addr[BD_ADDR_INDEX_ZERO],
        handsfree_ctxt_data.peer_bd_addr[BD_ADDR_INDEX_ONE],
        handsfree_ctxt_data.peer_bd_addr[BD_ADDR_INDEX_TWO],
        handsfree_ctxt_data.peer_bd_addr[BD_ADDR_INDEX_THREE],
        handsfree_ctxt_data.peer_bd_addr[BD_ADDR_INDEX_FOUR],
        handsfree_ctxt_data.peer_bd_addr[BD_ADDR_INDEX_FIVE],
        handsfree_ctxt_data.rfcomm_handle,
        handsfree_ctxt_data.sco_index);
    printf(
        "-----------------------------------------------------------------\n");
}

/*******************************************************************************
* Function Name: handsfree_get_indicator_name
********************************************************************************
* Summary:
*  Prints details about the indicator / Events received from AG
*
* Parameters:
*  wiced_bt_hfp_hf_event_t event : Indicator / Event received from AG
*
* Return:
*  const char *: Returns details about the indicator / Events received from AG
*  as a constant string
*
*******************************************************************************/
static const char *handsfree_get_indicator_name(wiced_bt_hfp_hf_event_t event) 
{
    WICED_BT_TRACE("RCVD FROM AG >> ");

    switch (event) {
    case WICED_BT_HFP_HF_AG_FEATURE_SUPPORT_EVT:
        return "+BRSF";

    /*WICED_BT_HFP_HF_SERVICE_STATE_EVT - received for both +CIEV and +CIND*/
    case WICED_BT_HFP_HF_SERVICE_STATE_EVT:
        return "Service Availability Indicator";

    case WICED_BT_HFP_HF_RSSI_IND_EVT:
        return "Signal Indicator"; /*received for both +CIEV and +CIND*/

    case WICED_BT_HFP_HF_SERVICE_TYPE_EVT:
        return "Service Type"; /*received for both +CIEV and +CIND*/

    /*WICED_BT_HFP_HF_BATTERY_STATUS_IND_EVT - received for both +CIEV and
     * +CIND*/
    case WICED_BT_HFP_HF_BATTERY_STATUS_IND_EVT:
        return "Battery Charge Indicator";

    case WICED_BT_HFP_HF_CALL_SETUP_EVT:
    case WICED_BT_HFP_HF_RING_EVT:
        return "RING";
        
    case WICED_BT_HFP_HF_INBAND_RING_STATE_EVT:
        return "+BSIR: Inband Ring Tone Status";

    case WICED_BT_HFP_HF_OK_EVT:
        return "OK";

    case WICED_BT_HFP_HF_ERROR_EVT:
        return "AT+ERROR";

    case WICED_BT_HFP_HF_CME_ERROR_EVT:
        return "+CME";

    case WICED_BT_HFP_HF_CLIP_IND_EVT:
        return "+CLIP";

    case WICED_BT_HFP_HF_BINP_EVT:
        return "+BNEP";

    case WICED_BT_HFP_HF_VOLUME_CHANGE_EVT:
        return "Audio Volume Gain";

    case WICED_BT_HFP_HFP_CODEC_SET_EVT:
        return "+BCS: Codec Selection";

    case WICED_BT_HFP_HFP_ACTIVE_CALL_EVT:
        return "+CLCC: Active Call Status";

    case WICED_BT_HFP_HF_CNUM_EVT:
        return "+CNUM: Subscriber Number Info";

    case WICED_BT_HFP_HF_BIND_EVT:
        return "+BIND: AG Supported Indicators";

    default:
        return "Indicator Unknown";
    }
}

/*******************************************************************************
* Function Name: handsfree_call_setup_event_handler
********************************************************************************
* Summary:
*  Prints the Call Status details based on the call setup event received from
*  AG
*
* Parameters:
*  wiced_bt_hfp_hf_call_data_t* call_data : call setup data
*
* Return:
*  NONE
*
*******************************************************************************/
static void handsfree_call_setup_event_handler(
    wiced_bt_hfp_hf_call_data_t* call_data)
{
    if (NULL != call_data)
    {
        if (handsfree_ctxt_data.call_active != call_data->active_call_present)
        {
            printf("Call Status = %d\n", call_data->active_call_present);
        }
        if (handsfree_ctxt_data.call_held != call_data->held_call_present)
        {
            printf("Call Held = %d\n", call_data->held_call_present);
        }
        if (handsfree_ctxt_data.call_setup != call_data->setup_state)
        {
            printf("Call Setup = %d\n", call_data->setup_state);
        }
        printf("\n-----------CALL SETUP DETAILS------------------\n");

        /* Third party call (Call Held) events are not handled */
        switch (call_data->setup_state)
        {
            case WICED_BT_HFP_HF_CALLSETUP_STATE_INCOMING:
                printf("Call(incoming) setting-up\n");
                start_audio_playback();
                incoming_call = true;
                break;

            case WICED_BT_HFP_HF_CALLSETUP_STATE_IDLE:
                if (WICED_FALSE == call_data->active_call_present)
                {
                    incoming_call = false;
                    if ((WICED_BT_HFP_HF_CALLSETUP_STATE_INCOMING == 
                        handsfree_ctxt_data.call_setup) ||
                        (WICED_BT_HFP_HF_CALLSETUP_STATE_DIALING == 
                        handsfree_ctxt_data.call_setup) ||
                        (WICED_BT_HFP_HF_CALLSETUP_STATE_ALERTING == 
                        handsfree_ctxt_data.call_setup))
                    {
                        printf("Call: Inactive; Call Set-up: IDLE\n");
                        stop_audio_playback();
                        
                        break;
                    }
                    /* If previous context has an active-call and 
                     * active_call_present is 0 
                     */
                    if (WICED_TRUE == handsfree_ctxt_data.call_active)
                    {
                        printf("Call Terminated\n");

                        stop_audio_capture();
                        stop_audio_playback();
                        break;
                    }
                }
                /* On call active, 1st we receive +CIEV:<CALL>,1 and later call 
                 * set up DONE (+CIEV:<CALLSETUP>,0) is sent */
                else if (WICED_TRUE == call_data->active_call_present)
                {
                    /* Following case if for +CIEV:<CALL>,1 when call is 
                     * picked up 
                     */
                    if (call_data->active_call_present != 
                        handsfree_ctxt_data.call_active)
                    {
                        printf("Call: Active\n");
                        handsfree_ctxt_data.call_active = 
                            call_data->active_call_present;

                        /* Enable noise reduction and echo cancellation on 
                         * remote end 
                         */
                        handsfree_send_at_command(
                            (uint16_t)handsfree_ctxt_data.rfcomm_handle, 
                            HF_AT_CMD_NREC, NREC_ENABLED, NULL);

                        /* Start Capture */
                        start_audio_capture();
                    }
                    /* Following case if for +CIEV:<CALLSETUP>,0 when call set 
                     * up done is sent 
                     */
                    else
                    {
                        printf("Call-setup: DONE\n");
                    }
                }
                break;

            case WICED_BT_HFP_HF_CALLSETUP_STATE_DIALING:
                printf("Call(outgoing) setting-up\n");
                start_audio_playback();
                break;

            case WICED_BT_HFP_HF_CALLSETUP_STATE_ALERTING:
                printf("Remote(outgoing) ringing\n");
                break;

            default:
                break;
        }

        handsfree_ctxt_data.call_active = call_data->active_call_present;
        handsfree_ctxt_data.call_setup  = call_data->setup_state;
        handsfree_ctxt_data.call_held   = call_data->held_call_present;
        printf("-----------------------------------------------\n");
    }
    else
    {
        printf("%s: call data is NULL\n",__FUNCTION__);
    }
}

/*******************************************************************************
* Function Name: handsfree_sco_data_app_callback
********************************************************************************
* Summary:
*  callback function called for incoming pcm data
*
* Parameters:
*  uint16_t sco_channel : sco channel
*  uint16_t length : SCO data callback length in bytes
*  uint8_t* p_data : incoming SCO pcm data
*
* Return:
*  NONE
*
*******************************************************************************/
static void handsfree_sco_data_app_callback(uint16_t sco_channel,
    uint16_t length, uint8_t *p_data) 
{
    uint32_t mic_data_len = ZERO;
    wiced_result_t result = WICED_ERROR;

    if (length) 
    {
        msbc_write_pcm_data(p_data, length);
    }
    
    if (handsfree_ctxt_data.call_active &&
        (WICED_TRUE == handsfree_ctxt_data.is_sco_connected)) 
    {
        mic_data_len = capture_mic_data(mic_data, length);
        if (ZERO < mic_data_len) 
        {
            result = wiced_bt_sco_write_buffer(handsfree_ctxt_data.sco_index,
                mic_data, mic_data_len);
            if (WICED_BT_SUCCESS != result) 
            {
                WICED_BT_TRACE("wiced_bt_sco_write_buffer error, sco_index = "
                    "%d, result = %hu\n",
                    handsfree_ctxt_data.sco_index, result);
            }
        }
    }
}

/*******************************************************************************
* Function Name: wait_init_done
********************************************************************************
* Summary:
*  This Function block to wait the BT stack FW download done
*  (notify_init_done)
*
* Parameters:
*  NONE
*
* Return:
*  NONE
*
*******************************************************************************/
void wait_init_done(void) 
{
    WICED_BT_TRACE("Waiting FW Download\r\n");
    xSemaphoreTake(bt_semaphore, portMAX_DELAY);
    WICED_BT_TRACE("wait_init_done[Release...]\r\n");
}

/*******************************************************************************
* Function Name: notify_init_done
********************************************************************************
* Summary:
*  This Function notify and release the blocking of wait_init_done()
*
* Parameters:
*  NONE
*
* Return:
*  NONE
*
*******************************************************************************/
void notify_init_done(void) 
{
    WICED_BT_TRACE("notify_init_done\r\n");
    xSemaphoreGive(bt_semaphore);
}

/* END OF FILE [] */
