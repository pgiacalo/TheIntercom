/*
 * SPDX-FileCopyrightText: 2021-2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */
/*
This file `bt_app_hf.c` seems to be a part of the Bluetooth Hands-Free Profile (HFP) implementation 
for the Espressif platform, specifically the ESP32 or similar Espressif chips. 
The code revolves around supporting hands-free capabilities, especially in scenarios like 
car kits or wireless headphones.

Here are the main points about the code:

1. Inclusion of Headers: 
    - The file includes various headers related to the ESP32 Bluetooth stack 
    (`esp_bt_main.h`, `esp_bt_device.h`, `esp_gap_bt_api.h`, `esp_hf_ag_api.h`, etc.), 
    FreeRTOS (`FreeRTOS.h`, `task.h`, `queue.h`, `semphr.h`, `ringbuf.h`) 
    and some standard C headers (`stdint.h`, `stdbool.h`, `stdlib.h`, `string.h`).

2. Global Variables and Enum Strings: 
    - It contains various string arrays for making log outputs more informative, 
    like `c_hf_evt_str`, `c_connection_state_str`, `c_audio_state_str`, etc.

3. Audio Data Handling (if using HCI):
    - If the configuration `CONFIG_BT_HFP_AUDIO_DATA_PATH_HCI` is defined, the file provides 
    a way to generate a sine wave audio signal and handle the incoming and outgoing audio data 
    through the HCI (Host Controller Interface).
    - The sine wave data is stored in `sine_int16` array, and there are functions to send audio data 
    (`bt_app_send_data`), handle the periodic sending of audio data (`bt_app_send_data_task`), 
    and shut down the send data task (`bt_app_send_data_shut_down`).

4. Bluetooth Event Callback: 
    - The main function of interest in the file is `bt_app_hf_cb`, which acts as a callback to 
    handle various Bluetooth Hands-Free events.
    - Each event, such as connection state changes, audio state changes, volume control, and more, 
    is handled within the switch-case construct of this function. Depending on the event, 
    appropriate actions are taken, and sometimes, responses are sent back.
    - For example, in the case of `ESP_HF_DIAL_EVT`, the code checks the type of dial request 
    (dialing by number, by memory, or the last number) and acts accordingly.
    - There's extensive logging throughout to ensure that the user (or developer) can 
    get insights into what's happening during the operation.

5. Miscellaneous:
    - There are function prototypes mentioned as "PHIL function declarations". 
    It seems like there's some customization or placeholders for additional functionalities.
    - The licensing information at the beginning indicates that the code can be used under 
    the "Unlicense" or the "CC0-1.0" license.

Overall, the code is structured to support Bluetooth hands-free operations and handle various events 
and scenarios related to Bluetooth HFP on an Espressif platform. If you're planning to integrate 
or modify this code, ensure that you understand each functionality, especially the audio handling 
part if you're using HCI for audio data path.
*/

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include "esp_log.h"
#include "esp_bt_main.h"
#include "esp_bt_device.h"
#include "esp_gap_bt_api.h"
#include "esp_hf_ag_api.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "freertos/ringbuf.h"
#include "time.h"
#include "sys/time.h"
#include "sdkconfig.h"
#include "bt_app_core.h"
#include "bt_app_hf.h"
#include "osi/allocator.h"

const char *c_hf_evt_str[] = {
    "CONNECTION_STATE_EVT",              /*!< SERVICE LEVEL CONNECTION STATE CONTROL */
    "AUDIO_STATE_EVT",                   /*!< AUDIO CONNECTION STATE CONTROL */
    "VR_STATE_CHANGE_EVT",               /*!< VOICE RECOGNITION CHANGE */
    "VOLUME_CONTROL_EVT",                /*!< AUDIO VOLUME CONTROL */
    "UNKNOW_AT_CMD",                     /*!< UNKNOW AT COMMAND RECIEVED */
    "IND_UPDATE",                        /*!< INDICATION UPDATE */
    "CIND_RESPONSE_EVT",                 /*!< CALL & DEVICE INDICATION */
    "COPS_RESPONSE_EVT",                 /*!< CURRENT OPERATOR EVENT */
    "CLCC_RESPONSE_EVT",                 /*!< LIST OF CURRENT CALL EVENT */
    "CNUM_RESPONSE_EVT",                 /*!< SUBSCRIBER INFORTMATION OF CALL EVENT */
    "DTMF_RESPONSE_EVT",                 /*!< DTMF TRANSFER EVT */
    "NREC_RESPONSE_EVT",                 /*!< NREC RESPONSE EVT */
    "ANSWER_INCOMING_EVT",               /*!< ANSWER INCOMING EVT */
    "REJECT_INCOMING_EVT",               /*!< AREJECT INCOMING EVT */
    "DIAL_EVT",                          /*!< DIAL INCOMING EVT */
    "WBS_EVT",                           /*!< CURRENT CODEC EVT */
    "BCS_EVT",                           /*!< CODEC NEGO EVT */
    "PKT_STAT_EVT",                      /*!< REQUEST PACKET STATUS EVT */
};

