/*
 * SPDX-FileCopyrightText: 2021 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */

/*
This file `gpio_pcm_config.c` relates to the configuration of the GPIO pins for Pulse Code Modulation (PCM) 
audio data transfer and optionally, for Acoustic Echo Cancellation (AEC) if enabled. 
PCM is commonly used to digitally represent analog signals, and AEC is useful to eliminate undesired echoing 
during voice calls.

Here's a breakdown of the file:

1. Inclusion of Headers:
    - The file includes relevant headers for GPIO and Bluetooth configuration and signal mapping, 
    as well as logging utilities (`esp_log.h`).

2. Role-Based GPIO Definitions:
    - the code sets GPIO pins, depending on whether the device is acting as a master or a slave, 
    per the `DEVICE_ROLE` macro defined in `bluetooth_config.h`.

3. PCM GPIO Configuration Function `app_gpio_pcm_io_cfg`:
    - This function configures GPIO pins for sending and receiving PCM audio data. Depending on 
    the role (master or slave), 
    - It also maps specific pins for PCM signals.
    - The function uses the `gpio_config_t` structure and the `gpio_config()` function from the 
    ESP-IDF framework to set up GPIO pins.

4. AEC GPIO Configuration (Optional):
    - If the `ACOUSTIC_ECHO_CANCELLATION_ENABLE` macro is defined (indicating that AEC is enabled), 
    the file contains the function `app_gpio_aec_io_cfg` to set up GPIO pins for AEC.
    - Acoustic Echo Cancellation (AEC) is a process to eliminate unwanted echoes that might be 
    heard during a phone call due to the speaker's audio output being captured by the microphone.
    - The function uses the same `gpio_config_t` structure and `gpio_config()` function to set up the GPIO pins.

5. Logging: 
    - Throughout the code, there are log statements (like `ESP_LOGI()`) that would output relevant information 
    and status messages to a console or terminal. This aids in debugging and understanding the flow of the program.

6. Comments: 
    - The code contains explanatory comments that describe the purpose and functionality of various code segments, 
    which is a good practice for understanding and maintaining the code.

To integrate this code into a project, ensure that the relevant pin numbers and roles (master/slave) 
are set in the `bluetooth_config.h` file. Also, if you need Acoustic Echo Cancellation, 
the relevant macro should be defined. As with all hardware-related configurations, ensure you understand 
the hardware design and connections before modifying or using this code.
*/

#include "bluetooth_config.h"
#include "driver/gpio.h"
#include "soc/gpio_reg.h"
#include "soc/gpio_sig_map.h"
#include "gpio_pcm_config.h"
#include "esp_rom_gpio.h"
#include "esp_log.h"

#define TAG     "gpio_pcm_config"

// see bluetooth_config.h for the actual pin number settings
#if DEVICE_ROLE == ROLE_MASTER
    #define GPIO_DIN         MASTER_GPIO_DIN
    #define GPIO_DOUT        MASTER_GPIO_DOUT    
    #define GPIO_BCLK        MASTER_GPIO_BCLK
    #define GPIO_LRC         MASTER_GPIO_LRC
#elif DEVICE_ROLE == ROLE_SLAVE
    #define GPIO_DIN         SLAVE_GPIO_DIN
    #define GPIO_DOUT        SLAVE_GPIO_DOUT    
    #define GPIO_BCLK        SLAVE_GPIO_BCLK
    #define GPIO_LRC         SLAVE_GPIO_LRC
#endif


#define GPIO_OUTPUT_PCM_PIN_SEL  ((1ULL<<GPIO_LRC) | (1ULL<<GPIO_BCLK) | (1ULL<<GPIO_DOUT))

#define GPIO_INPUT_PCM_PIN_SEL (1ULL<<GPIO_DIN)

/*
 * Sets up GPIO pins for Pulse Code Modulation (PCM) audio data. 
 * PCM is a method used to digitally represent analog signals. 
 * In the Bluetooth context, PCM is a standard interface for transporting audio data between chips.
 */
