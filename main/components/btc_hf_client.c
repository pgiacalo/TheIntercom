/*
 * SPDX-FileCopyrightText: 2015-2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/******************************************************************************
 **
 **  Name:          btc_hf_client.c
 **
 ******************************************************************************/

/*
btc_hf_client.c (Summary)

Module Overview:
The `btc_hf_client.c` module is part of the Bluetooth Hands-Free (HF) Client implementation, facilitating Bluetooth communication between a device and a Hands-Free service. This module specifically manages the HF Client's actions, from initializing connections to controlling voice calls, audio, and other Hands-Free Profile functionalities.

Key Functionalities:

1. Initialization and De-initialization:
   - The module provides functions to initialize (`btc_hf_client_init`) and de-initialize (`btc_hf_client_deinit`) the HF Client. Initialization involves setting default values, while de-initialization clears memory and deregisters the service.
   
2. Connection Management:
   - The module allows for connecting and disconnecting with remote devices (`btc_hf_client_connect`, `btc_hf_client_disconnect`) and their audio endpoints (`btc_hf_client_connect_audio`, `btc_hf_client_disconnect_audio`).
   
3. Voice and Call Control:
   - The HF Client can manage calls by answering (`btc_hf_client_answer_call`), rejecting (`btc_hf_client_reject_call`), dialing (`btc_hf_client_dial`), and sending DTMF codes (`btc_hf_client_send_dtmf`). It can also start and stop voice recognition sessions.
   
4. Event and Command Processing:
   - `btc_hf_client_cb_handler` is a primary callback handler processing various events, such as changes in connection states, audio management, indicators (e.g., volume, battery level), and AT command responses.
   - `btc_hf_client_call_handler` processes direct commands sent to the HF Client, ranging from voice recognition to query-related commands.
   
5. Queries and Information Retrieval:
   - The module provides functionalities to retrieve current call lists, operator names, and subscriber info, among other details.
   
6. Configuration and Settings:
   - Various functions allow for the configuration of audio properties, volume settings, sending specific AT commands, and more.
   
7. Utility and Helper Functions:
   - There are numerous utility functions that perform tasks like checking connection status, sending AT commands, and handling different events.
   
8. Custom Modifications:
   - Comments within the code suggest that there have been custom modifications, particularly ensuring that `connect()` and `connect_audio()` methods are always called post `init()`.

This module ensures that all HF Client features are coherent, effectively turning the device into a compliant Bluetooth Hands-Free entity.

===== 1st chunk ====
btc_hf_client.c

Overall Responsibility: 
This source file manages the operations related to a Bluetooth Hands-Free client profile, such as connecting, disconnecting, managing audio connections, voice recognition, volume controls, and other call management tasks.

Important Variables:

1. btc_hf_client_version: Denotes the version of the Hands-Free Client.
2. hf_client_local_param: A local parameter structure related to the hands-free client operations.

Important Functions:

1. btc_hf_client_cb_to_app(esp_hf_client_cb_event_t event, esp_hf_client_cb_param_t *param):
   - Purpose: Callback to the app layer when specific events occur.
   
2. clear_state(void):
   - Purpose: Clears the internal state of the hands-free client.

3. is_connected(bt_bdaddr_t *bd_addr):
   - Purpose: Check if the device is connected.

4. btc_hf_client_incoming_data_cb_to_app(const uint8_t *data, uint32_t len) and btc_hf_client_outgoing_data_cb_to_app(uint8_t *data, uint32_t len):
   - Purpose: Callbacks to handle incoming and outgoing data.

5. btc_hf_client_init(void):
   - Purpose: Initialize the hands-free client profile.

6. btc_hf_client_connect(bt_bdaddr_t *bd_addr):
   - Purpose: Connects to an audio gateway using the given Bluetooth address.

7. btc_hf_client_deinit(void):
   - Purpose: De-initializes the hands-free client.

8. btc_hf_client_disconnect(bt_bdaddr_t *bd_addr):
   - Purpose: Disconnects from the audio gateway.

9. btc_hf_client_connect_audio(bt_bdaddr_t *bd_addr) and btc_hf_client_disconnect_audio(bt_bdaddr_t *bd_addr):
   - Purpose: Functions to connect or disconnect the audio.

10. btc_hf_client_start_voice_recognition(void) and btc_hf_client_stop_voice_recognition(void):
    - Purpose: Start and stop voice recognition.

11. btc_hf_client_volume_update(esp_hf_volume_control_target_t type, int volume):
    - Purpose: Updates the volume.

12. btc_hf_client_dial(const char *number):
    - Purpose: Place a call to the given number.

13. btc_hf_client_dial_memory(int location):
    - Purpose: Dial a number based on a memory location (speed dial).

14. btc_hf_client_send_chld_cmd(esp_hf_chld_type_t type, int idx):
    - Purpose: Sends a "Call Hold" command.

15. btc_hf_client_send_btrh_cmd(esp_hf_btrh_cmd_t btrh):
    - Purpose: Sends a "Response and Hold" command.

16. btc_hf_client_answer_call(void) and btc_hf_client_reject_call(void):
    - Purpose: Answer or reject an incoming call.

17. btc_hf_client_query_current_calls(void):
    - Purpose: Query the current calls.

18. btc_hf_client_query_current_operator_name(void):
    - Purpose: Query the current operator's name.

The chunk of code provided primarily manages the functions and operations related to a Hands-Free client profile in Bluetooth. It provides APIs for initiating, managing, and terminating calls and audio connections, and for controlling features like volume and voice recognition.


======== 2nd chunk ====

btc_hf_client.c (Second Chunk)

Overall Responsibility: 
This section further extends the management of the operations related to a Bluetooth Hands-Free client profile, primarily focusing on querying various features and handling specific Bluetooth events.

Important Functions:

1. btc_hf_client_retrieve_subscriber_info(void):
   - Purpose: Retrieves subscriber number information.
   
2. btc_hf_client_send_dtmf(char code):
   - Purpose: Sends DTMF (Dual-Tone Multi-Frequency) codes, which represent the buttons pressed on a telephone keypad.

3. btc_hf_client_send_xapl(char *buf, UINT32 features):
   - Purpose: Sends the XAPL (iPhone Accessory Protocol) command.

4. btc_hf_client_send_iphoneaccev(uint32_t bat_level, BOOLEAN docked):
   - Purpose: Sends IPHONEACCEV (iPhone Accessory Event) command. Likely related to iPhone-specific notifications.

5. btc_hf_client_request_last_voice_tag_number(void):
   - Purpose: Requests the number from the AG (Audio Gateway) for Voice Recognition purposes.

6. btc_hf_client_send_nrec(void):
   - Purpose: Requests the AG to disable Echo Cancellation and Noise Reduction features.

7. btc_hf_client_pkt_stat_nums_get(UINT16 sync_conn_handle):
   - Purpose: Requests packet status numbers for a specific (e)SCO connection handle.

8. bte_hf_client_evt(tBTA_HF_CLIENT_EVT event, void *p_data):
   - Purpose: Switches the context from BTE (Bluetooth Embedded) to BTIF (Bluetooth Interface) for all HF Client events.

9. btc_hf_client_execute_service(BOOLEAN b_enable):
   - Purpose: Initializes or shuts down the service based on the provided flag.

10. process_ind_evt(tBTA_HF_CLIENT_IND *ind):
    - Purpose: Processes indicator events and sends them to the application layer.

The second chunk of code continues with more specific features related to the Hands-Free client profile in Bluetooth. Functions cover a range of tasks from retrieving subscriber information, sending DTMF tones, managing iPhone-specific events, to processing various other Bluetooth indicator events.

====== 3rd chunk ===

btc_hf_client.c (Third Chunk)

Overall Responsibility:
This chunk is focused on the handler functions for processing messages or commands sent to the Hands-Free (HF) client. The operations range from initializing and de-initializing the service, managing call controls, querying subscriber information, and more.

Important Functions:

1. btc_hf_client_cb_handler(btc_msg_t *msg):
   - Purpose: Handles various events related to the HF Client and processes them. It appears to be a primary callback handler for incoming events, deciding the actions or responses based on the event type.
   
2. btc_hf_client_call_handler(btc_msg_t *msg):
   - Purpose: Processes a set of commands directed to the HF Client. For example, commands related to voice recognition, volume control, dialing, and other Hands-Free Profile functionalities.

Detailed Breakdown:

The `btc_hf_client_cb_handler` function processes various events:

- Connection events: Handles changes in connection states, such as when a device connects or disconnects.
- Audio events: Manages the audio connection and its properties.
- Indicator events: Handles various indicators, such as volume, battery level, ring indications, etc.
- Command responses: Processes the responses to AT commands sent to the HF Client.
- Others: Manages other miscellaneous events.

The `btc_hf_client_call_handler` function processes commands:

- Initialization and de-initialization: Sets up and tears down the HF Client.
- Connection and disconnection: Controls connections to devices and audio endpoints.
- Voice recognition: Starts and stops voice recognition.
- Call control: Handles actions related to calls, such as dialing, answering, rejecting, and sending DTMF tones.
- Queries: Retrieves information like the current call list, operator name, and subscriber info.
- Configuration and settings: Updates volume, sends specific AT commands, and more.

Notably, there are comments suggesting that someone named "PHIL" added code to ensure that the `connect()` and `connect_audio()` methods are always called after `init()`. This might be a tweak or modification done to the original code.

The code ends with a conditional compilation block ensuring that these functionalities are only compiled when the HF Client feature is enabled.
*/

