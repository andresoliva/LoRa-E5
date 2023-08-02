/*
  LoRa-E5 (fork from https://github.com/idreamsi/LoRaE5)

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

#ifndef _LORA-E5_H_
#define _LORA-E5_H_
/*COMMAND LIST AND EXAMPLES*/
//https://files.seeedstudio.com/products/317990687/res/LoRa-E5%20AT%20Command%20Specification_V1.0%20.pdf
/*--------------PRINT TIME --------------------------**/
/*Define to print to the USER into UART terminal the commands messages sent and response received */
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

/*defines dependensies to avoid a CRASH of the program*/
#ifdef COMMAND_PRINT_TIME_MEASURE
  #ifndef COMMAND_PRINT_TO_USER
    #define COMMAND_PRINT_TO_USER
  #endif
#endif  
/*SERIAL PORT DEFINITION BASED ON PLATFORM**/
#include <Arduino.h>
#ifdef COMMAND_PRINT_TO_USER
 #define SerialUSB  Serial
#endif 
/*If you are not using Custom Serial, make this define */
  #if defined(ESP32)
    #define SerialLoRa_native Serial2    //M5Stack ESP32 Camera Module Development Board
  #else
    #define SerialLoRa_native Serial1    //For SAMD Variant and XIAO NRF
  #endif

#define DEFAULT_TIMEOUT   3000  //milliseconds to max wait for a command to get a response
#define DEFAULT_TIMEWAIT  100  //DO NOT CHANGE: milliseconds to wait after issuing command via serial and not getting an specific response

#define AT_NO_ACK "NO_ACK"  //For not checking the command response in order to send a command error
/*PARAMETERS FIDEX*/
//*******************************//
#define BEFFER_LENGTH_MAX 512   //reception buffer size. Commands response can be up to 400 bytes according to data sheet examples
#define MAC_COMMAND_FLAG "MACCMD:"
#define kLOCAL_BUFF_MAX  64
#define RXWIN1_DELAY  1000  //DO NOT CHANGE: milliseconds to wait after a transmition is being made in order to open RXWIN1 for reception
#define RXWIN2_DELAY  2000  //DO NOT CHANGE: milliseconds to wait after a transmition is being made in order to open RXWIN1 for reception

enum _deviceID{
   DevAddr=0,
   DevEui,
   AppEui};
/*All this values listed are the supported by the module*/   
#define LORA_BAUDRATE_DEFAULT BR_9600  

enum _baudrate_bps_supported{
      BR_9600=9600, /*9600 default value*/
	  //BR_14400=14400, //COMMENTED DUE TO NO SUPPORT WITH INIT. SEE LoRaE5Class::init for more info
	  //BR_19200=19200,//COMMENTED DUE TO NO SUPPORT WITH INIT. SEE LoRaE5Class::init for more info
	  BR_38400=38400,
	 // BR_57600=57600,//COMMENTED DUE TO NO SUPPORT WITH INIT. SEE LoRaE5Class::init for more info
	 // BR_76800=76800,//COMMENTED DUE TO NO SUPPORT WITH INIT. SEE LoRaE5Class::init for more info
	  BR_115200=115200,
	 // BR_230400=230400//COMMENTED DUE TO NO SUPPORT WITH INIT. SEE LoRaE5Class::init for more info
	  };
enum _debug_level{
   lora_DEBUG=0,
   lora_INFO,
   lora_WARN,
   lora_ERROR,
   lora_FATAL,
   lora_PANIC,
   lora_QUIET};
