/*
  LoRaWAN.cpp for M5Stack (fork from https://github.com/toddkrein/OTAA-LoRaWAN-Seeed)

  2013 Copyright (c) Seeed Technology Inc.  All right reserved.
  2017 Copyright (c) Todd Krein. All rights reserved.
  2023 Copyright (c) Ramin Sangesari. All rights reserved.
  2023 Copyright (c) Andres Oliva Trevisan. All rights reserved.
  
  The MIT License (MIT)
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:
  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.
  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.1  USA
*/

#ifndef _LORAE5_H_
#define _LORAE5_H_
/*COMMAND LIST AND EXAMPLES*/
//https://files.seeedstudio.com/products/317990687/res/LoRa-E5%20AT%20Command%20Specification_V1.0%20.pdf
#include <Arduino.h>

#define SerialUSB  Serial

#if defined(ESP32)
	#define SerialLoRa Serial2		//M5Stack ESP32 Camera Module Development Board
#else
	#define SerialLoRa Serial1		//For SAMD Variant and XIAO NRF
#endif

#define AT_NO_ACK "NO_ACK"  //For not checking the command response in order to send a command error
/*Define to print to the USER into UART terminal the commands messages sended and response recieved */
#define COMMAND_PRINT_TO_USER
/*Define to meassure and print */
#define COMMAND_TIME_MEASURE

#define _DEBUG_SERIAL_ 0
//#define DEFAULT_TIMEOUT 5000 // msecond
#define DEFAULT_TIMEOUT   3000  // msecond
#define DEFAULT_TIMEWAIT \
    100  // milliseconds to wait after issuing command via serial
#define DEFAULT_DEBUGTIME \
    500  // milliseconds to wait for a response after command

// #define BATTERY_POWER_PIN    A4
// #define CHARGE_STATUS_PIN    A5

#define BEFFER_LENGTH_MAX 512   //reception buffer size. Commands response can be up to 400 bytes according to data sheet examples

#define MAC_COMMAND_FLAG "MACCMD:"
#define kLOCAL_BUFF_MAX  64

enum _class_type_t { CLASS_A = 0, CLASS_C };
enum _physical_type_t {
    UNINIT = -1,
    EU434,
    EU868,
    US915,
    US915HYBRID,
    AU915,
    AS923,
    CN470,
    KR920,
    CN470PREQUEL,
    STE920,
    UNDEF
};
enum _device_mode_t { LWABP = 0, LWOTAA, TEST };
enum _otaa_join_cmd_t { JOIN = 0, FORCE };
enum _window_delay_t {
    RECEIVE_DELAY1 = 0,
    RECEIVE_DELAY2,
    JOIN_ACCEPT_DELAY1,
    JOIN_ACCEPT_DELAY2
};
enum _band_width_t { BW125 = 125, BW250 = 250, BW500 = 500 };
enum _spreading_factor_t {
    SF12 = 12,
    SF11 = 11,
    SF10 = 10,
    SF9  = 9,
    SF8  = 8,
    SF7  = 7
};
enum _data_rate_t {
    DR0 = 0,
    DR1,
    DR2,
    DR3,
    DR4,
    DR5,
    DR6,
    DR7,
    DR8,
    DR9,
    DR10,
    DR11,
    DR12,
    DR13,
    DR14,
    DR15
};