#include "common/bt_target.h"
#include "common/bt_trace.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdlib.h>
#include "common/bt_defs.h"
#include "device/bdaddr.h"
#include "btc/btc_dm.h"
#include "btc_hf_client.h"
#include "btc/btc_profile_queue.h"
#include "osi/allocator.h"
#include "btc/btc_manage.h"
#include "btc/btc_util.h"
#include "esp_hf_client_api.h"
#include "bta/bta_hf_client_api.h"
#include "esp_bt.h"
#include <assert.h>

#if (BTC_HF_CLIENT_INCLUDED == TRUE)

/************************************************************************************
**  Constants & Macros
************************************************************************************/

#ifndef BTC_HF_CLIENT_SERVICE_NAME
#define BTC_HF_CLIENT_SERVICE_NAME ("Handsfree")
#endif

#ifndef BTC_HF_CLIENT_SECURITY
#define BTC_HF_CLIENT_SECURITY    (BTA_SEC_AUTHENTICATE | BTA_SEC_ENCRYPT)
#endif

#ifndef BTC_HF_CLIENT_FEATURES
#define BTC_HF_CLIENT_FEATURES   ( BTA_HF_CLIENT_FEAT_ECNR    | \
                                    BTA_HF_CLIENT_FEAT_3WAY   | \
                                    BTA_HF_CLIENT_FEAT_CLI    | \
                                    BTA_HF_CLIENT_FEAT_VREC   | \
                                    BTA_HF_CLIENT_FEAT_VOL    | \
                                    BTA_HF_CLIENT_FEAT_ECS    | \
                                    BTA_HF_CLIENT_FEAT_ECC    | \
                                    BTA_HF_CLIENT_FEAT_CODEC)
#endif



/************************************************************************************
**  Static variables
************************************************************************************/
const int btc_hf_client_version = HFP_HF_VERSION_1_7;

#if HFP_DYNAMIC_MEMORY == FALSE
static hf_client_local_param_t hf_client_local_param;
#else
hf_client_local_param_t *hf_client_local_param_ptr;
#endif

/************************************************************************************
**  Static functions
************************************************************************************/
#define CHECK_HF_CLIENT_INIT() do { \
if (! hf_client_local_param.btc_hf_client_cb.initialized) { \
    return BT_STATUS_NOT_READY; \
} \
} while (0)

#define CHECK_HF_CLIENT_SLC_CONNECTED() do { \
if (! hf_client_local_param.btc_hf_client_cb.initialized || \
    hf_client_local_param.btc_hf_client_cb.state != ESP_HF_CLIENT_CONNECTION_STATE_SLC_CONNECTED) { \
    return BT_STATUS_NOT_READY; \
} \
} while (0)

static inline void btc_hf_client_cb_to_app(esp_hf_client_cb_event_t event, esp_hf_client_cb_param_t *param)
{
    esp_hf_client_cb_t btc_hf_client_callback = (esp_hf_client_cb_t)btc_profile_cb_get(BTC_PID_HF_CLIENT);
    if (btc_hf_client_callback) {
        btc_hf_client_callback(event, param);
    }
}

static void clear_state(void)
{
    memset(&hf_client_local_param.btc_hf_client_cb, 0, sizeof(btc_hf_client_cb_t));
}

static BOOLEAN is_connected(bt_bdaddr_t *bd_addr)
{
    if (((hf_client_local_param.btc_hf_client_cb.state == ESP_HF_CLIENT_CONNECTION_STATE_CONNECTED) ||
            (hf_client_local_param.btc_hf_client_cb.state == ESP_HF_CLIENT_CONNECTION_STATE_SLC_CONNECTED))&&
        ((bd_addr == NULL) || (bdcmp(bd_addr->address, hf_client_local_param.btc_hf_client_cb.connected_bda.address) == 0)))
        return TRUE;
    return FALSE;
}

void btc_hf_client_reg_data_cb(esp_hf_client_incoming_data_cb_t recv,
                               esp_hf_client_outgoing_data_cb_t send)
{
    hf_client_local_param.btc_hf_client_incoming_data_cb = recv;
    hf_client_local_param.btc_hf_client_outgoing_data_cb = send;
}

void btc_hf_client_incoming_data_cb_to_app(const uint8_t *data, uint32_t len)
{
    // todo: critical section protection
    if (hf_client_local_param.btc_hf_client_incoming_data_cb) {
        hf_client_local_param.btc_hf_client_incoming_data_cb(data, len);
    }
}

uint32_t btc_hf_client_outgoing_data_cb_to_app(uint8_t *data, uint32_t len)
{
    // todo: critical section protection
    if (hf_client_local_param.btc_hf_client_outgoing_data_cb) {
        return hf_client_local_param.btc_hf_client_outgoing_data_cb(data, len);
    } else {
        return 0;
    }
}

