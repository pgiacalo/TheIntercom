/*
 * SPDX-FileCopyrightText: 2021-2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */

/*
main.c 

Overall Responsibility: 
The `main.c` file serves as the entry point for the application. 

It is responsible for initializing various systems, including the NVS, 
Bluetooth controller, Bluedroid stack, and the REPL console for UART. 

Additionally, it sets up the Bluetooth device name, connection mode, 
profile, and handles certain Bluetooth events.

Important Variables:

1. BT_HF_AG_TAG: A string used for tagging log messages related to this file/module.
2. BT_APP_EVT_STACK_UP: An enumeration which represents a Bluetooth event indicating the Bluetooth stack is up.
3. bt_cfg: A structure that holds the configuration for the Bluetooth controller.
4. bluedroid_cfg: A structure that holds the configuration for the Bluedroid Bluetooth stack.
5. repl: A pointer to a REPL structure that represents the console.
6. repl_config: Configuration for the REPL console.
7. uart_config: Configuration for the UART console device.

Important Functions:

1. bt_hf_hdl_stack_evt(uint16_t event, void *p_param):
   - Purpose: Handler function for Bluetooth stack enabled events.
   - Key Actions: 
     - When the Bluetooth stack is up (as indicated by the `BT_APP_EVT_STACK_UP` event), it sets up the Bluetooth device name, registers a callback for HFP (Hands-Free Profile), initializes HFP functions, sets parameters for legacy pairing, and sets the device in discoverable and connectable modes.
     - Logs any unhandled events.

2. app_main(void):
   - Purpose: Entry point for the application.
   - Key Actions:
     - Initializes NVS for storing PHY calibration data.
     - Initializes and enables the Bluetooth controller in classic Bluetooth mode (BR/EDR).
     - Initializes and enables the Bluedroid Bluetooth stack.
     - Starts the application task using the `bt_app_task_start_up()` function.
     - Dispatches a task to handle Bluetooth stack enabled events.
     - Configures the PCM interface (if enabled via a preprocessor directive).
     - Configures the external chip for Acoustic Echo Cancellation (if enabled).
     - Sets up the REPL (Read-Eval-Print Loop) console for UART and registers related commands.
     - Provides instructions on how to test the HFP AG via printed messages.
     - Starts the REPL console.

From this breakdown, it's clear that the `main.c` file focuses on setting up necessary 
systems and configurations for the application to run and engage in Bluetooth operations,
especially related to the Hands-Free Profile (HFP).
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "esp_system.h"
#include "esp_log.h"
#include "esp_bt.h"
#include "bt_app_core.h"
#include "esp_bt_main.h"
#include "esp_bt_device.h"
#include "esp_gap_bt_api.h"
#include "esp_hf_ag_api.h"
#include "bt_app_hf.h"
#include "esp_console.h"
#include "app_hf_msg_set.h"
#include "gpio_pcm_config.h"


#define BT_HF_AG_TAG    "HF_AG_DEMO_MAIN"

/* event for handler "hf_ag_hdl_stack_up */
enum {
    BT_APP_EVT_STACK_UP = 0,
};

/* handler for bluetooth stack enabled events */
static void bt_hf_hdl_stack_evt(uint16_t event, void *p_param)
{
    ESP_LOGD(BT_HF_TAG, "%s evt %d", __func__, event);
    switch (event)
    {
        case BT_APP_EVT_STACK_UP:
        {
            /* set up device name */
            char *dev_name = "ESP_HFP_AG";
            esp_bt_dev_set_device_name(dev_name);

            esp_hf_ag_register_callback(bt_app_hf_cb);

            // init and register for HFP_AG functions
            esp_hf_ag_init();

            /*
            * Set default parameters for Legacy Pairing
            * Use variable pin, input pin code when pairing
            */
            esp_bt_pin_type_t pin_type = ESP_BT_PIN_TYPE_VARIABLE;
            esp_bt_pin_code_t pin_code;
            pin_code[0] = '0';
            pin_code[1] = '0';
            pin_code[2] = '0';
            pin_code[3] = '0';
            esp_bt_gap_set_pin(pin_type, 4, pin_code);

            /* set discoverable and connectable mode, wait to be connected */
            esp_bt_gap_set_scan_mode(ESP_BT_CONNECTABLE, ESP_BT_GENERAL_DISCOVERABLE);
            break;
        }
        default:
            ESP_LOGE(BT_HF_TAG, "%s unhandled evt %d", __func__, event);
            break;
    }
}


esp_err_t start_bluetooth_and_bluedroid(){
    //release BLE memory, since we're not using BLE
    ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_BLE));

    esp_err_t ret;

    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    if ((ret = esp_bt_controller_init(&bt_cfg)) != ESP_OK) {
        ESP_LOGE(BT_HF_TAG, "%s initialize controller failed: %s", __func__, esp_err_to_name(ret));
        return ret;
    }
    if ((ret = esp_bt_controller_enable(ESP_BT_MODE_CLASSIC_BT)) != ESP_OK) {                   
        ESP_LOGE(BT_HF_TAG, "%s enable controller failed: %s", __func__, esp_err_to_name(ret)); //to fix, set menuconfig to "BR/EDR only"
        return ret;
    }
    esp_bluedroid_config_t bluedroid_cfg = BT_BLUEDROID_INIT_CONFIG_DEFAULT();
    if ((ret = esp_bluedroid_init_with_cfg(&bluedroid_cfg)) != ESP_OK) {
        ESP_LOGE(BT_HF_TAG, "%s initialize bluedroid failed: %s", __func__, esp_err_to_name(ret));
        return ret;
    }
    if ((ret = esp_bluedroid_enable()) != ESP_OK) {
        ESP_LOGE(BT_HF_TAG, "%s enable bluedroid failed: %s", __func__, esp_err_to_name(ret));
        return ret;
    }

    return ESP_OK;
}