/*****************************************************************
Type    DataRate    Configuration   BitRate| TxPower Configuration
EU434   0           SF12/125 kHz    250    | 0       10dBm
        1           SF11/125 kHz    440    | 1       7 dBm
        2           SF10/125 kHz    980    | 2       4 dBm
        3           SF9 /125 kHz    1760   | 3       1 dBm
        4           SF8 /125 kHz    3125   | 4       -2dBm
        5           SF7 /125 kHz    5470   | 5       -5dBm
        6           SF7 /250 kHz    11000  | 6:15    RFU
        7           FSK:50 kbps     50000  |
        8:15        RFU                    |
******************************************************************
Type    DataRate    Configuration   BitRate| TxPower Configuration
EU868   0           SF12/125 kHz    250    | 0       20dBm
        1           SF11/125 kHz    440    | 1       14dBm
        2           SF10/125 kHz    980    | 2       11dBm
        3           SF9 /125 kHz    1760   | 3       8 dBm
        4           SF8 /125 kHz    3125   | 4       5 dBm
        5           SF7 /125 kHz    5470   | 5       2 dBm
        6           SF7 /250 kHz    11000  | 6:15    RFU
        7           FSK:50 kbps     50000  |
        8:15        RFU                    |
******************************************************************
Type    DataRate    Configuration   BitRate| TxPower Configuration
US915   0           SF10/125 kHz    980    | 0       30dBm
        1           SF9 /125 kHz    1760   | 1       28dBm
        2           SF8 /125 kHz    3125   | 2       26dBm
        3           SF7 /125 kHz    5470   | 3       24dBm
        4           SF8 /500 kHz    12500  | 4       22dBm
        5:7         RFU                    | 5       20dBm
        8           SF12/500 kHz    980    | 6       18dBm
        9           SF11/500 kHz    1760   | 7       16dBm
        10          SF10/500 kHz    3900   | 8       14dBm
        11          SF9 /500 kHz    7000   | 9       12dBm
        12          SF8 /500 kHz    12500  | 10      10dBm
        13          SF7 /500 kHz    21900  | 11:15   RFU
        14:15       RFU                    |
*******************************************************************
Type    DataRate    Configuration   BitRate| TxPower Configuration
CN780   0           SF12/125 kHz    250    | 0       10dBm
        1           SF11/125 kHz    440    | 1       7 dBm
        2           SF10/125 kHz    980    | 2       4 dBm
        3           SF9 /125 kHz    1760   | 3       1 dBm
        4           SF8 /125 kHz    3125   | 4       -2dBm
        5           SF7 /125 kHz    5470   | 5       -5dBm
        6           SF7 /250 kHz    11000  | 6:15    RFU
        7           FSK:50 kbps     50000  |
        8:15        RFU                    |
******************************************************************/

class LoRaE5Class {
   public:
    LoRaE5Class(void);

    /**
     *  \brief Initialize the conmunication interface
     *
     *  \return Return null
     */
    void init(void);

    /**
     *  \brief Read the version from device
     *
     *  \param [in] *buffer The output data cache
     *  \param [in] length The length of data cache
     *  \param [in] timeout The over time of read
     *
     *  \return Return null.
     */
     
	void init(uint8_t rx, uint8_t tx);
    /**
      * \Allows the user to send a command 
      * 
      * \param [in] *p_ack: pointer to Expected response from to indicate a proper execution of the command
      * \param [in] *timeout_ms: timeout to expect
      * \param [in] *p_cmd: pointer to command to send
      *
      * \return time in ms to send the command and recieve response
      * Example: at_send_check_response("+ID: AppEui", 1000, "AT+ID\r\n");
      * What is does?: Sends command "AT+ID\r\n\" to module
      * Will end execution if recieves "+ID: AppEui" or 1000 has passed
     */
  int at_send_check_response(char*p_cmd, char *p_ack, int timeout_ms,char*p_response);
    /**
     *  \brief Read the version from device
     *
     *  \param [in] *buffer The output data cache
     *  \param [in] timeout The over time of read
     *
     *  \return Return null.
     */
    
    int getVersion(char *buffer,
                    unsigned int timeout = DEFAULT_TIMEOUT);

    /**
     *  \brief Read the ID from device
     *
     *  \param [in] *buffer The output data cache
     *  \param [in] timeout The over time of read
     *
     *  \return Return null.
     */
    int getId(char *buffer,
               unsigned int timeout = DEFAULT_TIMEOUT);

    /**
     *  \brief Set the ID
     *
     *  \param [in] *DevAddr The end-device address
     *  \param [in] *DevEUI The end-device identifier
     *  \param [in] *AppEUI The application identifier
     *
     *  \return Return null.
     */
    int setId(char *DevAddr, char *DevEUI, char *AppEUI);

    /**
     *  \brief Set the key
     *
     *  \param [in] *NwkSKey The network session key
     *  \param [in] *AppSKey The application session key
     *  \param [in] *AppKey The Application key
     *
     *  \return Return null.
     */
    int setKey(char *NwkSKey, char *AppSKey, char *AppKey);

    /**
     *  \brief Set the data rate
     *
     *  \param [in] dataRate The date rate of encoding
     *  \param [in] physicalType The type of ISM
     *
     *  \return Return null.
     */
    int setDataRate(_data_rate_t dataRate         = DR0,
                     _physical_type_t physicalType = EU434);

    /**
     *  \brief ON/OFF adaptive data rate mode
     *
     *  \param [in] command The date rate of encoding
     *
     *  \return Return null.
     */
    int setAdaptiveDataRate(bool command);

    /**
     *  \brief Set the output power
     *
     *  \param [in] power The output power value
     *
     *  \return Return elapsed time to send command (+get response if corresponds) in mili seconds.
     */
    int setPower(short power);

    /**
     *  \brief Set the port number
     *
     *  \param [in] port The port number, range from 1 to 255
     *
     *  \return Return null.
     */
    int setPort(unsigned int port);