/*****************************************************************************
**
**   btc hf api functions
**
*****************************************************************************/

/*******************************************************************************
**
** Function        btc_hf_client_init
**
** Description     initializes the hf interface
**
** Returns         bt_status_t
**
*******************************************************************************/
bt_status_t btc_hf_client_init(void)
{
    BTC_TRACE_EVENT("%s", __FUNCTION__);

    uint8_t data_path;
    btc_dm_enable_service(BTA_HFP_HS_SERVICE_ID);

    clear_state();

    hf_client_local_param.btc_hf_client_cb.initialized = true;

#if BTM_SCO_HCI_INCLUDED
    data_path = ESP_SCO_DATA_PATH_HCI;
#else
    data_path = ESP_SCO_DATA_PATH_PCM;
#endif
    esp_bredr_sco_datapath_set(data_path);
    return BT_STATUS_SUCCESS;
}


/*******************************************************************************
**
** Function        btc_hf_client_connect
**
** Description     connect to audio gateway
**
** Returns         bt_status_t
**
*******************************************************************************/
static bt_status_t connect_int( bt_bdaddr_t *bd_addr, uint16_t uuid )
{
    if (is_connected(bd_addr)) {
        return BT_STATUS_BUSY;
    }

    hf_client_local_param.btc_hf_client_cb.state = ESP_HF_CLIENT_CONNECTION_STATE_CONNECTING;
    bdcpy(hf_client_local_param.btc_hf_client_cb.connected_bda.address, bd_addr->address);

    BTA_HfClientOpen(hf_client_local_param.btc_hf_client_cb.handle, hf_client_local_param.btc_hf_client_cb.connected_bda.address,
               BTC_HF_CLIENT_SECURITY);

    return BT_STATUS_SUCCESS;
}


bt_status_t btc_hf_client_connect( bt_bdaddr_t *bd_addr )
{
    BTC_TRACE_EVENT("HFP Client version is  0x%04x", btc_hf_client_version);
    CHECK_HF_CLIENT_INIT();
    return btc_queue_connect(UUID_SERVCLASS_HF_HANDSFREE, bd_addr, connect_int);
}

/*******************************************************************************
**
** Function         btc_hf_client_deinit
**
** Description      Closes the HF interface
**
** Returns          bt_status_t
**
*******************************************************************************/
void  btc_hf_client_deinit( void )
{
    BTC_TRACE_EVENT("%s", __FUNCTION__);

    btc_dm_disable_service(BTA_HFP_HS_SERVICE_ID);

    hf_client_local_param.btc_hf_client_cb.initialized = false;
}

/*******************************************************************************
**
** Function         btc_hf_client_disconnect
**
** Description      disconnect from audio gateway
**
** Returns         bt_status_t
**
*******************************************************************************/
bt_status_t btc_hf_client_disconnect( bt_bdaddr_t *bd_addr )
{
    CHECK_HF_CLIENT_INIT();

    if (is_connected(bd_addr))
    {
        BTA_HfClientClose(hf_client_local_param.btc_hf_client_cb.handle);
        return BT_STATUS_SUCCESS;
    }

    return BT_STATUS_FAIL;
}

/*******************************************************************************
**
** Function        btc_hf_client_connect_audio
**
** Description     create an audio connection
**
** Returns         bt_status_t
**
*******************************************************************************/
bt_status_t btc_hf_client_connect_audio( bt_bdaddr_t *bd_addr )
{
    CHECK_HF_CLIENT_SLC_CONNECTED();

    if (is_connected(bd_addr))
    {
        if (hf_client_local_param.btc_hf_client_cb.peer_feat & BTA_HF_CLIENT_PEER_CODEC)
        {
            BTA_HfClientSendAT(hf_client_local_param.btc_hf_client_cb.handle, BTA_HF_CLIENT_AT_CMD_BCC, 0, 0, NULL);
        }
        else
        {
            BTA_HfClientAudioOpen(hf_client_local_param.btc_hf_client_cb.handle);
        }

        /* Inform the application that the audio connection has been initiated successfully */
        do {
            esp_hf_client_cb_param_t param;
            memset(&param, 0, sizeof(esp_hf_client_cb_param_t));
            param.audio_stat.state = ESP_HF_CLIENT_AUDIO_STATE_CONNECTING;
            memcpy(param.audio_stat.remote_bda, &hf_client_local_param.btc_hf_client_cb.connected_bda, sizeof(esp_bd_addr_t));
            btc_hf_client_cb_to_app(ESP_HF_CLIENT_AUDIO_STATE_EVT, &param);
        } while (0);

        return BT_STATUS_SUCCESS;
    }

    return BT_STATUS_FAIL;
}

/*******************************************************************************
**
** Function         btc_hf_client_disconnect_audio
**
** Description      close the audio connection
**
** Returns         bt_status_t
**
*******************************************************************************/
bt_status_t btc_hf_client_disconnect_audio( bt_bdaddr_t *bd_addr )
{
    CHECK_HF_CLIENT_SLC_CONNECTED();

    if (is_connected(bd_addr))
    {
        BTA_HfClientAudioClose(hf_client_local_param.btc_hf_client_cb.handle);
        return BT_STATUS_SUCCESS;
    }

    return BT_STATUS_FAIL;
}

/*******************************************************************************
**
** Function         btc_hf_client_start_voice_recognition
**
** Description      start voice recognition
**
** Returns          bt_status_t
**
*******************************************************************************/
static bt_status_t btc_hf_client_start_voice_recognition(void)
{
    CHECK_HF_CLIENT_SLC_CONNECTED();

    if (hf_client_local_param.btc_hf_client_cb.peer_feat & BTA_HF_CLIENT_PEER_FEAT_VREC)
    {
        BTA_HfClientSendAT(hf_client_local_param.btc_hf_client_cb.handle, BTA_HF_CLIENT_AT_CMD_BVRA, 1, 0, NULL);

        return BT_STATUS_SUCCESS;
    }

    return BT_STATUS_UNSUPPORTED;
}


/*******************************************************************************
**
** Function         btc_hf_client_stop_voice_recognition
**
** Description      stop voice recognition
**
** Returns          bt_status_t
**
*******************************************************************************/
static bt_status_t btc_hf_client_stop_voice_recognition(void)
{
    CHECK_HF_CLIENT_SLC_CONNECTED();

    if (hf_client_local_param.btc_hf_client_cb.peer_feat & BTA_HF_CLIENT_PEER_FEAT_VREC)
    {
        BTA_HfClientSendAT(hf_client_local_param.btc_hf_client_cb.handle, BTA_HF_CLIENT_AT_CMD_BVRA, 0, 0, NULL);

        return BT_STATUS_SUCCESS;
    }

    return BT_STATUS_UNSUPPORTED;
}