/*
NVS stands for "Non-Volatile Storage." 

The data stored in NVS will remain even after the device is powered off or rebooted.

In the context of embedded systems and especially in relation to the ESP32 and ESP8266, 
NVS refers to a specific portion of the flash memory where key-value pairs can be stored persistently. 

Key features and uses of NVS include:

    - Persistency: Unlike RAM, the data in NVS does not get wiped out when power is lost. This makes it suitable for storing configuration settings, calibration data, state information, etc.
    - Key-Value Store: NVS provides a simple mechanism to store and retrieve data using key-value pairs. This means that for each piece of data (or value) you want to store, you associate it with a unique key. Later, you can retrieve the data using the same key.
    - Different Data Types: While it's essentially a byte-storage mechanism, NVS provides APIs that allow for the storage of different data types, including integers, blobs (binary large objects), and strings.
    - Wear Leveling: Flash memory has a limited number of write/erase cycles for each sector. Wear leveling ensures that these cycles are distributed evenly across the sectors, thus extending the life of the flash memory.
    - Partition: On platforms like ESP32, the NVS is typically a part of a partition in the flash memory, and its size and position can be configured based on the application's needs.
    - Developers often use NVS to store parameters that need to be retained across reboots, such as Wi-Fi credentials, device configuration, calibration data, and more.
*/
void init_nvs(){
    /* Initialize NVS — it is used to store PHY calibration data */
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
}


void configure_gpio_pins(){
    #if CONFIG_BT_HFP_AUDIO_DATA_PATH_PCM   //configured in sdkconfig
        /* configure the PCM interface and PINs used */
        app_gpio_pcm_io_cfg();
    #endif

        /* configure externel chip for acoustic echo cancellation */
    #if ACOUSTIC_ECHO_CANCELLATION_ENABLE   //configured in gpio_pcm_config.h
        app_gpio_aec_io_cfg();
    #endif /* ACOUSTIC_ECHO_CANCELLATION_ENABLE */

}

/*
A console REPL (Read-Eval-Print Loop) is an interactive computer programming environment that takes single user inputs, 
evaluates them, and returns the result to the user. It provides a way for developers to interactively run code and 
see results in real-time, which can be immensely helpful for debugging, learning, and experimentation.

Here's a breakdown of the acronym "REPL":

    - Read: Read user input. This is where the system waits for user input and then reads the command once entered.
    - Eval: Evaluate the input. This step interprets the user's command.
    - Print: Print the result. After evaluating the command, the result is then displayed to the user.
    - Loop: The system goes back to the read stage after printing, and the cycle repeats.
*/
void start_repl_console(){
    esp_console_repl_t *repl = NULL;
    esp_console_repl_config_t repl_config = ESP_CONSOLE_REPL_CONFIG_DEFAULT();
    esp_console_dev_uart_config_t uart_config = ESP_CONSOLE_DEV_UART_CONFIG_DEFAULT();
    repl_config.prompt = "hfp_ag>";

    // init console REPL environment ("Read-Eval-Print Loop")
    ESP_ERROR_CHECK(esp_console_new_repl_uart(&uart_config, &repl_config, &repl));

    /* Register commands */
    register_hfp_ag();
    printf("\n ==================================================\n");
    printf(" |       Steps to test hfp_ag                     |\n");
    printf(" |                                                |\n");
    printf(" |  1. Print 'help' to gain overview of commands  |\n");
    printf(" |  2. Setup a service level connection           |\n");
    printf(" |  3. Run hfp_ag to test                         |\n");
    printf(" |                                                |\n");
    printf(" =================================================\n\n");

    // start console REPL
    ESP_ERROR_CHECK(esp_console_start_repl(repl));

}


/*
 * 
 * Initializes NVS. This is useful for storing PHY calibration data.
 * Releases memory associated with Bluetooth Low Energy (BLE) mode, since the application is focused on classic Bluetooth (BR/EDR).
 * Initializes and enables the Bluetooth controller.
 * Initializes and enables the Bluedroid Bluetooth stack.
 * Starts the application task by calling bt_app_task_start_up.
 * Dispatches a task to handle Bluetooth stack enabled events.
 * If the Bluetooth HFP audio data path is set to PCM, it configures the PCM interface using app_gpio_pcm_io_cfg.
 * If Acoustic Echo Cancellation (AEC) is enabled, the related GPIOs are configured using app_gpio_aec_io_cfg.
 * Sets up the REPL (Read-Eval-Print Loop) console for UART, providing an interactive command-line interface for the application.
 * Registers commands for the HFP AG.
 * Provides a printed guide on how to test the HFP AG.
 * Starts the REPL console.
 */
void app_main(void){
    init_nvs();

    esp_err_t ret = start_bluetooth_and_bluedroid();
    if (ret != ESP_OK) {
        return;
    }

    /* create application task */
    bt_app_task_start_up();

    /* Setup bluetooth device name, connection mode and profile */
    bt_app_work_dispatch(bt_hf_hdl_stack_evt, BT_APP_EVT_STACK_UP, NULL, 0, NULL);

    configure_gpio_pins();

    start_repl_console();
}