enum _class_type_t { CLASS_A = 0, CLASS_C };
enum _physical_type_t {
    UNINIT = -1,
    EU434,
    EU868,
    US915,
    US915HYBRID,
    US915OLD,
    AU915,
    AS923,
    CN470,
    CN779,
    KR920,
    CN470PREQUEL,
    STE920,
    IN865,
    RU864,
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
enum _band_width_t { 
    BWX = 0, 
    BW50kbps = 50, 
	BW125 = 125, 
	BW250 = 250, 
	BW500 = 500 
};
enum _spreading_factor_t {
    SF12 = 12,
    SF11 = 11,
    SF10 = 10,
    SF9  = 9,
    SF8  = 8,
    SF7  = 7,
	SFX  = 0
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
    DR15,
    DRNONE /*USED FOR KNOWING THAT DATA RATE WAS NOT SET. DO NOT REMOVE*/
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
/* ********************************
Power consumption data sheet valyes
  *Only 60uA in sleep mode. To meassure
  *Ultra-low Power Consumption: as low as 2.1uA sleep current (WOR mode)
  *datasheet info Active-mode RX: 4.82 mA
  *Active-mode TX PA=0:      15 mA at 10 dBm and 87 mA at 20 dBm (LoRa ® 125 kHz)
  *Active-mode TX PA=0:490 : 13 mA at 10 dBm, 21 mA at 14 dBm and 72 mA at 20 dBm
  *Active-mode TX PA=0:868 :17.5mA at 10 dBm, 26 mA at 14 dBm and 92 mA at 20 dBm
Power consumption table
source: https://forum.seeedstudio.com/t/lora-e5-fw-bug-tx-eu868-85ma-too-high-consumption/260126/3
| LoRaWAN power | PHY tx power, dBm | ST firmware, mA | Patched, mA | Patched==firmware upgrade
| ------------: | ----------------: | --------------: | ----------: |
|             0 |                14 |              86 |          60 |
|             1 |                12 |              77 |          59 |
|             2 |                10 |              70 |          46 |
|             3 |                 8 |              63 |          44 |
|             4 |                 6 |              59 |          42 |
|             5 |                 4 |              52 |          39 |
|             6 |                 2 |              46 |          36 |
|             7 |                 0 |              41 |          32 |
*/
/*measures done using the default */
#define    SLEEPPOWER_mA    0.021 //measured: power consuption when module is feed with 3.3V and is in sleep mode
#define    RXPOWER_mA       5.65 //measured mA when LoRa-E5 module is working as a receptor
#define    TXPOWER_00dBm_mA 41.0  //NOT MEASURED. STIMATED  based on table
#define    TXPOWER_02dBm_mA 41.5 //meassured 41.5 mA at 868 Mhz
#define    TXPOWER_04dBm_mA 48.1 //meassured 48.1 mA at 868 Mhz
#define    TXPOWER_06dBm_mA 53.5 //meassured 53.5 mA at 868 Mhz
#define    TXPOWER_08dBm_mA 60.6 //meassured 60.6 mA at 868 Mhz
#define    TXPOWER_10dBm_mA 68.3 //meassured 68.3 mA at 868 Mhz
#define    TXPOWER_12dBm_mA 77.3 //meassured 77.3 mA at 868 Mhz
#define    TXPOWER_14dBm_mA 86.8 //meassured 86.8 mA at 868 Mhz
#define    TXPOWER_16dBm_mA 86.8 //Module says that 16 dBm were set up properlty, meassured 86.8 mA at 868 Mhz. Not checked if the effective TX 


/****************************************************************************
 ******************LORA CLASS DEFINITION*************************************
 ****************************************************************************
 ****************************************************************************/
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
     
	void init(uint8_t tx, uint8_t rx);
    /**
      * \Allows the user to read the serial buffer
      * 
      * \param [in] *buffer: pointer to buffer used to store characters
      * \param [in] *length: amounts to store into desired buffer.Must no excede tyhesize of the buffer
      * \param [in] *p_cmd: timeout_ms: timeout
      *
      * \return the amounts of bytes readed*/
  unsigned int readBuffer(char* buffer, unsigned int length, unsigned int timeout_ms);
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
  unsigned int at_send_check_response(char*p_cmd, char *p_ack, unsigned int timeout_ms,char*p_response);
    /**
     *  \brief Read the version from device
     *
     *  \param [in] *buffer The output data cache
     *  \param [in] timeout The over time of read
     *
     *  \return Return null.
     */
    
    unsigned int getVersion(char *buffer,
                    unsigned int timeout = DEFAULT_TIMEOUT);