    /**
     *  \brief Set the channel parameter
     *
     *  \param [in] channel The channel number, range from 0 to 15
     *  \param [in] frequency The frequency value
     *
     *  \return Return null.
     */
    int getChannel(void);
    int setChannel(unsigned char channel, float frequency);
    /**
     *  \brief Set the channel parameter
     *
     *  \param [in] channel The channel number, range from 0 to 15
     *  \param [in] frequency The frequency value. Set frequecy zero to disable
     * one channel \param [in] dataRata The date rate of channel
     *
     *  \return Return null.
     */
    int setChannel(unsigned char channel, float frequency,
                    _data_rate_t dataRata);
    /**
     *  \brief Set the channel parameter
     *
     *  \param [in] channel The channel number, range from 0 to 15
     *  \param [in] frequency The frequency value
     *  \param [in] dataRataMin The minimum date rate of channel
     *  \param [in] dataRataMax The maximum date rate of channel
     *
     *  \return Return null.
     */
    int setChannel(unsigned char channel, float frequency,
                    _data_rate_t dataRataMin, _data_rate_t dataRataMax);

    /**
     *  \brief Transfer the data
     *
     *  \param [in] *buffer The transfer data cache
     *  \param [in] timeout The over time of transfer
     *
     *  \return Return bool. Ture : transfer done, false : transfer failed
     */
    int transferPacket(char *buffer, unsigned int timeout = DEFAULT_TIMEOUT);
    /**
     *  \brief Transfer the data
     *
     *  \param [in] *buffer The transfer data cache
     *  \param [in] length The length of data cache
     *  \param [in] timeout The over time of transfer
     *
     *  \return Return bool. Ture : transfer done, false : transfer failed
     */
    int transferPacket(unsigned char *buffer, unsigned char length,
                        unsigned int timeout = DEFAULT_TIMEOUT);
    /**
     *  \brief Transfer the packet data
     *
     *  \param [in] *buffer The transfer data cache
     *  \param [in] timeout The over time of transfer
     *
     *  \return Return bool. Ture : Confirmed ACK, false : Confirmed NOT ACK
     */
    int transferPacketWithConfirmed(char *buffer,
                                     unsigned int timeout = DEFAULT_TIMEOUT);
    /**
     *  \brief Transfer the data
     *
     *  \param [in] *buffer The transfer data cache
     *  \param [in] length The length of data cache
     *  \param [in] timeout The over time of transfer
     *
     *  \return Return bool. Ture : Confirmed ACK, false : Confirmed NOT ACK
     */
    int transferPacketWithConfirmed(unsigned char *buffer,
                                     unsigned char length,
                                     unsigned int timeout = DEFAULT_TIMEOUT);

    /**
     *  \brief Receive the data
     *
     *  \param [in] *buffer The receive data cache
     *  \param [in] length The length of data cache
     *  \param [in] *rssi The RSSI cache
     *
     *  \return Return Receive data number
     */
    short receivePacket(char *buffer, short length, short *rssi);

    /**
     *  \brief Transfer the proprietary data
     *
     *  \param [in] *buffer The transfer data cache
     *  \param [in] timeout The over time of transfer
     *
     *  \return Return bool. Ture : transfer done, false : transfer failed
     */
    int transferProprietaryPacket(char *buffer,
                                   unsigned int timeout = DEFAULT_TIMEOUT);
    /**
     *  \brief Transfer the proprietary data
     *
     *  \param [in] *buffer The transfer data cache
     *  \param [in] length The length of data cache
     *  \param [in] timeout The over time of transfer
     *
     *  \return Return bool. Ture : transfer done, false : transfer failed
     */
    int transferProprietaryPacket(unsigned char *buffer, unsigned char length,
                                   unsigned int timeout = DEFAULT_TIMEOUT);

    /**
     *  \brief Set device mode
     *
     *  \param [in] mode The mode of device
     *
     *  \return Return null
     */
    int setDeviceMode(_device_mode_t mode);

    /**
     *  \brief Set device join a network
     *
     *  \param [in] command The type of join
     *  \param [in] timeout The over time of join
     *
     *  \return Return bool. True : join OK, false : join NOT OK
     */
    int setOTAAJoin(_otaa_join_cmd_t command,
                     unsigned int timeout = DEFAULT_TIMEOUT);

    /**
     *  \brief Set message unconfirmed repeat time
     *
     *  \param [in] time The repeat time, range from 1 to 15
     *
     *  \return Return null
     */
    int setUnconfirmedMessageRepeatTime(unsigned char time);