void app_gpio_pcm_io_cfg(void)
{
    gpio_config_t io_conf;
    /// configure the PCM output pins
    //disable interrupt
    io_conf.intr_type = GPIO_INTR_DISABLE;
    //set as output mode
    io_conf.mode = GPIO_MODE_OUTPUT;
    //bit mask of the pins that you want to set,e.g.GPIO18/19
    io_conf.pin_bit_mask = GPIO_OUTPUT_PCM_PIN_SEL;             //the output pin
    //disable pull-down mode
    io_conf.pull_down_en = 0;
    //disable pull-up mode
    io_conf.pull_up_en = 0;
    //configure GPIO with the given settings
    gpio_config(&io_conf);

    /// configure the PCM input pin
    //interrupt of rising edge
    io_conf.intr_type = GPIO_INTR_DISABLE;
    //bit mask of the pins, use GPIO4/5 here
    io_conf.pin_bit_mask = GPIO_INPUT_PCM_PIN_SEL;
    //set as input mode
    io_conf.mode = GPIO_MODE_INPUT;
    //enable pull-up mode
    io_conf.pull_up_en = 0;
    io_conf.pull_down_en = 0;
    //configure GPIO with the given settings
    gpio_config(&io_conf);

#if DEVICE_ROLE == ROLE_MASTER      //PHIL - see the esp-idf API's gpio_sig_map.h defines the PCM and I2S pin indexes (different versions for the ESP32-S3)
    // Master device sends data to the Slave
    ESP_LOGI(TAG, "USING MASTER INPUT AND OUTPUT PINS | SD_IN: %d, SD_OUT: %d, BCLK_OUT: %d, LRC_OUT: %d", GPIO_DIN, GPIO_DOUT, GPIO_BCLK, GPIO_LRC);
    esp_rom_gpio_connect_out_signal(GPIO_DOUT, PCMDOUT_IDX, false, false);   
    esp_rom_gpio_connect_out_signal(GPIO_BCLK, PCMCLK_OUT_IDX, false, false); 
    esp_rom_gpio_connect_out_signal(GPIO_LRC, PCMFSYNC_OUT_IDX, false, false);
    // Master device listens to the input from the Slave
    esp_rom_gpio_connect_in_signal(GPIO_DIN, PCMDIN_IDX, false); 
#elif DEVICE_ROLE == ROLE_SLAVE
    ESP_LOGI(TAG, "USING SLAVE INPUT AND OUTPUT PINS | SD_IN: %d, SD_OUT: %d, BCLK_IN: %d, LRC_IN: %d", GPIO_DIN, GPIO_DOUT, GPIO_BCLK, GPIO_LRC);
    // Slave device sends data to the Master
    esp_rom_gpio_connect_out_signal(GPIO_DOUT, PCMDOUT_IDX, false, false); 
    // The slave listens to the Master's clock, LRC select, and data
    esp_rom_gpio_connect_in_signal(GPIO_BCLK, PCMCLK_IN_IDX, false);    
    esp_rom_gpio_connect_in_signal(GPIO_LRC, PCMFSYNC_IN_IDX, false);  
    esp_rom_gpio_connect_in_signal(GPIO_DIN, PCMDIN_IDX, false);
#endif

}

#if ACOUSTIC_ECHO_CANCELLATION_ENABLE

#define GPIO_OUTPUT_AEC_1      (19)
#define GPIO_OUTPUT_AEC_2      (21)
#define GPIO_OUTPUT_AEC_3      (22)
#define GPIO_OUTPUT_AEC_PIN_SEL  ((1ULL<<GPIO_OUTPUT_AEC_1) | (1ULL<<GPIO_OUTPUT_AEC_2) | (1ULL<<GPIO_OUTPUT_AEC_3))

/* 
 * If AEC is enabled (with macro ACOUSTIC_ECHO_CANCELLATION_ENABLE), this function sets up the required GPIO pins for the AEC process. 
 * AEC is crucial for voice call quality, as it helps remove the echo from the speaker that can be captured by the microphone.
 */
void app_gpio_aec_io_cfg(void)
{
    gpio_config_t io_conf;
    //disable interrupt
    io_conf.intr_type = GPIO_INTR_DISABLE;
    //set as output mode
    io_conf.mode = GPIO_MODE_OUTPUT;
    //bit mask of the pins that you want to set,e.g.GPIO18/19
    io_conf.pin_bit_mask = GPIO_OUTPUT_AEC_PIN_SEL;
    //disable pull-down mode
    io_conf.pull_down_en = 0;
    //disable pull-up mode
    io_conf.pull_up_en = 0;
    //configure GPIO with the given settings
    gpio_config(&io_conf);

    // set the output pins
    gpio_set_level(GPIO_OUTPUT_AEC_2, 0);

    gpio_set_level(GPIO_OUTPUT_AEC_1, 0);

    gpio_set_level(GPIO_OUTPUT_AEC_1, 1);

    gpio_set_level(GPIO_OUTPUT_AEC_3, 1);
}
#endif /* ACOUSTIC_ECHO_CANCELLATION_ENABLE */
