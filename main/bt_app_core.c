/*
 * SPDX-FileCopyrightText: 2021-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */

/*
bt_app_core.c 

Overall Responsibility: 
This file handles the dispatching and processing of Bluetooth-related tasks and messages. 
It manages a FreeRTOS queue to handle Bluetooth tasks/messages and facilitates inter-task communication.

Important Variables:

1. bt_app_task_queue: A FreeRTOS queue that holds Bluetooth application messages/tasks.
2. bt_app_task_handle: A handle to the task that processes Bluetooth messages/tasks.

Important Functions:

1. bt_app_work_dispatch(bt_app_cb_t p_cback, uint16_t event, void *p_params, int param_len, bt_app_copy_cb_t p_copy_cback):
   - Purpose: Creates a message and sends it to the task queue for further processing. Can handle deep copying via a provided callback.
   - Parameters:
     - p_cback: Callback function to execute.
     - event: The event associated with the message.
     - p_params: Pointer to message parameters.
     - param_len: Length of the parameters.
     - p_copy_cback: Optional deep copy callback.
   
2. bt_app_send_msg(bt_app_msg_t *msg):
   - Purpose: Sends a message to the `bt_app_task_queue`.
   - Parameters:
     - msg: Pointer to the message to send.
   
3. bt_app_work_dispatched(bt_app_msg_t *msg):
   - Purpose: Executes the callback associated with a dispatched message.
   - Parameters:
     - msg: The message containing the callback and other information.
   
4. bt_app_task_handler(void *arg):
   - Purpose: Task that waits for and processes messages from the `bt_app_task_queue`.
   - Key Actions:
     - Waits for a message from the queue.
     - Logs the message.
     - Handles the message based on its signature, currently supporting `BT_APP_SIG_WORK_DISPATCH`.
     - Frees any dynamically allocated parameters in the message.
   
5. bt_app_task_start_up(void):
   - Purpose: Initializes the task and queue for Bluetooth message handling.
   - Key Actions:
     - Creates the `bt_app_task_queue`.
     - Creates the `bt_app_task_handler` task.
   
6. bt_app_task_shut_down(void):
   - Purpose: De-initializes the task and queue associated with Bluetooth message handling.
   - Key Actions:
     - Deletes the `bt_app_task_handler` task.
     - Deletes the `bt_app_task_queue`.

From this analysis, the `bt_app_core.c` file focuses on managing the dispatch and handling 
of Bluetooth-related tasks/messages. It uses a FreeRTOS queue to hold tasks/messages and 
provides functions for dispatching work to this queue, starting the task to handle messages, 
and shutting down the task.
*/

#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include "freertos/xtensa_api.h"
#include "freertos/FreeRTOSConfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "bt_app_core.h"

static void bt_app_task_handler(void *arg);
static bool bt_app_send_msg(bt_app_msg_t *msg);
static void bt_app_work_dispatched(bt_app_msg_t *msg);

static QueueHandle_t bt_app_task_queue = NULL;
static TaskHandle_t bt_app_task_handle = NULL;

bool bt_app_work_dispatch(bt_app_cb_t p_cback, uint16_t event, void *p_params, int param_len, bt_app_copy_cb_t p_copy_cback)
{
    ESP_LOGD(BT_APP_CORE_TAG, "%s event 0x%x, param len %d", __func__, event, param_len);

    bt_app_msg_t msg;
    memset(&msg, 0, sizeof(bt_app_msg_t));

    msg.sig = BT_APP_SIG_WORK_DISPATCH;
    msg.event = event;
    msg.cb = p_cback;

    if (param_len == 0) {
        return bt_app_send_msg(&msg);
    } else if (p_params && param_len > 0) {
        if ((msg.param = malloc(param_len)) != NULL) {
            memcpy(msg.param, p_params, param_len);
            /* check if caller has provided a copy callback to do the deep copy */
            if (p_copy_cback) {
                p_copy_cback(&msg, msg.param, p_params);
            }
            return bt_app_send_msg(&msg);
        }
    }
    return false;
}

static bool bt_app_send_msg(bt_app_msg_t *msg)
{
    if (msg == NULL) {
        return false;
    }

    if (xQueueSend(bt_app_task_queue, msg, 10 / portTICK_PERIOD_MS) != pdTRUE) {
        ESP_LOGE(BT_APP_CORE_TAG, "%s xQueue send failed", __func__);
        return false;
    }
    return true;
}

static void bt_app_work_dispatched(bt_app_msg_t *msg)
{
    if (msg->cb) {
        msg->cb(msg->event, msg->param);
    }
}

static void bt_app_task_handler(void *arg)
{
    bt_app_msg_t msg;
    for (;;) {
        if (pdTRUE == xQueueReceive(bt_app_task_queue, &msg, (TickType_t)portMAX_DELAY)) {
            ESP_LOGD(BT_APP_CORE_TAG, "%s, sig 0x%x, 0x%x", __func__, msg.sig, msg.event);
            switch (msg.sig) {
            case BT_APP_SIG_WORK_DISPATCH:
                bt_app_work_dispatched(&msg);
                break;
            default:
                ESP_LOGW(BT_APP_CORE_TAG, "%s, unhandled sig: %d", __func__, msg.sig);
                break;
            } // switch (msg.sig)

            if (msg.param) {
                free(msg.param);
            }
        }
    }
}

void bt_app_task_start_up(void)
{
    bt_app_task_queue = xQueueCreate(10, sizeof(bt_app_msg_t));
    xTaskCreate(bt_app_task_handler, "BtAppT", 2048, NULL, configMAX_PRIORITIES - 3, &bt_app_task_handle);
    return;
}

void bt_app_task_shut_down(void)
{
    if (bt_app_task_handle) {
        vTaskDelete(bt_app_task_handle);
        bt_app_task_handle = NULL;
    }
    if (bt_app_task_queue) {
        vQueueDelete(bt_app_task_queue);
        bt_app_task_queue = NULL;
    }
}