/*******************************************************************************
**
** Function         btc_hf_client_volume_update
**
** Description      volume update
**
** Returns          bt_status_t
**
*******************************************************************************/
static bt_status_t btc_hf_client_volume_update(esp_hf_volume_control_target_t type, int volume)
{
    CHECK_HF_CLIENT_SLC_CONNECTED();

    switch (type)
    {
        case ESP_HF_VOLUME_CONTROL_TARGET_SPK:
            BTA_HfClientSendAT(hf_client_local_param.btc_hf_client_cb.handle, BTA_HF_CLIENT_AT_CMD_VGS, volume, 0, NULL);
            break;
        case ESP_HF_VOLUME_CONTROL_TARGET_MIC:
            BTA_HfClientSendAT(hf_client_local_param.btc_hf_client_cb.handle, BTA_HF_CLIENT_AT_CMD_VGM, volume, 0, NULL);
            break;
        default:
            return BT_STATUS_UNSUPPORTED;
    }

    return BT_STATUS_SUCCESS;
}

/*******************************************************************************
**
** Function         btc_hf_client_dial
**
** Description      place a call
**
** Returns          bt_status_t
**
*******************************************************************************/
static bt_status_t btc_hf_client_dial(const char *number)
{
    CHECK_HF_CLIENT_SLC_CONNECTED();

    if (strlen(number) != 0)
    {
        BTA_HfClientSendAT(hf_client_local_param.btc_hf_client_cb.handle, BTA_HF_CLIENT_AT_CMD_ATD, 0, 0, number);
    }
    else
    {
        BTA_HfClientSendAT(hf_client_local_param.btc_hf_client_cb.handle, BTA_HF_CLIENT_AT_CMD_BLDN, 0, 0, NULL);
    }

    return BT_STATUS_SUCCESS;
}

/*******************************************************************************
**
** Function         dial_memory
**
** Description      place a call with number specified by location (speed dial)
**
** Returns          bt_status_t
**
*******************************************************************************/
static bt_status_t btc_hf_client_dial_memory(int location)
{
    CHECK_HF_CLIENT_SLC_CONNECTED();

    BTA_HfClientSendAT(hf_client_local_param.btc_hf_client_cb.handle, BTA_HF_CLIENT_AT_CMD_ATD, location, 0, NULL);

    return BT_STATUS_SUCCESS;
}

static bt_status_t btc_hf_client_send_chld_cmd(esp_hf_chld_type_t type, int idx)
{
    CHECK_HF_CLIENT_SLC_CONNECTED();

    switch (type)
    {
    case ESP_HF_CHLD_TYPE_REL:
        if (hf_client_local_param.btc_hf_client_cb.chld_feat & BTA_HF_CLIENT_CHLD_REL)
        {
            BTA_HfClientSendAT(hf_client_local_param.btc_hf_client_cb.handle, BTA_HF_CLIENT_AT_CMD_CHLD, 0, 0, NULL);
            break;
        }
        return BT_STATUS_UNSUPPORTED;
    case ESP_HF_CHLD_TYPE_REL_ACC:
        // CHLD 1 is mandatory for 3 way calling
        if (hf_client_local_param.btc_hf_client_cb.peer_feat & BTA_HF_CLIENT_PEER_FEAT_3WAY)
        {
            BTA_HfClientSendAT(hf_client_local_param.btc_hf_client_cb.handle, BTA_HF_CLIENT_AT_CMD_CHLD, 1, 0, NULL);
            break;
        }
        return BT_STATUS_UNSUPPORTED;
    case ESP_HF_CHLD_TYPE_HOLD_ACC:
        // CHLD 2 is mandatory for 3 way calling
        if (hf_client_local_param.btc_hf_client_cb.peer_feat & BTA_HF_CLIENT_PEER_FEAT_3WAY)
        {
            BTA_HfClientSendAT(hf_client_local_param.btc_hf_client_cb.handle, BTA_HF_CLIENT_AT_CMD_CHLD, 2, 0, NULL);
            break;
        }
        return BT_STATUS_UNSUPPORTED;
    case ESP_HF_CHLD_TYPE_MERGE:
        if (hf_client_local_param.btc_hf_client_cb.chld_feat & BTA_HF_CLIENT_CHLD_MERGE)
        {
            BTA_HfClientSendAT(hf_client_local_param.btc_hf_client_cb.handle, BTA_HF_CLIENT_AT_CMD_CHLD, 3, 0, NULL);
            break;
        }
        return BT_STATUS_UNSUPPORTED;
    case ESP_HF_CHLD_TYPE_MERGE_DETACH:
        if (hf_client_local_param.btc_hf_client_cb.chld_feat & BTA_HF_CLIENT_CHLD_MERGE_DETACH)
        {
            BTA_HfClientSendAT(hf_client_local_param.btc_hf_client_cb.handle, BTA_HF_CLIENT_AT_CMD_CHLD, 4, 0, NULL);
            break;
        }
        return BT_STATUS_UNSUPPORTED;
    case ESP_HF_CHLD_TYPE_REL_X:
        if (hf_client_local_param.btc_hf_client_cb.peer_feat & BTA_HF_CLIENT_PEER_ECC)
        {
            if (idx < 1)
            {
                return BT_STATUS_FAIL;
            }
            BTA_HfClientSendAT(hf_client_local_param.btc_hf_client_cb.handle, BTA_HF_CLIENT_AT_CMD_CHLD, 1, idx, NULL);
            break;
        }
        return BT_STATUS_UNSUPPORTED;
    case ESP_HF_CHLD_TYPE_PRIV_X:
        if (hf_client_local_param.btc_hf_client_cb.peer_feat & BTA_HF_CLIENT_PEER_ECC)
        {
            if (idx < 1)
            {
                return BT_STATUS_FAIL;
            }
            BTA_HfClientSendAT(hf_client_local_param.btc_hf_client_cb.handle, BTA_HF_CLIENT_AT_CMD_CHLD, 2, idx, NULL);
            break;
        }
        return BT_STATUS_UNSUPPORTED;

    }
    return BT_STATUS_SUCCESS;
}

static bt_status_t btc_hf_client_send_btrh_cmd(esp_hf_btrh_cmd_t btrh)
{
    CHECK_HF_CLIENT_SLC_CONNECTED();

    switch (btrh) {
    case ESP_HF_BTRH_CMD_HOLD:
        BTA_HfClientSendAT(hf_client_local_param.btc_hf_client_cb.handle, BTA_HF_CLIENT_AT_CMD_BTRH, 0, 0, NULL);
        break;
    case ESP_HF_BTRH_CMD_ACCEPT:
        BTA_HfClientSendAT(hf_client_local_param.btc_hf_client_cb.handle, BTA_HF_CLIENT_AT_CMD_BTRH, 1, 0, NULL);
        break;
    case ESP_HF_BTRH_CMD_REJECT:
        BTA_HfClientSendAT(hf_client_local_param.btc_hf_client_cb.handle, BTA_HF_CLIENT_AT_CMD_BTRH, 2, 0, NULL);
        break;
    default:
        return BT_STATUS_FAIL;
    }

    return BT_STATUS_SUCCESS;
}

static bt_status_t btc_hf_client_answer_call(void)
{
    CHECK_HF_CLIENT_SLC_CONNECTED();
    BTA_HfClientSendAT(hf_client_local_param.btc_hf_client_cb.handle, BTA_HF_CLIENT_AT_CMD_ATA, 0, 0, NULL);
    return BT_STATUS_SUCCESS;
}