//----------
//PHIL function declarations
 // void bt_app_send_data(void);
//----------

//esp_hf_connection_state_t
const char *c_connection_state_str[] = {
    "DISCONNECTED",
    "CONNECTING",
    "CONNECTED",
    "SLC_CONNECTED",
    "DISCONNECTING",
};

// esp_hf_audio_state_t
const char *c_audio_state_str[] = {
    "disconnected",
    "connecting",
    "connected",
    "connected_msbc",
};

/// esp_hf_vr_state_t
const char *c_vr_state_str[] = {
    "Disabled",
    "Enabled",
};

// esp_hf_nrec_t
const char *c_nrec_status_str[] = {
    "NREC DISABLE",
    "NREC ABLE",
};

// esp_hf_control_target_t
const char *c_volume_control_target_str[] = {
    "SPEAKER",
    "MICROPHONE",
};

// esp_hf_subscriber_service_type_t
char *c_operator_name_str[] = {
    "中国移动",
    "中国联通",
    "中国电信",
};

// esp_hf_subscriber_service_type_t
char *c_subscriber_service_type_str[] = {
    "UNKNOWN",
    "VOICE",
    "FAX",
};

// esp_hf_nego_codec_status_t
const char *c_codec_mode_str[] = {
    "CVSD Only",
    "Use CVSD",
    "Use MSBC",
};

#if CONFIG_BT_HFP_AUDIO_DATA_PATH_HCI
#define TABLE_SIZE         100
#define TABLE_SIZE_BYTE    200
// Produce a sine audio
static const int16_t sine_int16[TABLE_SIZE] = {
     0,    2057,    4107,    6140,    8149,   10126,   12062,   13952,   15786,   17557,
 19260,   20886,   22431,   23886,   25247,   26509,   27666,   28714,   29648,   30466,
 31163,   31738,   32187,   32509,   32702,   32767,   32702,   32509,   32187,   31738,
 31163,   30466,   29648,   28714,   27666,   26509,   25247,   23886,   22431,   20886,
 19260,   17557,   15786,   13952,   12062,   10126,    8149,    6140,    4107,    2057,
     0,   -2057,   -4107,   -6140,   -8149,  -10126,  -12062,  -13952,  -15786,  -17557,
-19260,  -20886,  -22431,  -23886,  -25247,  -26509,  -27666,  -28714,  -29648,  -30466,
-31163,  -31738,  -32187,  -32509,  -32702,  -32767,  -32702,  -32509,  -32187,  -31738,
-31163,  -30466,  -29648,  -28714,  -27666,  -26509,  -25247,  -23886,  -22431,  -20886,
-19260,  -17557,  -15786,  -13952,  -12062,  -10126,   -8149,   -6140,   -4107,   -2057,
};

#define ESP_HFP_RINGBUF_SIZE 3600

// 7500 microseconds(=12 slots) is aligned to 1 msbc frame duration, and is multiple of common Tesco for eSCO link with EV3 or 2-EV3 packet type
#define PCM_BLOCK_DURATION_US        (7500)

#define WBS_PCM_SAMPLING_RATE_KHZ    (16)
#define PCM_SAMPLING_RATE_KHZ        (8)

#define BYTES_PER_SAMPLE             (2)

// input can refer to Enhanced Setup Synchronous Connection Command in core spec4.2 Vol2, Part E
#define WBS_PCM_INPUT_DATA_SIZE  (WBS_PCM_SAMPLING_RATE_KHZ * PCM_BLOCK_DURATION_US / 1000 * BYTES_PER_SAMPLE) //240
#define PCM_INPUT_DATA_SIZE      (PCM_SAMPLING_RATE_KHZ * PCM_BLOCK_DURATION_US / 1000 * BYTES_PER_SAMPLE)     //120

#define PCM_GENERATOR_TICK_US        (4000)

