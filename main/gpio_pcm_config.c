/*
 * SPDX-FileCopyrightText: 2021 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */

#include "bluetooth_config.h"
#include "driver/gpio.h"
#include "soc/gpio_reg.h"
#include "soc/gpio_sig_map.h"
#include "gpio_pcm_config.h"
#include "esp_rom_gpio.h"
#include "esp_log.h"

#define TAG     "DEBUG: "

// #define GPIO_LRC     (25)  //PHIL commented out these original pin assignments 
// #define GPIO_DOUT    (5)
// #define GPIO_BCLK    (26)
// #define GPIO_DIN     (35)

#if DEVICE_ROLE == ROLE_MASTER
    #define GPIO_DIN         (14)    //PHIL changed to these pin assignments (see my note below)
    #define GPIO_DOUT        (25)    
    #define GPIO_BCLK        (26)    //PHIL Bit Clock (1.4 MHz pulses)
    #define GPIO_LRC         (27)    //PHIL LRC - Left/Right Channel Select or Word Select (was called FRAME SYNC)
#elif DEVICE_ROLE == ROLE_SLAVE
    #define GPIO_DIN         (14)    //PHIL changed to these pin assignments (see my note below)
    #define GPIO_DOUT        (25)    
    #define GPIO_BCLK        (27)    //PHIL Bit Clock (1.4 MHz pulses)
    #define GPIO_LRC         (26)    //PHIL LRC - Left/Right Channel Select or Word Select (was called FRAME SYNC)
#endif

/**************************************
//PHIL added this note about the I2S and I2S pin numbers

I2S (Inter-IC Sound) uses PCM (Pulse-Code Modulation) to encode the audio data being transmitted.

Here are the standard I2S pin numbers for the ESP32-WROOM-32D module.
The ESP32 has two I2S peripherals, so there are 2 sets of I2S pins.

    I2S0:
        I2S Data: GPIO25
        I2S Clock: GPIO26
        I2S WS (Word Select): GPIO27

    I2S1:
        I2S Data: GPIO32
        I2S Clock: GPIO33
        I2S WS: GPIO34
*************************************/

#define GPIO_OUTPUT_PCM_PIN_SEL  ((1ULL<<GPIO_LRC) | (1ULL<<GPIO_BCLK) | (1ULL<<GPIO_DOUT))

#define GPIO_INPUT_PCM_PIN_SEL (1ULL<<GPIO_DIN)

void app_gpio_pcm_io_cfg(void)
{
    gpio_config_t io_conf;
    /// configure the PCM output pins
    //disable interrupt
    io_conf.intr_type = GPIO_INTR_DISABLE;
    //set as output mode
    io_conf.mode = GPIO_MODE_OUTPUT;
    //bit mask of the pins that you want to set,e.g.GPIO18/19
    io_conf.pin_bit_mask = GPIO_OUTPUT_PCM_PIN_SEL;
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


#if DEVICE_ROLE == ROLE_MASTER
    // Master device sends data to the Slave
    ESP_LOGI(TAG, "USING MASTER INPUT AND OUTPUT PINS");

    esp_rom_gpio_connect_out_signal(GPIO_DOUT, PCMDOUT_IDX, false, false);           //output DOUT pin 25
    esp_rom_gpio_connect_out_signal(GPIO_BCLK, PCMCLK_OUT_IDX, false, false);        //output BCLK pin 26
    esp_rom_gpio_connect_out_signal(GPIO_LRC, PCMFSYNC_OUT_IDX, false, false);       //output LRC  pin 27
    // Master device listens to the input from the Slave
    esp_rom_gpio_connect_in_signal(GPIO_DIN, PCMDIN_IDX, false);                     //input DIN   pin 14
#elif DEVICE_ROLE == ROLE_SLAVE
    ESP_LOGI(TAG, "USING SLAVE INPUT AND OUTPUT PINS");
    // Slave device sends data to the Master
    esp_rom_gpio_connect_out_signal(GPIO_DOUT, PCMDOUT_IDX, false, false);           //output DOUT pin 25
    // The slave listens to the Master's clock, LRC select, and data
    esp_rom_gpio_connect_in_signal(GPIO_BCLK, PCMCLK_IN_IDX, false);                 //input BCLK pin 26 
    esp_rom_gpio_connect_in_signal(GPIO_LRC, PCMFSYNC_IN_IDX, false);                //input LRC  pin 27
    esp_rom_gpio_connect_in_signal(GPIO_DIN, PCMDIN_IDX, false);                     //input DIN  pin 14
#endif

}

#if ACOUSTIC_ECHO_CANCELLATION_ENABLE

#define GPIO_OUTPUT_AEC_1      (19)
#define GPIO_OUTPUT_AEC_2      (21)
#define GPIO_OUTPUT_AEC_3      (22)
#define GPIO_OUTPUT_AEC_PIN_SEL  ((1ULL<<GPIO_OUTPUT_AEC_1) | (1ULL<<GPIO_OUTPUT_AEC_2) | (1ULL<<GPIO_OUTPUT_AEC_3))

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