static bt_status_t btc_hf_client_reject_call(void)
{
    CHECK_HF_CLIENT_SLC_CONNECTED();
    BTA_HfClientSendAT(hf_client_local_param.btc_hf_client_cb.handle, BTA_HF_CLIENT_AT_CMD_CHUP, 0, 0, NULL);
    return BT_STATUS_SUCCESS;
}

/*******************************************************************************
**
** Function         btc_hf_client_query_current_calls
**
** Description      query list of current calls
**
** Returns          bt_status_t
**
*******************************************************************************/
static bt_status_t btc_hf_client_query_current_calls(void)
{
    CHECK_HF_CLIENT_SLC_CONNECTED();

    if (hf_client_local_param.btc_hf_client_cb.peer_feat & BTA_HF_CLIENT_PEER_ECS)
    {
        BTA_HfClientSendAT(hf_client_local_param.btc_hf_client_cb.handle, BTA_HF_CLIENT_AT_CMD_CLCC, 0, 0, NULL);

        return BT_STATUS_SUCCESS;
    }

    return BT_STATUS_UNSUPPORTED;
}

/*******************************************************************************
**
** Function         btc_hf_client_query_current_operator_name
**
** Description      query current selected operator name
**
** Returns          bt_status_t
**
*******************************************************************************/
static bt_status_t btc_hf_client_query_current_operator_name(void)
{
    CHECK_HF_CLIENT_SLC_CONNECTED();

    BTA_HfClientSendAT(hf_client_local_param.btc_hf_client_cb.handle, BTA_HF_CLIENT_AT_CMD_COPS, 0, 0, NULL);

    return BT_STATUS_SUCCESS;
}

/*******************************************************************************
**
** Function         btc_hf_client_retieve_subscriber_info
**
** Description      retrieve subscriber number information
**
** Returns          bt_status_t
**
*******************************************************************************/
static bt_status_t btc_hf_client_retrieve_subscriber_info(void)
{
    CHECK_HF_CLIENT_SLC_CONNECTED();

    BTA_HfClientSendAT(hf_client_local_param.btc_hf_client_cb.handle, BTA_HF_CLIENT_AT_CMD_CNUM, 0, 0, NULL);

    return BT_STATUS_SUCCESS;
}

/*******************************************************************************
**
** Function         btc_hf_client_send_dtmf
**
** Description      send dtmf
**
** Returns          bt_status_t
**
*******************************************************************************/
static bt_status_t btc_hf_client_send_dtmf(char code)
{
    CHECK_HF_CLIENT_SLC_CONNECTED();

    BTA_HfClientSendAT(hf_client_local_param.btc_hf_client_cb.handle, BTA_HF_CLIENT_AT_CMD_VTS, code, 0, NULL);

    return BT_STATUS_SUCCESS;
}

/*******************************************************************************
**
** Function         btc_hf_client_send_xapl
**
** Description      send xapl
**
** Returns          bt_status_t
**
*******************************************************************************/
static bt_status_t btc_hf_client_send_xapl(char *buf, UINT32 features)
{
    CHECK_HF_CLIENT_SLC_CONNECTED();

    BTA_HfClientSendAT(hf_client_local_param.btc_hf_client_cb.handle, BTA_HF_CLIENT_AT_CMD_XAPL, features, 0, buf);

    return BT_STATUS_SUCCESS;
}

/*******************************************************************************
**
** Function         btc_hf_client_send_iphoneaccev
**
** Description      send IPHONEACCEV
**
** Returns          bt_status_t
**
*******************************************************************************/
static bt_status_t btc_hf_client_send_iphoneaccev(uint32_t bat_level, BOOLEAN docked)
{
    CHECK_HF_CLIENT_SLC_CONNECTED();

    BTA_HfClientSendAT(hf_client_local_param.btc_hf_client_cb.handle, BTA_HF_CLIENT_AT_CMD_IPHONEACCEV, bat_level, (UINT32)docked, NULL);

    return BT_STATUS_SUCCESS;
}
/*******************************************************************************
**
** Function         btc_hf_client_request_last_voice_tag_number
**
** Description      Request number from AG for VR purposes
**
** Returns          bt_status_t
**
*******************************************************************************/
static bt_status_t btc_hf_client_request_last_voice_tag_number(void)
{
    CHECK_HF_CLIENT_SLC_CONNECTED();

    if (hf_client_local_param.btc_hf_client_cb.peer_feat & BTA_HF_CLIENT_PEER_VTAG)
    {
        BTA_HfClientSendAT(hf_client_local_param.btc_hf_client_cb.handle, BTA_HF_CLIENT_AT_CMD_BINP, 1, 0, NULL);

        return BT_STATUS_SUCCESS;
    }

    return BT_STATUS_UNSUPPORTED;
}

/*******************************************************************************
**
** Function         btc_hf_client_send_nrec
**
** Description      Request AG to disable echo cancellation & noise reduction
**
** Returns          bt_status_t
**
*******************************************************************************/
static bt_status_t btc_hf_client_send_nrec(void)
{
    CHECK_HF_CLIENT_SLC_CONNECTED();

    if (hf_client_local_param.btc_hf_client_cb.peer_feat & BTA_HF_CLIENT_PEER_FEAT_ECNR)
    {
        BTA_HfClientSendAT(hf_client_local_param.btc_hf_client_cb.handle, BTA_HF_CLIENT_AT_CMD_NREC, 0, 0, NULL);
        return BT_STATUS_SUCCESS;
    }
    return BT_STATUS_UNSUPPORTED;
}

/*******************************************************************************
**
** Function         btc_hf_client_pkt_stat_nums_get;
**
** Description      Request packet status numbers for a specific (e)SCO connection handle
**
** Returns          bt_status_t
**
*******************************************************************************/
static bt_status_t btc_hf_client_pkt_stat_nums_get(UINT16 sync_conn_handle)
{
    CHECK_HF_CLIENT_SLC_CONNECTED();
#if (BTM_SCO_HCI_INCLUDED == TRUE)
    BTA_HfClientPktStatsNumsGet(sync_conn_handle);
#endif /*#if (BTM_SCO_HCI_INCLUDED == TRUE) */
    return BT_STATUS_SUCCESS;
}

/*******************************************************************************
**
** Function         bte_hf_client_evt
**
** Description      Switches context from BTE to BTIF for all HF Client events
**
** Returns          void
**
*******************************************************************************/
static void bte_hf_client_evt(tBTA_HF_CLIENT_EVT event, void *p_data)
{
    bt_status_t stat;
    btc_msg_t msg;
    int arg_len = BTA_HfClientGetCbDataSize(event);
    void *arg = (p_data != NULL && arg_len > 0) ? p_data : NULL;

    msg.sig = BTC_SIG_API_CB;
    msg.pid = BTC_PID_HF_CLIENT;
    msg.act = (uint8_t) event;

    stat = btc_transfer_context(&msg, arg, arg_len, NULL, NULL);

    if (stat) {
        BTC_TRACE_ERROR("%s transfer failed\n", __func__);
    }
}

