# Grove-Wio-E5_advanced
Advanced application of SeedStudio module Grove-Wio-E5 based on chip STM32WLE5JC from STM connected to an Arduino Nano 33 BLE Sense board.
### Grove-Wio-E5 commands list:
https://files.seeedstudio.com/products/317990687/res/LoRa-E5%20AT%20Command%20Specification_V1.0%20.pdf
## Features:
This project features a LoRa library in order to perform a complete configuration of the Grove-Wio-E5 according to the user needs.
This LoRa library is a modified version of the Ramin Sangesari (https://github.com/idreamsi/LoRaE5) with some rework in order to optimize the performance and add some useful functionalities. Is important to know that Ramin Sangesari's work supports P2P communication between LoRa node devices (this means that communications happen directly between both end node devices without any Gateway involved in the communication).
### Key features added:
* Allows to make the use of SoftwareSerial (or Software UART) on boards like Arduino Nano 33BLE by using macros. This is useful if you are using a Grove shield that uses your main serial and you need to use other pins of your board as a Serial interface to communicate with the Grove-Wio-E5 module.
* Allows to display commands sent, the responses from the Grove-Wio-E5 and enable the print them. Also allows the device to measure the time it took to both send the command and receive a response from Grove-Wio-E5 (and gateway ACK if there is a need for that)
* You can enable or disable the printing of the command messages. This means that you can run the code with your debugging serial terminal disabled
* Allows the use of the different debug modes (DEBUG/INFO/WARN/ERROR/FATAL/PANIC/QUIET).
* Allows to estimate the time that is going to take to send a message based on your selected DR and Frequency Band (like EU868).
* Added examples of how to use the low power mode of the device and added a wakeUp function.
* Added an example to change the DevEUI of the device in order to make the testing of different codes simpler when interacting with a LoRa Gateway.
## Hardware Setup:
Same as featured in https://github.com/andresoliva/Grove-Wio-E5 . But in this case you
##
AppKey value: 2B7E151628AED2A609CF4F3CABF71588
## LoRa library important configuration:
#### LoRa.h Serial port selection
```html
/*If you want to use other pins as serial interfaces in boards like Arduino Nano BLE33 which does not support the
"SerialLoRa.begin(9600, SERIAL_8N1, rx, tx)" function call, enable this: */
#define CUSTOM_LORA_SERIAL
#define CUSTOM_LORA_SERIAL_TX_PIN A4 //example for using other pins as Tx    
#define CUSTOM_LORA_SERIAL_RX_PIN A5 //examples for using other pins as Rx
```
#### LoRa.h Serial port selection
```html
/*--------------PRINT TIME --------------------------**/
/*Define to print to the USER into UART terminal the commands messages sent and response received */
#define COMMAND_PRINT_TO_USER
/*Define to print the result of times measures
Important Note: This time is measured using because of this, this is only an estimation
Regarding Transmition time: it was tested and it cannot be measured properly using this method
What you get instead is the transmission time + the time to get ACK from the gateway. Because the transmission time 
is included, if you subtract the times of two transmissions with different payloads, it will be the subtraction
of the transmission times. In this way, you can compare the times changes due to the payload size and know what to spect*/
#define COMMAND_PRINT_TIME_MEASURE
```
## Example brief explanation:
The implemented example consists of:
* A complete set-up of the device in order to work with a EU868 LoRa Gateway in OTA mode. So you must add the DevEUI and settled AppKey of your device to the gateway in order to work like it was done in SeedStudio's original example (https://wiki.seeedstudio.com/Grove_LoRa_E5_New_Version/#ttn-console-configuration-setup).
* The device will not send any packet until it joins the network. And this will only happen if you set your device properly to communicate with your Gateway by selecting the same Frequency band and configuring in the Gateway the DevEUI and settled AppKey.
* A loop transmission of one string packet and two hex packet
* A comparison between the expected differences in time
* Entering Low Power and Wake up functions to optimize performance.
### Initial setup expected responses
#### Initial set Up
```html
--------Command sent:
AT+DR= EU868
--------Command responses:
+DR: EU868
--------End of Commands responses
Total Command Time + Time to get ACK response: 107 ms.
--------Command sent:
AT+DR=4
--------Command responses:
+DR: DR4
+DR: EU868 DR4  SF8  BW125K 
--------Command sent:
AT+CLASS=A
--------Command responses:
+CLASS: A
--------End of Commands responses
Total Command Time + Time to get ACK response: 32 ms.
--------Command sent:
AT+PORT=8
--------Command responses:
+PORT: 8
--------End of Commands responses
Total Command Time + Time to get ACK response: 10 ms.
--------Command sent:
AT+POWER=14
--------Command responses:
+POWER: 14
--------End of Commands responses
Total Command Time + Time to get ACK response: 13 ms.
--------Command sent:
AT+CH=2
--------Command responses:
+CH: 2
--------End of Commands responses
Total Command Time + Time to get ACK response: 9 ms.
--------Command sent:
AT+ADR=OFF
--------Command responses:
+ADR: OFF
--------End of Commands responses
Total Command Time + Time to get ACK response: 18 ms.
```
#### Join failed
```html
--------Command sent:
AT+JOIN
--------Command responses:
+JOIN: Start
+JOIN: NORMAL
+JOIN: Join failed
+JOIN: Done
--------End of Commands responses
!!Command Failed!! Did not get the expected "Ok" or "ACK" response from E5 module after sending the command.
```
#### Join ok
```html
--------Command sent:
AT+JOIN
--------Command responses:
+JOIN: Start
+JOIN: NORMAL
+JOIN: Network joined
--------End of Commands responses
Total Command Time + Time to get ACK response: 6585 ms.
```
#### Change device address
```html
--------Command sent:
AT+KEY= APPKEY,"2B7E151628AED2A609CF4F3CABF71588"
--------Command responses:
+KEY: APPKEY 2B7E151628AED2A609CF4F3CABF71588
--------End of Commands responses
Total Command Time + Time to get ACK response: 112 ms.
```
#### Send String packet and waits for ACK
```html
Sending 44 characters to a LoRa Gateway and waits for ACK
--------Command sent:
AT+CMSG="I am sending this message to a LoRa Gateway."
--------Command responses:
+CMSG: Start
+CMSG: Wait ACK
+CMSG: ACK Received
+CMSG: RXWIN1, RSSI -82, SNR 10.0
+CMSG: Done
--------End of Commands responses
Time to Transmit message and Recieve ACK from TX message: 1277 ms.
Total Command Time + Time to get ACK response: 1376 ms.
```
#### Send Binary packet and waits for ACK
```html
Sending 10 bytes to a LoRa Gateway and waits for ACK
--------Command sent:
AT+CMSGHEX="00010203040506070809"
--------Command responses:
+CMSGHEX: Start
+CMSGHEX: Wait ACK
+CMSGHEX: ACK Received
+CMSGHEX: RXWIN1, RSSI -82, SNR 10.0
+CMSGHEX: Done
--------End of Commands responses
Time to Transmit message and Recieve ACK from TX message: 1192 ms.
Total Command Time + Time to get ACK response: 1303 ms.
```
#### DEBUG ON Send Binary packet and waits for ACK
```html
Sending 10 bytes to a LoRa Gateway and waits for ACK
--------Command sent:
AT+CMSGHEX="00010203040506070809"
--------Command responses:
+LOG:  WARN  7729182 LW      tx 8, 00010203040506070809(10)
+CMSGHEX: Start
+CMSGHEX: Wait ACK
+LOG: DEBUG  7729183 LW      ch 00
+LOG: DEBUG  7729183 LORA    TX, 868100000, SF8, 125KHz, 8, 14
+LOG: DEBUG  7729183 LORA    TX, 8080E43806000A00087F55A49CBBA871DA154AFF294815
+LOG: DEBUG  7730298 LORA    RX, 868100000, SF8, 125KHz, 14
+LOG: DEBUG  7730386 LORA    RX, 6080E43806A00A00672972FA, -88, 10
+CMSGHEX: ACK Received
+CMSGHEX: RXWIN1, RSSI -88, SNR 10.0
+CMSGHEX: Done
--------End of Commands responses
Time to Transmit message and Recieve ACK from TX message: 1199 ms.
Total Command Time + Time to get ACK response: 1374 ms.
```
#### Enable time comparison between 10 bytes and 50 bytes messages
```html
Estimated transmission time of messages with 10 and 50 bytes as payload: 108.0 ms, 210.4 ms.
Calculated time difference between transmitting 10 and 50 bytes as payload: 102.4 ms.
Measured difference between transmitting with 10 and 50 bytes as payload: 102.4 ms.
```
## To test
  * Meassure power consumption in low power mode.
  * Re-test P2P communication after library rework
