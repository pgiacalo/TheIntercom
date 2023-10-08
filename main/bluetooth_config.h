#ifndef BLUETOOTH_CONFIG_H
#define BLUETOOTH_CONFIG_H

// #include "sdkconfig.h"

//PHIL added this config file to set the role and the mac addresses

// Define your settings here
#define ROLE_MASTER     0
#define ROLE_SLAVE      1
/**********************************/
#define DEVICE_ROLE     ROLE_MASTER  	// CHANGE THIS AS NEEDED
/**********************************/

// MAC Addresses of bluetooth earbuds
#if DEVICE_ROLE == ROLE_MASTER
    #define HF_PEER_ADDR {0x58, 0xFC, 0xC6, 0x6C, 0x0A,0x03}		//TOZO Mazda earbuds
#elif DEVICE_ROLE == ROLE_SLAVE
    #define HF_PEER_ADDR {0x54, 0xB7, 0xE5, 0x8C, 0x07,0x71}		//TOZO Home earbuds
    // #define HF_PEER_ADDR {0x74, 0x74, 0x46, 0xED, 0x07, 0x6B}		//Pixel Buds Series A  
#endif


// //setup the ESP32 target in menuconfig
// #if CONFIG_IDF_TARGET_ESP32         

    // Pin configurations for ESP32 Master
    #define MASTER_GPIO_DIN         14
    #define MASTER_GPIO_DOUT        25
    #define MASTER_GPIO_BCLK        26  //was 5 (green)
    #define MASTER_GPIO_LRC         27  //was 4 (blue)

    // Pin configurations for ESP32 Slave
    #define SLAVE_GPIO_DOUT         25
    #define SLAVE_GPIO_DIN          14
    #define SLAVE_GPIO_BCLK         27  //was 17 (green)
    #define SLAVE_GPIO_LRC          26  //was 19 (blue)

// #elif CONFIG_IDF_TARGET_ESP32S3

//     // Pin configurations for ESP32-S3 Master (Modify pins as needed)
//     #define MASTER_GPIO_DIN         /* Your S3 Pin */
//     #define MASTER_GPIO_DOUT        /* Your S3 Pin */
//     #define MASTER_GPIO_BCLK        /* Your S3 Pin */
//     #define MASTER_GPIO_LRC         /* Your S3 Pin */

//     // Pin configurations for ESP32-S3 Slave (Modify pins as needed)
//     #define SLAVE_GPIO_DOUT         /* Your S3 Pin */
//     #define SLAVE_GPIO_DIN          /* Your S3 Pin */
//     #define SLAVE_GPIO_BCLK         /* Your S3 Pin */
//     #define SLAVE_GPIO_LRC          /* Your S3 Pin */

// #else
//     #error "Unsupported IDF target (see bluetooth_config.h)"
// #endif 

#endif // BLUETOOTH_CONFIG_H


/**************************************
//PHIL added this note about the "standard" ESP32 I2S pin numbers (but pin choices are flexible)

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