static long s_data_num = 0;
static RingbufHandle_t s_m_rb = NULL;
static uint64_t s_time_new, s_time_old;
static esp_timer_handle_t s_periodic_timer;
static uint64_t s_last_enter_time, s_now_enter_time;
static uint64_t s_us_duration;
static SemaphoreHandle_t s_send_data_Semaphore = NULL;
static TaskHandle_t s_bt_app_send_data_task_handler = NULL;
static esp_hf_audio_state_t s_audio_code;

static void print_speed(void);

static uint32_t bt_app_hf_outgoing_cb(uint8_t *p_buf, uint32_t sz)
{
    size_t item_size = 0;
    uint8_t *data;
    if (!s_m_rb) {
        return 0;
    }
    vRingbufferGetInfo(s_m_rb, NULL, NULL, NULL, NULL, &item_size);
    if (item_size >= sz) {
        data = xRingbufferReceiveUpTo(s_m_rb, &item_size, 0, sz);
        memcpy(p_buf, data, item_size);
        vRingbufferReturnItem(s_m_rb, data);
        return sz;
    } else {
        // data not enough, do not read\n
        return 0;
    }
    return 0;
}

static void bt_app_hf_incoming_cb(const uint8_t *buf, uint32_t sz)
{
    s_time_new = esp_timer_get_time();
    s_data_num += sz;
    if ((s_time_new - s_time_old) >= 3000000) {
        print_speed();
    }
}

static uint32_t bt_app_hf_create_audio_data(uint8_t *p_buf, uint32_t sz)
{
    static int index = 0;
    uint8_t *data = (uint8_t *)sine_int16;

    for (uint32_t i = 0; i < sz; i++) {
        p_buf[i] = data[index++];
        if (index >= TABLE_SIZE_BYTE) {
            index -= TABLE_SIZE_BYTE;
        }
    }
    return sz;
}

static void print_speed(void)
{
    float tick_s = (s_time_new - s_time_old) / 1000000.0;
    float speed = s_data_num * 8 / tick_s / 1000.0;
    ESP_LOGI(BT_HF_TAG, "speed(%fs ~ %fs): %f kbit/s" , s_time_old / 1000000.0, s_time_new / 1000000.0, speed);
    s_data_num = 0;
    s_time_old = s_time_new;
}

static void bt_app_send_data_timer_cb(void *arg)
{
    if (!xSemaphoreGive(s_send_data_Semaphore)) {
        ESP_LOGE(BT_HF_TAG, "%s xSemaphoreGive failed", __func__);
        return;
    }
    return;
}

