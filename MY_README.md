Oct 3, 2023

Overview
--------
This project started with the latest code from one of Espressif ESP-IDF ESP32 examples.

Specifically, it began with example code provided in Espressif's esp-idf development environment. 
The code example is called "Hands Free Protocol Audio Gateway" (hfp_ag). 

The directory path to the example is given below. Note that it is found under bluetooth/bluedroid. 
	esp/esp-idf/examples/bluetooth/bluedroid/classic_bt/hfp_ag 

The term "Gateway" got me interested in exploring this example, since it implies code that might 
enable a central point for bluetooth communication (which is my goal). 

The goal of the code modifications is to try to enable bi-directional bluetooth communication
using consumer bluetooth earbuds. In other words, using bluetooth earbuds to create a wireless
intercom over short distances. 

Envisioned Architecture
-----------------------
In the scenario envisioned, two ESP32s acting as wireless hubs -- each ESP32 connecting via bluetooth to a single set of wireless earbuds. 
One ESP32 will act as the Master and the other ESP32 will act as the Slave.

In order to transfer the audio data to/from one set of earbuds to the other, the ESP32s 
will be wired together using pins assigned as follows:

	ESP32 Master
	------------
 	SD_OUT 		Pin 25 (serial data output)
 	BCLK_OUT	Pin 26 (bit clock)
 	LRC_OUT 	Pin 27 (left/right channel select)
 	SD_IN 		Pin 14 (serial data input - from the ESP32 Slave)

	ESP32 Slave
	------------
 	SD_OUT 		Pin 25 (serial data output)
 	BCLK_IN		Pin 26 (bit clock - from the ESP32 Master)
 	LRC_IN 		Pin 27 (left/right channel select - from the ESP32 Master)
 	SD_IN 		Pin 14 (serial data input - from the ESP32 Master)

The wireless and wired connections are pictured below:

					ESP32 Master			ESP32 Slave
					------------			-----------
				|	SD_IN 		<---------- SD_OUT	|
Bluetooth		| 	SD_OUT 		----------> SD_IN	|	  	  Bluetooth
Earbuds	<.....> | 	BCLK_OUT	----------> BCLK_IN	| <.....> Earbuds	
				| 	LRC_OUT 	----------> LRC_IN	|

To be continued...