/*******************************************************************************
**
** Function         btc_hf_client_execute_service
**
** Description      Initializes/Shuts down the service
**
** Returns          BT_STATUS_SUCCESS on success, BT_STATUS_FAIL otherwise
**
*******************************************************************************/
bt_status_t btc_hf_client_execute_service(BOOLEAN b_enable)
{
    BTC_TRACE_EVENT("%s enable:%d", __FUNCTION__, b_enable);

    if (b_enable)
    {
        /* Enable and register with BTA-HFClient */
        BTA_HfClientEnable(bte_hf_client_evt);
        hf_client_local_param.btc_hf_client_features = BTC_HF_CLIENT_FEATURES;
        if (btc_hf_client_version >= HFP_HF_VERSION_1_7)
        {
            hf_client_local_param.btc_hf_client_features |= BTA_HF_CLIENT_FEAT_ESCO_S4;
            BTC_TRACE_EVENT("eSCO S4 Setting Supported");

        }
        else if (btc_hf_client_version >= HFP_HF_VERSION_1_6)
        {
            BTC_TRACE_EVENT("No eSCO S4 Setting Supported");
        }
        else
        {
            BTC_TRACE_EVENT("No Codec Nego Supported");
            hf_client_local_param.btc_hf_client_features = hf_client_local_param.btc_hf_client_features & (~BTA_HF_CLIENT_FEAT_CODEC);
        }
        BTC_TRACE_EVENT("hf_client_local_param.btc_hf_client_features is   %d", hf_client_local_param.btc_hf_client_features);
        BTA_HfClientRegister(BTC_HF_CLIENT_SECURITY, hf_client_local_param.btc_hf_client_features,
                    BTC_HF_CLIENT_SERVICE_NAME);
    }
    else
    {
        BTA_HfClientDeregister(hf_client_local_param.btc_hf_client_cb.handle);
        BTA_HfClientDisable();
    }
    return BT_STATUS_SUCCESS;
}

static void process_ind_evt(tBTA_HF_CLIENT_IND *ind)
{
    esp_hf_client_cb_param_t param;
    memset(&param, 0, sizeof(esp_hf_client_cb_param_t));

    switch (ind->type)
    {
        case BTA_HF_CLIENT_IND_CALL:
            param.call.status = ind->value;
            btc_hf_client_cb_to_app(ESP_HF_CLIENT_CIND_CALL_EVT, &param);
            break;

        case BTA_HF_CLIENT_IND_CALLSETUP:
            param.call_setup.status = ind->value;
            btc_hf_client_cb_to_app(ESP_HF_CLIENT_CIND_CALL_SETUP_EVT, &param);
            break;
        case BTA_HF_CLIENT_IND_CALLHELD:
            param.call_held.status = ind->value;
            btc_hf_client_cb_to_app(ESP_HF_CLIENT_CIND_CALL_HELD_EVT, &param);
            break;

        case BTA_HF_CLIENT_IND_SERVICE:
            param.service_availability.status = ind->value;
            btc_hf_client_cb_to_app(ESP_HF_CLIENT_CIND_SERVICE_AVAILABILITY_EVT, &param);
            break;

        case BTA_HF_CLIENT_IND_SIGNAL:
            param.signal_strength.value = ind->value;
            btc_hf_client_cb_to_app(ESP_HF_CLIENT_CIND_SIGNAL_STRENGTH_EVT, &param);
            break;

        case BTA_HF_CLIENT_IND_ROAM:
            param.roaming.status = ind->value;
            btc_hf_client_cb_to_app(ESP_HF_CLIENT_CIND_ROAMING_STATUS_EVT, &param);
            break;

        case BTA_HF_CLIENT_IND_BATTCH:
            param.battery_level.value = ind->value;
            btc_hf_client_cb_to_app(ESP_HF_CLIENT_CIND_BATTERY_LEVEL_EVT, &param);
            break;

        default:
            break;
    }
}