    /**
     *  \brief Read the ID from device
     *
     *  \param [in] *buffer The output data cache
	 *  \param [in] Type of Id to get (DevAddr, DevEui, AppEui)
     *  \param [in] timeout The over time of read
     *
     *  \return Return null.
     */
    unsigned int getId(char *buffer,_deviceID id, unsigned int timeout=DEFAULT_TIMEWAIT);

    /**
     *  \brief Set the ID
     *
     *  \param [in] *DevAddr The end-device address
     *  \param [in] *DevEUI The end-device identifier
     *  \param [in] *AppEUI The application identifier
     *
     *  \return Return null.
     */
    unsigned int setId(char *DevAddr, char *DevEUI, char *AppEUI);

    /**
     *  \brief Set the key
     *
     *  \param [in] *NwkSKey The network session key
     *  \param [in] *AppSKey The application session key
     *  \param [in] *AppKey The Application key
     *
     *  \return Return null.
     */
    unsigned int setKey(char *NwkSKey, char *AppSKey, char *AppKey);
       /**
     *  \brief Get the 
     *
     *  \param [in] ptr to store the bits per seconds seted on the device
     *
     *  \return Return null.
     */
    unsigned int getbitRate(unsigned int* pbitRate,float* ptxHead_time);

      /**
     *  \brief Set the standard frequencyBand utilized
     *
     *  \param [in] physicalType The type of ISM
     *
     *  \return Return null.
     */
unsigned int setFrequencyBand(_physical_type_t physicalType);

   /**
     *  \brief Set the data rate
     *
     *  \param [in] dataRate The date rate of encoding
     *  \param [in] physicalType The type of ISM
     *
     *  \return Return null.
     */
    unsigned int setDataRate(_data_rate_t dataRate         = DR5,
                     _physical_type_t physicalType = EU868);
       /**
     *  \brief Set the data rate acoording to the SF, BW and Phys provided
     *
     *  \param [in] dataRate The date rate of encoding
     *  \param [in] physicalType The type of ISM
     *
     *  \return Return null.
     */
    unsigned int setSpreadFactor(_spreading_factor_t SF, _band_width_t BW,
                                   _physical_type_t physicalType);
    /**
     *  \brief ON/OFF adaptive data rate mode
     *
     *  \param [in] command The date rate of encoding
     *
     *  \return Return null.
     */
    unsigned int setAdaptiveDataRate(bool command);

    /**
     *  \brief Set the output power
     *
     *  \param [in] power The output power value
     *
     *  \return Return elapsed time to send command (+get response if corresponds) in mili seconds.
     */
    unsigned int setPower(short power);

    /**
     *  \brief Set the port number
     *
     *  \param [in] port The port number, range from 1 to 255
     *
     *  \return Return null.
     */
    unsigned int setPort(unsigned char port);

