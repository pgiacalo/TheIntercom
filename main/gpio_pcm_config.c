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