void btc_hf_client_cb_handler(btc_msg_t *msg)
{
    uint16_t event = msg->act;
    tBTA_HF_CLIENT *p_data = (tBTA_HF_CLIENT *)msg->arg;
    esp_hf_client_cb_param_t param;
    bdstr_t bdstr;

    memset(&param, 0, sizeof(esp_hf_client_cb_param_t));

    switch (event)
    {
        case BTA_HF_CLIENT_ENABLE_EVT:
        case BTA_HF_CLIENT_DISABLE_EVT:
            break;
        case BTA_HF_CLIENT_REGISTER_EVT:
            hf_client_local_param.btc_hf_client_cb.handle = p_data->reg.handle;
            break;
        case BTA_HF_CLIENT_OPEN_EVT:
            if (p_data->open.status == BTA_HF_CLIENT_SUCCESS)
            {
                bdcpy(hf_client_local_param.btc_hf_client_cb.connected_bda.address, p_data->open.bd_addr);
                hf_client_local_param.btc_hf_client_cb.state = ESP_HF_CLIENT_CONNECTION_STATE_CONNECTED;
                hf_client_local_param.btc_hf_client_cb.peer_feat = 0;
                hf_client_local_param.btc_hf_client_cb.chld_feat = 0;
                //clear_phone_state();
            }
            else if (hf_client_local_param.btc_hf_client_cb.state == ESP_HF_CLIENT_CONNECTION_STATE_CONNECTING)
            {
                hf_client_local_param.btc_hf_client_cb.state = ESP_HF_CLIENT_CONNECTION_STATE_DISCONNECTED;
            }
            else
            {
                BTC_TRACE_WARNING("%s: HF CLient open failed, but another device connected. status=%d state=%d connected device=%s",
                        __FUNCTION__, p_data->open.status, hf_client_local_param.btc_hf_client_cb.state, bdaddr_to_string(&hf_client_local_param.btc_hf_client_cb.connected_bda, bdstr, sizeof(bdstr)));
                UNUSED(bdstr);
                break;
            }

            do {
                param.conn_stat.state = hf_client_local_param.btc_hf_client_cb.state;
                param.conn_stat.peer_feat = 0;
                param.conn_stat.chld_feat = 0;

                memcpy(param.conn_stat.remote_bda, &hf_client_local_param.btc_hf_client_cb.connected_bda,
                       sizeof(esp_bd_addr_t));

                btc_hf_client_cb_to_app(ESP_HF_CLIENT_CONNECTION_STATE_EVT, &param);
            } while (0);

            if (hf_client_local_param.btc_hf_client_cb.state == ESP_HF_CLIENT_CONNECTION_STATE_DISCONNECTED)
                bdsetany(hf_client_local_param.btc_hf_client_cb.connected_bda.address);

            if (p_data->open.status != BTA_HF_CLIENT_SUCCESS) {
                btc_queue_advance();
            }

            break;

        case BTA_HF_CLIENT_CONN_EVT:
            hf_client_local_param.btc_hf_client_cb.peer_feat = p_data->conn.peer_feat;
            hf_client_local_param.btc_hf_client_cb.chld_feat = p_data->conn.chld_feat;
            hf_client_local_param.btc_hf_client_cb.state = ESP_HF_CLIENT_CONNECTION_STATE_SLC_CONNECTED;

            do {
                param.conn_stat.state = hf_client_local_param.btc_hf_client_cb.state;
                param.conn_stat.peer_feat = hf_client_local_param.btc_hf_client_cb.peer_feat;
                param.conn_stat.chld_feat = hf_client_local_param.btc_hf_client_cb.chld_feat;

                memcpy(param.conn_stat.remote_bda, &hf_client_local_param.btc_hf_client_cb.connected_bda,
                       sizeof(esp_bd_addr_t));

                btc_hf_client_cb_to_app(ESP_HF_CLIENT_CONNECTION_STATE_EVT, &param);
            } while (0);

            /* Inform the application about in-band ringtone */
            if (hf_client_local_param.btc_hf_client_cb.peer_feat & BTA_HF_CLIENT_PEER_INBAND)
            {
                do {
                    param.bsir.state =  ESP_HF_CLIENT_IN_BAND_RINGTONE_PROVIDED;
                    btc_hf_client_cb_to_app(ESP_HF_CLIENT_BSIR_EVT, &param);
                } while (0);
            }

            btc_queue_advance();
            break;

        case BTA_HF_CLIENT_CLOSE_EVT:
            hf_client_local_param.btc_hf_client_cb.state = ESP_HF_CLIENT_CONNECTION_STATE_DISCONNECTED;
            do {
                param.conn_stat.state = ESP_HF_CLIENT_CONNECTION_STATE_DISCONNECTED;
                param.conn_stat.peer_feat = 0;
                param.conn_stat.chld_feat = 0;

                memcpy(param.conn_stat.remote_bda, &hf_client_local_param.btc_hf_client_cb.connected_bda,
                       sizeof(esp_bd_addr_t));

                btc_hf_client_cb_to_app(ESP_HF_CLIENT_CONNECTION_STATE_EVT, &param);
            } while (0);

            bdsetany(hf_client_local_param.btc_hf_client_cb.connected_bda.address);
            hf_client_local_param.btc_hf_client_cb.peer_feat = 0;
            hf_client_local_param.btc_hf_client_cb.chld_feat = 0;
            btc_queue_advance();
            break;
        case BTA_HF_CLIENT_IND_EVT:
            process_ind_evt(&p_data->ind);
            break;
        case BTA_HF_CLIENT_MIC_EVT:
            do {
                param.volume_control.type = ESP_HF_VOLUME_CONTROL_TARGET_MIC;
                param.volume_control.volume = p_data->val.value;

                btc_hf_client_cb_to_app(ESP_HF_CLIENT_VOLUME_CONTROL_EVT, &param);
            } while (0);
            break;
        case BTA_HF_CLIENT_SPK_EVT:
            do {
                param.volume_control.type = ESP_HF_VOLUME_CONTROL_TARGET_SPK;
                param.volume_control.volume = p_data->val.value;

                btc_hf_client_cb_to_app(ESP_HF_CLIENT_VOLUME_CONTROL_EVT, &param);
            } while (0);
            break;
        case BTA_HF_CLIENT_VOICE_REC_EVT:
            do {
                param.bvra.value = p_data->val.value;

                btc_hf_client_cb_to_app(ESP_HF_CLIENT_BVRA_EVT, &param);
            } while (0);
            break;
        case BTA_HF_CLIENT_OPERATOR_NAME_EVT:
            do {
                param.cops.name = p_data->operator.name;
                btc_hf_client_cb_to_app(ESP_HF_CLIENT_COPS_CURRENT_OPERATOR_EVT, &param);
            } while (0);
            break;
        case BTA_HF_CLIENT_CLIP_EVT:
            do {
                param.clip.number = p_data->number.number;
                btc_hf_client_cb_to_app(ESP_HF_CLIENT_CLIP_EVT, &param);
            } while (0);
            break;
        case BTA_HF_CLIENT_BINP_EVT:
            do {
                param.binp.number = p_data->number.number;
                btc_hf_client_cb_to_app(ESP_HF_CLIENT_BINP_EVT, &param);
            } while (0);
            break;
        case BTA_HF_CLIENT_CCWA_EVT:
            do {
                param.ccwa.number = p_data->number.number;
                btc_hf_client_cb_to_app(ESP_HF_CLIENT_CCWA_EVT, &param);
            } while (0);
            break;
        case BTA_HF_CLIENT_AT_RESULT_EVT:
            do {
                param.at_response.code = p_data->result.type;
                param.at_response.cme = p_data->result.cme;

                btc_hf_client_cb_to_app(ESP_HF_CLIENT_AT_RESPONSE_EVT, &param);
            } while (0);
            break;
        case BTA_HF_CLIENT_CLCC_EVT:
            do {
                param.clcc.idx = p_data->clcc.idx;
                param.clcc.dir = p_data->clcc.inc ? ESP_HF_CURRENT_CALL_DIRECTION_INCOMING :
                    ESP_HF_CURRENT_CALL_DIRECTION_OUTGOING;
                param.clcc.status = p_data->clcc.status;
                param.clcc.mpty = p_data->clcc.mpty ? ESP_HF_CURRENT_CALL_MPTY_TYPE_MULTI :
                    ESP_HF_CURRENT_CALL_MPTY_TYPE_SINGLE;
                param.clcc.number = p_data->clcc.number_present ? p_data->clcc.number : NULL;
                btc_hf_client_cb_to_app(ESP_HF_CLIENT_CLCC_EVT, &param);
            } while (0);
            break;
        case BTA_HF_CLIENT_CNUM_EVT:
            do {
                param.cnum.number = p_data->cnum.number;
                if (p_data->cnum.service == 4) {
                    param.cnum.type = ESP_HF_SUBSCRIBER_SERVICE_TYPE_VOICE;
                } else if (p_data->cnum.service == 5) {
                    param.cnum.type = ESP_HF_SUBSCRIBER_SERVICE_TYPE_FAX;
                } else {
                    param.cnum.type = ESP_HF_SUBSCRIBER_SERVICE_TYPE_UNKNOWN;
                }
                btc_hf_client_cb_to_app(ESP_HF_CLIENT_CNUM_EVT, &param);
                break;
            } while (0);
            break;
        case BTA_HF_CLIENT_BTRH_EVT:
            if (p_data->val.value <= ESP_HF_BTRH_STATUS_REJECTED) {
                do {
                    param.btrh.status = p_data->val.value;

                    btc_hf_client_cb_to_app(ESP_HF_CLIENT_BTRH_EVT, &param);
                } while (0);
            }
            break;
        case BTA_HF_CLIENT_BSIR_EVT:
            if (p_data->val.value != 0)
            {
                param.bsir.state = ESP_HF_CLIENT_IN_BAND_RINGTONE_PROVIDED;
            }
            else
            {
                param.bsir.state = ESP_HF_CLIENT_IN_BAND_RINGTONE_NOT_PROVIDED;
            }
            btc_hf_client_cb_to_app(ESP_HF_CLIENT_BSIR_EVT, &param);
            break;
        case BTA_HF_CLIENT_AUDIO_OPEN_EVT:
            do {
                param.audio_stat.state = ESP_HF_CLIENT_AUDIO_STATE_CONNECTED;
                memcpy(param.audio_stat.remote_bda, &hf_client_local_param.btc_hf_client_cb.connected_bda,
                       sizeof(esp_bd_addr_t));
                param.audio_stat.sync_conn_handle = p_data->hdr.sync_conn_handle;
                btc_hf_client_cb_to_app(ESP_HF_CLIENT_AUDIO_STATE_EVT, &param);
            } while (0);
            break;
        case BTA_HF_CLIENT_AUDIO_MSBC_OPEN_EVT:
            do {
                param.audio_stat.state = ESP_HF_CLIENT_AUDIO_STATE_CONNECTED_MSBC;
                memcpy(param.audio_stat.remote_bda, &hf_client_local_param.btc_hf_client_cb.connected_bda,
                       sizeof(esp_bd_addr_t));
                param.audio_stat.sync_conn_handle = p_data->hdr.sync_conn_handle;
                btc_hf_client_cb_to_app(ESP_HF_CLIENT_AUDIO_STATE_EVT, &param);
            } while (0);
            break;
        case BTA_HF_CLIENT_AUDIO_CLOSE_EVT:
            do {
                param.audio_stat.state = ESP_HF_CLIENT_AUDIO_STATE_DISCONNECTED;
                memcpy(param.audio_stat.remote_bda, &hf_client_local_param.btc_hf_client_cb.connected_bda,
                       sizeof(esp_bd_addr_t));
                param.audio_stat.sync_conn_handle = p_data->hdr.sync_conn_handle;
                btc_hf_client_cb_to_app(ESP_HF_CLIENT_AUDIO_STATE_EVT, &param);
            } while (0);
            break;
        case BTA_HF_CLIENT_RING_INDICATION:
            do {
                btc_hf_client_cb_to_app(ESP_HF_CLIENT_RING_IND_EVT, NULL);
            } while (0);
            break;
        case BTA_HF_CLIENT_PKT_STAT_NUMS_GET_EVT:
        {
            memcpy(&param.pkt_nums, &p_data->pkt_num, sizeof(struct hf_client_pkt_status_nums));
            btc_hf_client_cb_to_app(ESP_HF_CLIENT_PKT_STAT_NUMS_GET_EVT, &param);
            break;
        }
        default:
            BTC_TRACE_WARNING("%s: Unhandled event: %d", __FUNCTION__, event);
            break;
    }
}