    /*check what channel is the supported by the system*/
    unsigned int getChannel(void);
        /**
     *  \brief Set the channel parameter
     *
     *  \param [in] channel The channel number, range from 0 to 15
     *  \param [in] frequency The frequency value
     *
     *  \return Return null.
     */
    unsigned int setChannel(unsigned char channel);
    /**
     *  \brief Set the channel parameter
     *
     *  \param [in] channel The channel number, range from 0 to 15
     *
     *  \return Return null.
     */
    unsigned int setChannel(unsigned char channel, float frequency);
    /**
     *  \brief Set the channel parameter
     *
     *  \param [in] channel The channel number, range from 0 to 15
     *  \param [in] frequency The frequency value. Set frequecy zero to disable
     * one channel \param [in] dataRata The date rate of channel
     *
     *  \return Return null.
     */
    unsigned int setChannel(unsigned char channel, float frequency,
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
    unsigned int setChannel(unsigned char channel, float frequency,
                    _data_rate_t dataRataMin, _data_rate_t dataRataMax);

    /**
     *  \brief Transfer the data
     *
     *  \param [in] *buffer The transfer data cache
     *  \param [in] timeout The over time of transfer
     *
     *  \return Return bool. Ture : transfer done, false : transfer failed
     */
    unsigned int transferPacket(char *buffer, unsigned int timeout = DEFAULT_TIMEOUT);
    /**
     *  \brief Transfer the data
     *
     *  \param [in] *buffer The transfer data cache
     *  \param [in] length The length of data cache
     *  \param [in] timeout The over time of transfer
     *
     *  \return Return bool. Ture : transfer done, false : transfer failed
     */
    unsigned int transferPacket(unsigned char *buffer, unsigned char length,
                        unsigned int timeout = DEFAULT_TIMEOUT);
    /**
     *  \brief Transfer the packet data
     *
     *  \param [in] *buffer The transfer data cache
     *  \param [in] timeout The over time of transfer
     *
     *  \return Return bool. Ture : Confirmed ACK, false : Confirmed NOT ACK
     */
    unsigned int transferPacketWithConfirmed(char *buffer,
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
    unsigned int transferPacketWithConfirmed(unsigned char *buffer,
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
    short receivePacket(char *buffer, short length, short *rssi,unsigned int timeout);

    /**
     *  \brief Transfer the proprietary data
     *
     *  \param [in] *buffer The transfer data cache
     *  \param [in] timeout The over time of transfer
     *
     *  \return Return bool. Ture : transfer done, false : transfer failed
     */
    unsigned int transferProprietaryPacket(char *buffer,
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
    unsigned int transferProprietaryPacket(unsigned char *buffer, unsigned char length,
                                   unsigned int timeout = DEFAULT_TIMEOUT);

    /**
     *  \brief Set device mode
     *
     *  \param [in] mode The mode of device
     *
     *  \return Return null
     */
    unsigned int setDeviceMode(_device_mode_t mode);

    /**
     *  \brief Set device join a network
     *
     *  \param [in] command The type of join
     *  \param [in] timeout The over time of join
     *
     *  \return Return bool. True : join OK, false : join NOT OK
     */
    unsigned int setOTAAJoin(_otaa_join_cmd_t command,
                     unsigned int timeout = DEFAULT_TIMEOUT);

    /**
     *  \brief Set message unconfirmed repeat time
     *
     *  \param [in] time The repeat time, range from 1 to 15
     *
     *  \return Return null
     */
    unsigned int setUnconfirmedMessageRepeatTime(unsigned char time);

    /**
     *  \brief Set message retry times time
     *
     *  \param [in] time The retry time, range from 0 to 254
     *
     *  \return Return null
     */
    unsigned int setConfirmedMessageRetryTime(unsigned char time);

    /**
     *  \brief ON/OFF receice window 1
     *
     *  \param [in] command The true : ON, false OFF
     *
     *  \return Return null
     */
    unsigned int getReceiveWindowFirst(void);
    unsigned int setReceiveWindowFirst(bool command);
    /**
     *  \brief Set receice window 1 channel mapping
     *
     *  \param [in] channel The channel number, range from 0 to 71
     *  \param [in] frequency The frequency value of channel
     *
     *  \return Return null
     */
    unsigned int setReceiveWindowFirst(unsigned char channel, float frequency);

    /**
     *  \brief Set receice window 2 channel mapping
     *
     *  \param [in] frequency The frequency value of channel
     *  \param [in] dataRate The date rate value
     *
     *  \return Return null
     */
    unsigned int setReceiveWindowSecond(float frequency, _data_rate_t dataRate);
    /**
     *  \brief Set receice window 2 channel mapping
     *
     *  \param [in] frequency The frequency value of channel
     *  \param [in] spreadingFactor The spreading factor value
     *  \param [in] bandwidth The band width value
     *
     *  \return Return null
     */
    unsigned int setReceiveWindowSecond(float frequency,
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
    unsigned int setReceiveWindowDelay(_window_delay_t command, unsigned short _delay);

    /**
     *  \brief Set LoRaWAN class type
     *
     *  \param [in] type The class type
     *
     *  \return Return null
     */
    unsigned int setClassType(_class_type_t type);

    	/**
     *  \brief Change the device baud rate
     *
     *  \return Return null
     */
	unsigned int setDeviceBaudRate(_baudrate_bps_supported baud_rate );
	/**
     *  \brief Set device into low power mode
     *
     *  \return Return null
     */
    unsigned int setDeviceLowPower(unsigned int time_to_wakeups=0 );
	    /**
     *  \brief Set device into low power mode
     *
     *  \return Return null
     */
    unsigned int setDeviceLowPowerAutomode(bool mode );
    
    /**
     *  \brief Wakes up the device
     *
     *  \return Return null
     */
    unsigned int setDeviceWakeUp(void);

    /**
     *  \brief Reset device
     *
     *  \return Return null
     */
    unsigned int setDeviceReset(void);

    /**
     *  \brief Setup device default
     *
     *  \return Return null
     */
    unsigned int setDeviceDefault(void);

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
    unsigned int initP2PMode(unsigned short frequency            = 433,
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
    unsigned int transferPacketP2PMode(char *buffer);
    /**
     *  \brief Transfer the data
     *
     *  \param [in] *buffer The transfer data cache
     *  \param [in] length The length of data cache
     *
     *  \return Return bool. Ture : transfer done, false : transfer failed
     */
    unsigned int transferPacketP2PMode(unsigned char *buffer, unsigned char length);
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
        /* \brief Returns a very accurate Estimation of packet Transmition time expresed in mseconds
     *  \param [in]  payload size (data to transmit) in bytes
     *
     *  \return Return time in seconds to perform the packet transmition
     */
    float getTransmissionTime(unsigned int payload_size);
        /* \brief Returns a very accurate Estimation of packet Transmition current used to transmit that packet
		       and 
     *  \param [in]  payload size (data to transmit) in bytes
	 *  \param [in]  tx_period_s period in seconds in which you will transmit the packet. Default is 1 for hour.
     *
     *  \return Return power expressed in mAh expected for a voltage input of 3.3V.
     */
    float getTransmissionPower(unsigned int payload_size, float tx_period_s=3600); 
    
    /**
     *  \brief LoRaWAN raw data
     *
     *  \return Return null
     */
    unsigned int Debug(_debug_level value);
	
    /*Returns the last adquired bitRate by the last call of "getbitRate" */
    unsigned int readbitRate(void);
    /*Returns the last adquired txHead_time (time used to transmit LoRa message Header in ms) by the last call of "getbitRate" */
    float        readtxHead_time(void);
   /*variables declarations*/ 
   private:
    void SF_BW_to_bitrate_txhead_time(_spreading_factor_t SF, _band_width_t BW);/*stimate time to tx based on selected parameters*/
	bool init_first_call(void);/*Function only to be called by first LoRa Init*/
    void initSerial(_baudrate_bps_supported baud_rate); /*allows an easy init of the serial port*/
    uint8_t uart_tx, uart_rx ;   /*Uart Tx and RX pins for communication with LoRa_WIO*/
    bool lowpower_auto; /*LowPower Autoonmode enable*/
    bool adaptative_DR; /*Adatpative data rate. is true by default in the module*/
    unsigned int bitRate; /*[bitsps]set only by "getbitRate" function. Must be called before reading this variable */
    float txHead_time;   /*[miliseconds]set only by "getbitRate" function Must be called before reading this variable*/
    float freq_band;    /*[MHz]set only by "setDataRate" or "SetSpreadFactor" function Must be called before reading this variable*/
    short txPower;    /*[dBm]set only by "setPower" or "SetSpreadFactor" function Must be called before reading this variable*/

	
    char recv_buf[BEFFER_LENGTH_MAX];//reception buffer. Commands response can be up to 400 bytes according to data sheet examples
    char cmd[556];//store command to send
    char cmd_resp_ack[64];//store command response ACK to compare with string recieved and thus verify if the command worked. 
    #ifdef COMMAND_PRINT_TIME_MEASURE
    char cmd_time[128];//store commands time response
    #endif
    UART SerialLoRa;//memory allocate the serial lora inside the class. Used to allow a construction of the class outside
};

extern LoRaE5Class lora; /*Do not comment or change this!!. Original:extern LoRaE5Class lora;*/

#endif