    /**
     *  \brief Set message retry times time
     *
     *  \param [in] time The retry time, range from 0 to 254
     *
     *  \return Return null
     */
    int setConfirmedMessageRetryTime(unsigned char time);

    /**
     *  \brief ON/OFF receice window 1
     *
     *  \param [in] command The true : ON, false OFF
     *
     *  \return Return null
     */
    int getReceiveWindowFirst(void);
    int setReceiveWindowFirst(bool command);
    /**
     *  \brief Set receice window 1 channel mapping
     *
     *  \param [in] channel The channel number, range from 0 to 71
     *  \param [in] frequency The frequency value of channel
     *
     *  \return Return null
     */
    int setReceiveWindowFirst(unsigned char channel, float frequency);

    /**
     *  \brief Set receice window 2 channel mapping
     *
     *  \param [in] frequency The frequency value of channel
     *  \param [in] dataRate The date rate value
     *
     *  \return Return null
     */
    int setReceiveWindowSecond(float frequency, _data_rate_t dataRate);
    /**
     *  \brief Set receice window 2 channel mapping
     *
     *  \param [in] frequency The frequency value of channel
     *  \param [in] spreadingFactor The spreading factor value
     *  \param [in] bandwidth The band width value
     *
     *  \return Return null
     */
    int setReceiveWindowSecond(float frequency,
                                _spreading_factor_t spreadingFactor,
                                _band_width_t bandwidth);

    /**
     *  \brief Set receice window delay
     *
     *  \param [in] command The delay type
     *  \param [in] _delay The delay value(millisecond)
     *
     *  \return Return null
     */
    int setReceiveWindowDelay(_window_delay_t command, unsigned short _delay);

    /**
     *  \brief Set LoRaWAN class type
     *
     *  \param [in] type The class type
     *
     *  \return Return null
     */
    int setClassType(_class_type_t type);

    /**
     *  \brief Set device into low power mode
     *
     *  \return Return null
     */
    int setDeviceLowPower(void);
    
    /**
     *  \brief Wakes up the device
     *
     *  \return Return null
     */
    int setDeviceWakeUp(void);

    /**
     *  \brief Reset device
     *
     *  \return Return null
     */
    int setDeviceReset(void);

    /**
     *  \brief Setup device default
     *
     *  \return Return null
     */
    int setDeviceDefault(void);

    /**
     *  \brief Initialize device into P2P mode
     *
     *  \param [in] frequency The ISM frequency value
     *  \param [in] spreadingFactor The spreading factor value
     *  \param [in] bandwidth The band width value
     *  \param [in] txPreamble The transfer packet preamble number
     *  \param [in] rxPreamble The receive packet preamble number
     *  \param [in] power The output power
     *
     *  \return Return null
     */
    int initP2PMode(unsigned short frequency            = 433,
                     _spreading_factor_t spreadingFactor = SF12,
                     _band_width_t bandwidth             = BW125,
                     unsigned char txPreamble = 8, unsigned char rxPreamble = 8,
                     short power = 20);
    /**
     *  \brief Transfer the data
     *
     *  \param [in] *buffer The transfer data cache
     *
     *  \return Return bool. Ture : transfer done, false : transfer failed
     */
    int transferPacketP2PMode(char *buffer);
    /**
     *  \brief Transfer the data
     *
     *  \param [in] *buffer The transfer data cache
     *  \param [in] length The length of data cache
     *
     *  \return Return bool. Ture : transfer done, false : transfer failed
     */
    int transferPacketP2PMode(unsigned char *buffer, unsigned char length);
    /**
     *  \brief Receive the data
     *
     *  \param [in] *buffer The receive data cache
     *  \param [in] length The length of data cache
     *  \param [in] *rssi The RSSI cache
     *  \param [in] timeout The over time of receive
     *
     *  \return Return Receive data number
     */
    short receivePacketP2PMode(unsigned char *buffer, short length, short *rssi,
                               unsigned int timeout = DEFAULT_TIMEOUT);

    /**
     *  \brief LoRaWAN raw data
     *
     *  \return Return null
     */
    void loraDebug(void);

    /**
     *  \brief Read battery voltage
     *
     *  \return Return battery voltage
     */
    // short getBatteryVoltage(void);

   private:
    char recv_buf[BEFFER_LENGTH_MAX];//reception buffer. Commands response can be up to 400 bytes according to data sheet examples
    char cmd[556];//store command to send
    char cmd_resp_ack[64];//store command response ACK to compare with string recieved and thus verify if the command worked. 
    #ifdef COMMAND_TIME_MEASURE
    char cmd_time[128];//store commands time response
    #endif
};

extern LoRaE5Class lora;

#endif