void btc_hf_client_call_handler(btc_msg_t *msg)
{
    btc_hf_client_args_t *arg = (btc_hf_client_args_t *)(msg->arg);
    switch (msg->act) {

    case BTC_HF_CLIENT_INIT_EVT:
        btc_hf_client_init();
        bt_status_t result = btc_hf_client_connect(&arg->connect);
        if (result == BT_STATUS_SUCCESS){
            btc_hf_client_connect_audio(&arg->connect_audio);   //PHIL added so connect() and connect_audio() always gets called after init() 
        } else {
            ESP_LOGI(TAG, "BTC_HF_CLIENT_INIT_EVT failed");
        }
        break;
    case BTC_HF_CLIENT_DEINIT_EVT:
        btc_hf_client_deinit();
        break;
    case BTC_HF_CLIENT_CONNECT_EVT:
        bt_status_t result = btc_hf_client_connect(&arg->connect);
        if (result == BT_STATUS_SUCCESS){
            btc_hf_client_connect_audio(&arg->connect_audio);   //PHIL added so connect() and connect_audio() always gets called after init() 
        } else {
            ESP_LOGI(TAG, "BTC_HF_CLIENT_CONNECT_EVT failed");
        }
        break;
    case BTC_HF_CLIENT_DISCONNECT_EVT:
        btc_hf_client_disconnect(&arg->disconnect);
        break;
    case BTC_HF_CLIENT_CONNECT_AUDIO_EVT:
        btc_hf_client_connect_audio(&arg->connect_audio);
        break;
    case BTC_HF_CLIENT_DISCONNECT_AUDIO_EVT:
        btc_hf_client_disconnect_audio(&arg->disconnect_audio);
        break;
    case BTC_HF_CLIENT_START_VOICE_RECOGNITION_EVT:
        btc_hf_client_start_voice_recognition();
        break;
    case BTC_HF_CLIENT_STOP_VOICE_RECOGNITION_EVT:
        btc_hf_client_stop_voice_recognition();
        break;
    case BTC_HF_CLIENT_VOLUME_UPDATE_EVT:
        btc_hf_client_volume_update(arg->volume_update.type, arg->volume_update.volume);
        break;
    case BTC_HF_CLIENT_DIAL_EVT:
        btc_hf_client_dial(arg->dial.number);
        break;
    case BTC_HF_CLIENT_DIAL_MEMORY_EVT:
        btc_hf_client_dial_memory(arg->dial_memory.location);
        break;
    case BTC_HF_CLIENT_SEND_CHLD_CMD_EVT:
        btc_hf_client_send_chld_cmd(arg->chld.type, arg->chld.idx);
        break;
    case BTC_HF_CLIENT_SEND_BTRH_CMD_EVT:
        btc_hf_client_send_btrh_cmd(arg->btrh.cmd);
        break;
    case BTC_HF_CLIENT_ANSWER_CALL_EVT:
        btc_hf_client_answer_call();
        break;
    case BTC_HF_CLIENT_REJECT_CALL_EVT:
        btc_hf_client_reject_call();
        break;
    case BTC_HF_CLIENT_QUERY_CURRENT_CALLS_EVT:
        btc_hf_client_query_current_calls();
        break;
    case BTC_HF_CLIENT_QUERY_CURRENT_OPERATOR_NAME_EVT:
        btc_hf_client_query_current_operator_name();
        break;
    case BTC_HF_CLIENT_RETRIEVE_SUBSCRIBER_INFO_EVT:
        btc_hf_client_retrieve_subscriber_info();
        break;
    case BTC_HF_CLIENT_SEND_DTMF_EVT:
        btc_hf_client_send_dtmf(arg->send_dtmf.code);
        break;
    case BTC_HF_CLIENT_REQUEST_LAST_VOICE_TAG_NUMBER_EVT:
        btc_hf_client_request_last_voice_tag_number();
        break;
    case BTC_HF_CLIENT_REGISTER_DATA_CALLBACK_EVT:
        btc_hf_client_reg_data_cb(arg->reg_data_cb.recv, arg->reg_data_cb.send);
        break;
    case BTC_HF_CLIENT_SEND_NREC_EVT:
        btc_hf_client_send_nrec();
        break;
    case BTC_HF_CLIENT_SEND_XAPL_EVT:
        btc_hf_client_send_xapl(arg->send_xapl.information, arg->send_xapl.features);
        break;
    case BTC_HF_CLIENT_SEND_IPHONEACCEV_EVT:
        btc_hf_client_send_iphoneaccev(arg->send_iphoneaccev.bat_level, arg->send_iphoneaccev.docked);
        break;
    case BTC_HF_CLIENT_REQUEST_PKT_STAT_EVT:
        btc_hf_client_pkt_stat_nums_get(arg->pkt_sync_hd.sync_conn_handle);
        break;
    default:
        BTC_TRACE_WARNING("%s : unhandled event: %d\n", __FUNCTION__, msg->act);
    }
}

#endif /* #if (BTC_HF_CLIENT_INCLUDED == TRUE) */