static void bt_app_send_data_task(void *arg)
{
    uint64_t frame_data_num;
    size_t item_size = 0;
    uint8_t *buf = NULL;
    for (;;) {
        if (xSemaphoreTake(s_send_data_Semaphore, (TickType_t)portMAX_DELAY)) {
            s_now_enter_time = esp_timer_get_time();
            s_us_duration = s_now_enter_time - s_last_enter_time;
            if(s_audio_code == ESP_HF_AUDIO_STATE_CONNECTED_MSBC) {
            // time of a frame is 7.5ms, sample is 120, data is 2 (byte/sample), so a frame is 240 byte (HF_SBC_ENC_RAW_DATA_SIZE)
                frame_data_num = s_us_duration / PCM_BLOCK_DURATION_US * WBS_PCM_INPUT_DATA_SIZE;
                s_last_enter_time += frame_data_num / WBS_PCM_INPUT_DATA_SIZE * PCM_BLOCK_DURATION_US;
            } else {
                frame_data_num = s_us_duration / PCM_BLOCK_DURATION_US * PCM_INPUT_DATA_SIZE;
                s_last_enter_time += frame_data_num / PCM_INPUT_DATA_SIZE * PCM_BLOCK_DURATION_US;
            }
            if (frame_data_num == 0) {
                continue;
            }
            buf = osi_malloc(frame_data_num);
            if (!buf) {
                ESP_LOGE(BT_HF_TAG, "%s, no mem", __FUNCTION__);
                continue;
            }
            bt_app_hf_create_audio_data(buf, frame_data_num);
            BaseType_t done = xRingbufferSend(s_m_rb, buf, frame_data_num, 0);
            if (!done) {
                ESP_LOGE(BT_HF_TAG, "rb send fail");
            }
            osi_free(buf);
            vRingbufferGetInfo(s_m_rb, NULL, NULL, NULL, NULL, &item_size);

            if(s_audio_code == ESP_HF_AUDIO_STATE_CONNECTED_MSBC) {
                if(item_size >= WBS_PCM_INPUT_DATA_SIZE) {
                    esp_hf_ag_outgoing_data_ready();
                }
            } else {
                if(item_size >= PCM_INPUT_DATA_SIZE) {
                    esp_hf_ag_outgoing_data_ready();
                }
            }
        }
    }
}
void bt_app_send_data(void)
{
    s_send_data_Semaphore = xSemaphoreCreateBinary();
    xTaskCreate(bt_app_send_data_task, "BtAppSendDataTask", 2048, NULL, configMAX_PRIORITIES - 3, &s_bt_app_send_data_task_handler);
    s_m_rb = xRingbufferCreate(ESP_HFP_RINGBUF_SIZE, RINGBUF_TYPE_BYTEBUF);
    const esp_timer_create_args_t c_periodic_timer_args = {
            .callback = &bt_app_send_data_timer_cb,
            .name = "periodic"
    };
    ESP_ERROR_CHECK(esp_timer_create(&c_periodic_timer_args, &s_periodic_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(s_periodic_timer, PCM_GENERATOR_TICK_US));
    s_last_enter_time = esp_timer_get_time();
    return;
}

void bt_app_send_data_shut_down(void)
{
    if (s_bt_app_send_data_task_handler) {
        vTaskDelete(s_bt_app_send_data_task_handler);
        s_bt_app_send_data_task_handler = NULL;
    }
    if(s_periodic_timer) {
        ESP_ERROR_CHECK(esp_timer_stop(s_periodic_timer));
        ESP_ERROR_CHECK(esp_timer_delete(s_periodic_timer));
    }
    if (s_send_data_Semaphore) {
        vSemaphoreDelete(s_send_data_Semaphore);
        s_send_data_Semaphore = NULL;
    }
    if (s_m_rb) {
        vRingbufferDelete(s_m_rb);
    }
    return;
}
#endif /* #if CONFIG_BT_HFP_AUDIO_DATA_PATH_HCI */

/*
 * The function bt_app_hf_cb is critical for handling Bluetooth events. 
 * This function handles all the critical bluetooth events: connect, audio, voice recognition, 
 * volume control, unknown AT commands, call indication, current operator events, and more. 
 *
 * These events guide the application on how to respond to various Bluetooth interactions.
 */
void bt_app_hf_cb(esp_hf_cb_event_t event, esp_hf_cb_param_t *param)
{
    if (event <= ESP_HF_PKT_STAT_NUMS_GET_EVT) {
        ESP_LOGI(BT_HF_TAG, "APP HFP event: %s", c_hf_evt_str[event]);
    } else {
        ESP_LOGE(BT_HF_TAG, "APP HFP invalid event %d", event);
    }

    switch (event) {
        case ESP_HF_CONNECTION_STATE_EVT:
        {
            ESP_LOGI(BT_HF_TAG, "--connection state %s, peer feats 0x%"PRIx32", chld_feats 0x%"PRIx32,
                    c_connection_state_str[param->conn_stat.state],
                    param->conn_stat.peer_feat,
                    param->conn_stat.chld_feat);
            memcpy(hf_peer_addr, param->conn_stat.remote_bda, ESP_BD_ADDR_LEN);
            break;
        }

        case ESP_HF_AUDIO_STATE_EVT:
        {
            ESP_LOGI(BT_HF_TAG, "--Audio State %s", c_audio_state_str[param->audio_stat.state]);
#if CONFIG_BT_HFP_AUDIO_DATA_PATH_HCI
            if (param->audio_stat.state == ESP_HF_AUDIO_STATE_CONNECTED ||
                param->audio_stat.state == ESP_HF_AUDIO_STATE_CONNECTED_MSBC)
            {
                if(param->audio_stat.state == ESP_HF_AUDIO_STATE_CONNECTED) {
                    s_audio_code = ESP_HF_AUDIO_STATE_CONNECTED;
                } else {
                    s_audio_code = ESP_HF_AUDIO_STATE_CONNECTED_MSBC;
                }
                s_time_old = esp_timer_get_time();
                esp_hf_ag_register_data_callback(bt_app_hf_incoming_cb, bt_app_hf_outgoing_cb);
                /* Begin send esco data task */
                bt_app_send_data();
            } else if (param->audio_stat.state == ESP_HF_AUDIO_STATE_DISCONNECTED) {
                ESP_LOGI(BT_HF_TAG, "--ESP AG Audio Connection Disconnected.");
                bt_app_send_data_shut_down();
            }
#endif /* #if CONFIG_BT_HFP_AUDIO_DATA_PATH_HCI */
            break;
        }

        case ESP_HF_BVRA_RESPONSE_EVT:
        {
            ESP_LOGI(BT_HF_TAG, "--Voice Recognition is %s", c_vr_state_str[param->vra_rep.value]);
            break;
        }

        case ESP_HF_VOLUME_CONTROL_EVT:
        {
            ESP_LOGI(BT_HF_TAG, "--Volume Target: %s, Volume %d", c_volume_control_target_str[param->volume_control.type], param->volume_control.volume);
            break;
        }

        case ESP_HF_UNAT_RESPONSE_EVT:
        {
            ESP_LOGI(BT_HF_TAG, "--UNKOW AT CMD: %s", param->unat_rep.unat);
            esp_hf_ag_unknown_at_send(param->unat_rep.remote_addr, NULL);
            break;
        }

        case ESP_HF_IND_UPDATE_EVT:
        {
            ESP_LOGI(BT_HF_TAG, "--UPDATE INDCATOR!");
            esp_hf_call_status_t call_state = 1;
            esp_hf_call_setup_status_t call_setup_state = 2;
            esp_hf_network_state_t ntk_state = 1;
            int signal = 2;
            // esp_hf_ag_devices_status_indchange(param->ind_upd.remote_addr, call_state, call_setup_state, ntk_state, signal); //deprecated
            esp_hf_ag_ciev_report(param->ind_upd.remote_addr, ESP_HF_IND_TYPE_CALL, call_state);
            esp_hf_ag_ciev_report(param->ind_upd.remote_addr, ESP_HF_IND_TYPE_CALLSETUP, call_setup_state);
            esp_hf_ag_ciev_report(param->ind_upd.remote_addr, ESP_HF_IND_TYPE_SERVICE, ntk_state);
            esp_hf_ag_ciev_report(param->ind_upd.remote_addr, ESP_HF_IND_TYPE_SIGNAL, signal);
            // esp_hf_ag_ciev_report(param->ind_upd.remote_addr, ESP_HF_IND_TYPE_BATTCHG, battery);

            break;
        }

        case ESP_HF_CIND_RESPONSE_EVT:
        {
            ESP_LOGI(BT_HF_TAG, "--CIND Start.");
            esp_hf_call_status_t call_status = 0;
            esp_hf_call_setup_status_t call_setup_status = 0;
            esp_hf_network_state_t ntk_state = 1;
            int signal = 4;
            esp_hf_roaming_status_t roam = 0;
            int batt_lev = 3;
            esp_hf_call_held_status_t call_held_status = 0;
            esp_hf_ag_cind_response(param->cind_rep.remote_addr,call_status,call_setup_status,ntk_state,signal,roam,batt_lev,call_held_status);
            break;
        }

        case ESP_HF_COPS_RESPONSE_EVT:
        {
            const int svc_type = 1;
            esp_hf_ag_cops_response(param->cops_rep.remote_addr, c_operator_name_str[svc_type]);
            break;
        }

        case ESP_HF_CLCC_RESPONSE_EVT:
        {
            int index = 1;
            //mandatory
            esp_hf_current_call_direction_t dir = 1;
            esp_hf_current_call_status_t current_call_status = 0;
            esp_hf_current_call_mode_t mode = 0;
            esp_hf_current_call_mpty_type_t mpty = 0;
            //option
            char *number = {"123456"};
            esp_hf_call_addr_type_t type = ESP_HF_CALL_ADDR_TYPE_UNKNOWN;

            ESP_LOGI(BT_HF_TAG, "--Calling Line Identification.");
            esp_hf_ag_clcc_response(param->clcc_rep.remote_addr, index, dir, current_call_status, mode, mpty, number, type);
            break;
        }


        case ESP_HF_CNUM_RESPONSE_EVT:
        {
            char *number = {"123456"};
            int number_type = 129;
            esp_hf_subscriber_service_type_t service_type = ESP_HF_SUBSCRIBER_SERVICE_TYPE_VOICE;
            if (service_type == ESP_HF_SUBSCRIBER_SERVICE_TYPE_VOICE || service_type == ESP_HF_SUBSCRIBER_SERVICE_TYPE_FAX) {
                ESP_LOGI(BT_HF_TAG, "--Current Number is %s, Number Type is %d, Service Type is %s.", number, number_type, c_subscriber_service_type_str[service_type - 3]);
            } else {
                ESP_LOGI(BT_HF_TAG, "--Current Number is %s, Number Type is %d, Service Type is %s.", number, number_type, c_subscriber_service_type_str[0]);
            }
            esp_hf_ag_cnum_response(hf_peer_addr, number, number_type, service_type);
            break;
        }

        // case ESP_HF_CNUM_RESPONSE_EVT:
        // {
        //     char *number = {"123456"};
        //     esp_hf_subscriber_service_type_t type = 1;
        //     ESP_LOGI(BT_HF_TAG, "--Current Number is %s ,Type is %s.", number, c_subscriber_service_type_str[type]);
        //     esp_hf_ag_cnum_response(param->cnum_rep.remote_addr, number, type);
        //     esp_hf_ag_cnum_response(hf_peer_addr, number, number_type, type);
        //     //  signature:
        //     //      esp_err_t esp_hf_ag_cnum_response(esp_bd_addr_t remote_addr, char *number, int number_type, esp_hf_subscriber_service_type_t service_type);

        //     break;
        // }

        case ESP_HF_VTS_RESPONSE_EVT:
        {
            ESP_LOGI(BT_HF_TAG, "--DTMF code is: %s.", param->vts_rep.code);
            break;
        }

        case ESP_HF_NREC_RESPONSE_EVT:
        {
            ESP_LOGI(BT_HF_TAG, "--NREC status is: %s.", c_nrec_status_str[param->nrec.state]);
            break;
        }

        case ESP_HF_ATA_RESPONSE_EVT:
        {
            ESP_LOGI(BT_HF_TAG, "--Asnwer Incoming Call.");
            char *number = {"123456"};
            esp_hf_ag_answer_call(param->ata_rep.remote_addr,1,0,1,0,number,0);
            break;
        }

        case ESP_HF_CHUP_RESPONSE_EVT:
        {
            ESP_LOGI(BT_HF_TAG, "--Reject Incoming Call.");
            char *number = {"123456"};
            esp_hf_ag_reject_call(param->chup_rep.remote_addr,0,0,0,0,number,0);
            break;
        }

        case ESP_HF_DIAL_EVT:
        {
            if (param->out_call.num_or_loc) {
                if (param->out_call.type == ESP_HF_DIAL_NUM) {
                    // dia_num
                    ESP_LOGI(BT_HF_TAG, "--Dial number \"%s\".", param->out_call.num_or_loc);
                    esp_hf_ag_out_call(param->out_call.remote_addr,1,0,1,0,param->out_call.num_or_loc,0);
                } else if (param->out_call.type == ESP_HF_DIAL_MEM) {
                    // dia_mem
                    ESP_LOGI(BT_HF_TAG, "--Dial memory \"%s\".", param->out_call.num_or_loc);
                    // AG found phone number by memory position
                    bool num_found = true;
                    if (num_found) {
                        char *number = "123456";
                        esp_hf_ag_cmee_send(param->out_call.remote_addr, ESP_HF_AT_RESPONSE_CODE_OK, ESP_HF_CME_AG_FAILURE);
                        esp_hf_ag_out_call(param->out_call.remote_addr,1,0,1,0,number,0);
                    } else {
                        esp_hf_ag_cmee_send(param->out_call.remote_addr, ESP_HF_AT_RESPONSE_CODE_CME, ESP_HF_CME_MEMORY_FAILURE);
                    }
                }
            } else {
                //dia_last
                //refer to dia_mem
                ESP_LOGI(BT_HF_TAG, "--Dial last number.");
            }
            break;
        }
#if (CONFIG_BT_HFP_WBS_ENABLE)
        case ESP_HF_WBS_RESPONSE_EVT:
        {
            ESP_LOGI(BT_HF_TAG, "--Current codec: %s",c_codec_mode_str[param->wbs_rep.codec]);
            break;
        }
#endif
        case ESP_HF_BCS_RESPONSE_EVT:
        {
            ESP_LOGI(BT_HF_TAG, "--Consequence of codec negotiation: %s",c_codec_mode_str[param->bcs_rep.mode]);
            break;
        }
        case ESP_HF_PKT_STAT_NUMS_GET_EVT:
        {
            ESP_LOGI(BT_HF_TAG, "ESP_HF_PKT_STAT_NUMS_GET_EVT: %d.", event);
            break;
        }

        default:
            ESP_LOGI(BT_HF_TAG, "Unsupported HF_AG EVT: %d.", event);
            break;

    }
}
