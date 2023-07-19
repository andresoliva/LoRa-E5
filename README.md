# Grove-Wio-E5_advanced
Advanced application of SeedStudio module Grove-Wio-E5 based on chip STM32WLE5JC from STM connected to an Arduino Nano 33 BLE Sense board. Works with any Arduino that supports UART
### Grove-Wio-E5 commands list:
https://files.seeedstudio.com/products/317990687/res/LoRa-E5%20AT%20Command%20Specification_V1.0%20.pdf
## Features:
This project features a Library for allowing the configuration and 
This library is a modified version of the Ramin Sangesari (https://github.com/idreamsi/LoRaE5) with some rework in order to omptimize the performance. Now the functions will only attempt to read
-Allows to display commands sent, the responses from the Grove-Wio-E5 and enable the print
-Allows measuring the time it took the device to both send the command and recieves a command response from Grove-Wio-E5 (and gateway ACK if there is a need for that) 
-Allows sending both messages
-Allows P2P communication between LoRa node devices (this means that communications happens directly between both end node devices without any Gateway involved in the communication)
## Hardware Setup:
Same as featured in https://github.com/andresoliva/Grove-Wio-E5
## To test
  * Meassure power consumption in low power mode.
  * Test why the adaptative rate is changing when communicating with Gateway(I set it )
  * Re test P2P communication after library rework
