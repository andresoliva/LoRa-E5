/*
  LoRaE5 (fork from https://github.com/toddkrein/OTAA-LoRaWAN-Seeed)

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

#include "LoRaE5.h"
const char *physTypeStr[10] = {"EU434",        "EU868", "US915", "US915HYBRID",
                               "AU915",        "AS923", "CN470", "KR920",
                               "CN470PREQUEL", "STE920"};

LoRaE5Class::LoRaE5Class(void) {
    memset(_buffer, 0, 256);
    debug = false;
}

void LoRaE5Class::init(void) {         //For Hardware Serial
    SerialLoRa.begin(9600);             //For SAMD Variant and XIAO NRF
}

void LoRaE5Class::init(uint8_t rx, uint8_t tx) {      //For Custom Serial
    #if defined(ESP32)
        SerialLoRa.begin(9600, SERIAL_8N1, rx, tx); //M5Stack ESP32 Camera Module Development Board 
    #else
        SerialLoRa.begin(9600);   
    #endif
}

int LoRaE5Class::at_send_check_response(char* p_cmd, char* p_ack, int timeout_ms,char* p_response){
  #ifdef COMMAND_TIME_MEASURE
    int rx_ACK_time=0;
    int tx_msj_time=0;
    cmd_time[0]=0;//init the string len
  #endif 
    int ch;
    int num = 0;
    int i;
    int index = 0;
    int ret_val=0;//init with 0 as default return value
    int startMillis;
    //clean the serial port before issuing the command
    while (SerialLoRa.available() > 0){ ch = SerialLoRa.read();}//clean the read buffer
    /*Send the comand*/
    SerialLoRa.print(p_cmd); //sends command to Grove LoRa_E5 module
    //---------
    startMillis = millis();// Starts meassuring time after the command was sended
    #ifdef COMMAND_TIME_MEASURE
    tx_msj_time=millis();//original 0//ToDo: Check if 0 or millis(). check with longer packets because it does not seems to make a differnece
    #endif
    #ifdef COMMAND_PRINT_TO_USER
    SerialUSB.print(p_cmd);  // Serial.print(p_p_cmd, args);
    #endif
    /*ensure a valid p_ack (pointer to command string expected response from the module) was provided*/
    if (p_ack == NULL) { return ret_val;}
    /*Parse the response to the command. Also meassure the time to get the response*/
    while ((millis() - startMillis) < timeout_ms) {
       if (SerialLoRa.available() > 0){ //check if they are characters to be read 
           //we read one character at time because is the only way to get the ack tx time
            ch = SerialLoRa.read();
            recv_buf[index++] = ch;
            //begin with times callculation
            #ifdef COMMAND_TIME_MEASURE
              //Tx message time
              if (tx_msj_time==0){ //check if command to send message was issued. Then retrieves time
                 if (strstr(recv_buf, "Start") != NULL){ tx_msj_time=millis();}
              }
              if(tx_msj_time>0){
                //if (strstr(recv_buf, "Start") != NULL){
                if (strstr(recv_buf, "Wait ACK") != NULL){
                  tx_msj_time=millis() - tx_msj_time;
                  sprintf(cmd_time+strlen(cmd_time),"\r\nTime to TX message: %i ms.",tx_msj_time);
                  tx_msj_time=-1; //indicates the program to stop this parsing 
                }
              }
              //Reception ACK wait time
              if (rx_ACK_time==0){
                if (strstr(recv_buf, "Wait ACK") != NULL){ rx_ACK_time=millis();}
              }
              if(rx_ACK_time>0){
                if (strstr(recv_buf, "ACK Received") != NULL){
                  rx_ACK_time=millis() - rx_ACK_time;
                  sprintf(cmd_time+strlen(cmd_time),"\r\nTime to RX ACK from TX message: %i ms.",rx_ACK_time);
                  rx_ACK_time=-1;//indicates the program to stop this parsing 
                }
              }
            #endif     
        //check if the command sended was acknowledged properly
        if (strstr(recv_buf, p_ack) != NULL) {
            ret_val= millis() - startMillis;//returns command execution time
            break;//goes outside of code
          }
       }
      else{//If there are no characters to be read, delays 1 ms and tryes to read again
           delay(1);
           }  
      }/*End of While parsing loop*/    
      #ifdef COMMAND_PRINT_TO_USER
      recv_buf[index]=0;//indicates end of string
      SerialUSB.println("\r\n");
      SerialUSB.print(recv_buf);
      SerialUSB.println("\r\n");
      //delay(100);//gives enough time to print the commands, but it do not work :/
      #endif
      #ifdef COMMAND_TIME_MEASURE
      /*add the time used to print*/
      if (ret_val>0){sprintf(cmd_time+strlen(cmd_time),"\r\nTotal Command Time + Time to get ACK response: %i ms.",ret_val);}
      /*print the accumulated message*/
      if(strlen(cmd_time)>0){SerialUSB.print(cmd_time);}//print if something was written
      if(ret_val==0){SerialUSB.print("\r\n!!Command Failed!! Did not get expected \"Ok\"or\"ACK\" response from E5 module after sent the command");}
      #endif
    //----------------------
    /*if a buffer was provided, copy the response to the buffer */
    if (not(p_response == NULL)) { strcpy(p_response, recv_buf);}  
    //end of code: return cmd elapsed time in ms or 0 if did not work  
    return ret_val;
}



void LoRaE5Class::getVersion(char *buffer, short length,
                              unsigned int timeout) {
    if (buffer) {
        while (SerialLoRa.available()) SerialLoRa.read();
        sendCommand("AT+VER=?\r\n");
        readBuffer(buffer, length, timeout);
    }
}

int LoRaE5Class::getId(char *buffer, unsigned int timeout) {
    return(at_send_check_response("AT+ID\r\n","Done",timeout,buffer));
}

int LoRaE5Class::setId(char *DevAddr, char *DevEUI, char *AppEUI) {
    int time_cmd=0;

    if (DevAddr) {
        memset(cmd, 0, 64);
        sprintf(cmd, "AT+ID=DevAddr,\"%s\"\r\n", DevAddr);
        time_cmd=+at_send_check_response(cmd,"+ID: DevAddr",DEFAULT_TIMEOUT,NULL);
    }
    if (DevEUI) {
        memset(cmd, 0, 64);
        sprintf(cmd, "AT+ID=DevEui,\"%s\"\r\n", DevEUI);
        time_cmd=+at_send_check_response(cmd,"+ID: DevEui",DEFAULT_TIMEOUT,NULL);
    }
    if (AppEUI) {
        memset(cmd, 0, 64);
        sprintf(cmd, "AT+ID=AppEui,\"%s\"\r\n", AppEUI);
        time_cmd=+at_send_check_response(cmd,"+ID: AppEui",DEFAULT_TIMEOUT,NULL);
    }
    return(time_cmd);
}
/*Set each of the keys used for communication
Session Key (NwkSKey) is used for interaction between the Node and the Network Server.
Application Session Key (AppSKey) is used for encryption and decryption of the payload.
The application key (AppKey) is only known by the device and by the application. Dynamically activated devices (OTAA) 
use the Application Key (AppKey) to derive the two session keys during the activation procedure
*/
void LoRaE5Class::setKey(char *NwkSKey, char *AppSKey, char *AppKey) {
    char cmd[64];

    if (NwkSKey) {
        memset(cmd, 0, 64);
        sprintf(cmd, "AT+KEY=NWKSKEY,\"%s\"\r\n", NwkSKey);
        sendCommand(cmd);
#if _DEBUG_SERIAL_
        loraDebugPrint(DEFAULT_DEBUGTIME);
#endif
        delay(DEFAULT_TIMEWAIT);
    }

    if (AppSKey) {
        memset(cmd, 0, 64);
        sprintf(cmd, "AT+KEY=APPSKEY,\"%s\"\r\n", AppSKey);
        sendCommand(cmd);
#if _DEBUG_SERIAL_
        loraDebugPrint(DEFAULT_DEBUGTIME);
#endif
        delay(DEFAULT_TIMEWAIT);
    }

    if (AppKey) {
        memset(cmd, 0, 64);
        sprintf(cmd, "AT+KEY= APPKEY,\"%s\"\r\n", AppKey);
        sendCommand(cmd);
#if _DEBUG_SERIAL_
        loraDebugPrint(DEFAULT_DEBUGTIME);
#endif
        delay(DEFAULT_TIMEWAIT);
    }
}

bool LoRaE5Class::setDataRate(_data_rate_t dataRate,
                               _physical_type_t physicalType) {
    char cmd[32];
    // const char *str;

    if ((physicalType <= UNINIT) && (physicalType >= UNDEF)) {
        myType = UNINIT;
        debugPrint("Unknown datarate\n");
        return false;
    }

    myType = physicalType;
    sendCommand(F("AT+DR="));
    //    str = (const char*)(physTypeStr[(int)myType]);
    sendCommand(physTypeStr[myType]);
    //    sendCommand(str);
    sendCommand(F("\r\n"));
//    if(physicalType == EU434)sendCommand("AT+DR=EU433\r\n");
//    else if(physicalType == EU868)sendCommand("AT+DR=EU868\r\n");
//    else if(physicalType == US915)sendCommand("AT+DR=US915\r\n");
//    else if(physicalType == AU920)sendCommand("AT+DR=AU920\r\n");
#if _DEBUG_SERIAL_
    loraDebugPrint(DEFAULT_DEBUGTIME);
#endif
    delay(DEFAULT_TIMEWAIT);

    memset(cmd, 0, 32);
    sprintf(cmd, "AT+DR=%d\r\n", dataRate);
    sendCommand(cmd);
#if _DEBUG_SERIAL_
    loraDebugPrint(DEFAULT_DEBUGTIME);
#endif
    delay(DEFAULT_TIMEWAIT);
    return true;
}

void LoRaE5Class::setPower(short power) {
    char cmd[32];

    memset(cmd, 0, 32);
    sprintf(cmd, "AT+POWER=%d\r\n", power);
    sendCommand(cmd);
#if _DEBUG_SERIAL_
    loraDebugPrint(DEFAULT_DEBUGTIME);
#endif
    delay(DEFAULT_TIMEWAIT);
}

void LoRaE5Class::setPort(unsigned int port) {
    char cmd[32];

    memset(cmd, 0, 32);
    sprintf(cmd, "AT+PORT=%i\r\n", port);
    sendCommand(cmd);
    delay(DEFAULT_TIMEWAIT);
}

void LoRaE5Class::setAdaptiveDataRate(bool command) {
    if (command)
        sendCommand("AT+ADR=ON\r\n");
    else
        sendCommand("AT+ADR=OFF\r\n");
#if _DEBUG_SERIAL_
    loraDebugPrint(DEFAULT_DEBUGTIME);
#endif
    delay(DEFAULT_TIMEWAIT);
}

void LoRaE5Class::getChannel(void) {
    sendCommand("AT+CH\r\n");

    //loraDebugPrint(DEFAULT_DEBUGTIME);

    delay(DEFAULT_TIMEWAIT);
}

void LoRaE5Class::setChannel(unsigned char channel, float frequency) {
    char cmd[32];

    //    if(channel > 16) channel = 16;      // ??? this is wrong for US915

    memset(cmd, 0, 32);
    if (frequency == 0)
        sprintf(cmd, "AT+CH=%d,0\r\n", channel);
    else
        sprintf(cmd, "AT+CH=%d,%d.%d\r\n", channel, (short)frequency,
                short(frequency * 10) % 10);
    sendCommand(cmd);
#if _DEBUG_SERIAL_
    loraDebugPrint(DEFAULT_DEBUGTIME);
#endif
    delay(DEFAULT_TIMEWAIT);
}

void LoRaE5Class::setChannel(unsigned char channel, float frequency,
                              _data_rate_t dataRata) {
    char cmd[32];

    if (channel > 16) channel = 16;

    memset(cmd, 0, 32);
    sprintf(cmd, "AT+CH=%d,%d.%d,%d\r\n", channel, (short)frequency,
            short(frequency * 10) % 10, dataRata);
    sendCommand(cmd);
#if _DEBUG_SERIAL_
    loraDebugPrint(DEFAULT_DEBUGTIME);
#endif
    delay(DEFAULT_TIMEWAIT);
}

void LoRaE5Class::setChannel(unsigned char channel, float frequency,
                              _data_rate_t dataRataMin,
                              _data_rate_t dataRataMax) {
    char cmd[32];

    if (channel > 16) channel = 16;

    memset(cmd, 0, 32);
    sprintf(cmd, "AT+CH=%d,%d.%d,%d,%d\r\n", channel, (short)frequency,
            short(frequency * 10) % 10, dataRataMin, dataRataMax);
    sendCommand(cmd);
#if _DEBUG_SERIAL_
    loraDebugPrint(DEFAULT_DEBUGTIME);
#endif
    delay(DEFAULT_TIMEWAIT);
}

bool LoRaE5Class::transferPacket(char *buffer, unsigned int timeout) {
    unsigned char length = strlen(buffer);
    unsigned long timerStart, timerEnd;
    int count;

    while (SerialLoRa.available()) SerialLoRa.read();

    sendCommand("AT+MSG=\"");
    for (int i = 0; i < length; i++) SerialLoRa.write(buffer[i]);
    sendCommand("\"\r\n");

    while (true) {
        memset(_buffer, 0, BEFFER_LENGTH_MAX);
        count = readLine(_buffer, BEFFER_LENGTH_MAX, timeout);
        if (count == 0) continue;
            // handle timout!
#if _DEBUG_SERIAL_
        SerialUSB.print(_buffer);
#endif
        if (strstr(_buffer, "+MSG: Done")) return true;
    }

    //    memset(_buffer, 0, BEFFER_LENGTH_MAX);
    //    readBuffer(_buffer, BEFFER_LENGTH_MAX, timeout);
    //#if _DEBUG_SERIAL_
    //    SerialUSB.print(_buffer);
    //#endif
    //    if(strstr(_buffer, "+MSG: Done"))return true;
    //    return false;
}

bool LoRaE5Class::transferPacket(unsigned char *buffer, unsigned char length,
                                  unsigned int timeout) {
    char temp[3] = {0};
    unsigned long timerStart, timerEnd;
    while (SerialLoRa.available()) SerialLoRa.read();

    sendCommand("AT+MSGHEX=\"");
    for (int i = 0; i < length; i++) {
        sprintf(temp, "%02x", buffer[i]);
        SerialLoRa.write(temp);
    }
    sendCommand("\"\r\n");

    memset(_buffer, 0, BEFFER_LENGTH_MAX);
    readBuffer(_buffer, BEFFER_LENGTH_MAX, timeout);
#if _DEBUG_SERIAL_
    SerialUSB.print(_buffer);
#endif
    if (strstr(_buffer, "+MSGHEX: Done")) return true;
    return false;
}

int LoRaE5Class::transferPacketWithConfirmed(char *buffer,
                                               unsigned int timeout) {
    unsigned char length = strlen(buffer);
    int i;
    int time_ret;
    cmd[0]=0;//reset the string
    sprintf(cmd,"AT+CMSG=\"%s\"\r\n",buffer);
    time_ret=at_send_check_response(cmd,"Done",timeout,NULL);
    return time_ret;
}

bool LoRaE5Class::transferPacketWithConfirmed(unsigned char *buffer,
                                               unsigned char length,
                                               unsigned int timeout) {
    char temp[3] = {0};
    int i;
    int time_ret;
    /*sendCommand("AT+CMSGHEX=\"");
    for (int i = 0; i < length; i++) {
        sprintf(temp, "%02x", buffer[i]);
        SerialLoRa.write(temp);
    }*/
    cmd[0]=0;//reset the string size
    sprintf(cmd+strlen(cmd),"AT+CMSGHEX=\"");//name of the command
    for (int i = 0; i < length; i++) { sprintf(cmd+strlen(cmd), "%02x", buffer[i]);}//add the characters in hex format
    sprintf(cmd+strlen(cmd),"\"\r\n");//end of command
    time_ret=at_send_check_response(cmd,"Done",timeout,NULL);
    return time_ret;
}

short LoRaE5Class::receivePacket(char *buffer, short length, short *rssi) {
    char *ptr;
    short number = 0;

    ptr = strstr(_buffer, "RSSI ");
    if (ptr)
        *rssi = atoi(ptr + 5);
    else
        *rssi = -255;

    ptr = strstr(_buffer, "RX: \"");
    if (ptr) {
        ptr += 5;
        for (short i = 0;; i++) {
            char temp[3]      = {0, 0};
            unsigned char tmp = '?', result = 0;

            temp[0] = *(ptr + i * 3);
            temp[1] = *(ptr + i * 3 + 1);

            for (unsigned char j = 0; j < 2; j++) {
                if ((temp[j] >= '0') && (temp[j] <= '9'))
                    tmp = temp[j] - '0';
                else if ((temp[j] >= 'A') && (temp[j] <= 'F'))
                    tmp = temp[j] - 'A' + 10;
                else if ((temp[j] >= 'a') && (temp[j] <= 'f'))
                    tmp = temp[j] - 'a' + 10;

                result = result * 16 + tmp;
            }

            if (i < length) buffer[i] = result;

            if (*(ptr + i * 3 + 3) == '\"' && *(ptr + i * 3 + 4) == '\r' &&
                *(ptr + i * 3 + 5) == '\n') {
                number = i + 1;
                break;
            }
        }
    }

    ptr = strstr(_buffer, "MACCMD: \"");
    if (ptr) {
        buffer[0] = 'M';
        buffer[1] = 'A';
        buffer[2] = 'C';
        buffer[3] = 'C';
        buffer[4] = 'M';
        buffer[5] = 'D';
        buffer[6] = ':';

        ptr += 9;
        for (short i = 0;; i++) {
            char temp[3]      = {0};
            unsigned char tmp = '?', result = 0;

            temp[0] = *(ptr + i * 3);
            temp[1] = *(ptr + i * 3 + 1);

            for (unsigned char j = 0; j < 2; j++) {
                if ((temp[j] >= '0') && (temp[j] <= '9'))
                    tmp = temp[j] - '0';
                else if ((temp[j] >= 'A') && (temp[j] <= 'F'))
                    tmp = temp[j] - 'A' + 10;
                else if ((temp[j] >= 'a') && (temp[j] <= 'f'))
                    tmp = temp[j] - 'a' + 10;

                result = result * 16 + tmp;
            }

            if ((i + 7) < length) buffer[i + 7] = result;

            if (*(ptr + i * 3 + 3) == '\"' && *(ptr + i * 3 + 4) == '\r' &&
                *(ptr + i * 3 + 5) == '\n') {
                number = i + 1 + 7;
                break;
            }
        }
    }

    memset(_buffer, 0, BEFFER_LENGTH_MAX);

    return number;
}

bool LoRaE5Class::transferProprietaryPacket(char *buffer,
                                             unsigned int timeout) {
    unsigned char length = strlen(buffer);

    while (SerialLoRa.available()) SerialLoRa.read();

    sendCommand("AT+PMSG=\"");
    for (int i = 0; i < length; i++) SerialLoRa.write(buffer[i]);
    sendCommand("\"\r\n");

    memset(_buffer, 0, BEFFER_LENGTH_MAX);
    readBuffer(_buffer, BEFFER_LENGTH_MAX, timeout);
#if _DEBUG_SERIAL_
    SerialUSB.print(_buffer);
#endif
    if (strstr(_buffer, "+PMSG: Done")) return true;
    return false;
}

bool LoRaE5Class::transferProprietaryPacket(unsigned char *buffer,
                                             unsigned char length,
                                             unsigned int timeout) {
    char temp[3] = {0};

    while (SerialLoRa.available()) SerialLoRa.read();

    sendCommand("AT+PMSGHEX=\"");
    for (int i = 0; i < length; i++) {
        sprintf(temp, "%02x", buffer[i]);
        SerialLoRa.write(temp);
    }
    sendCommand("\"\r\n");

    memset(_buffer, 0, BEFFER_LENGTH_MAX);
    readBuffer(_buffer, BEFFER_LENGTH_MAX, timeout);
#if _DEBUG_SERIAL_
    SerialUSB.print(_buffer);
#endif
    if (strstr(_buffer, "+PMSGHEX: Done")) return true;
    return false;
}

void LoRaE5Class::setUnconfirmedMessageRepeatTime(unsigned char time) {
    char cmd[32];

    if (time > 15)
        time = 15;
    else if (time == 0)
        time = 1;

    memset(cmd, 0, 32);
    sprintf(cmd, "AT+REPT=%d\r\n", time);
    sendCommand(cmd);
#if _DEBUG_SERIAL_
    loraDebugPrint(DEFAULT_DEBUGTIME);
#endif
    delay(DEFAULT_TIMEWAIT);
}

void LoRaE5Class::setConfirmedMessageRetryTime(unsigned char time) {
    char cmd[32];

    if (time > 15)
        time = 15;
    else if (time == 0)
        time = 1;

    memset(cmd, 0, 32);
    sprintf(cmd, "AT+RETRY=%d\r\n", time);
    sendCommand(cmd);
#if _DEBUG_SERIAL_
    loraDebugPrint(DEFAULT_DEBUGTIME);
#endif
    delay(DEFAULT_TIMEWAIT);
}

void LoRaE5Class::getReceiveWindowFirst(void) {
    sendCommand("AT+RXWIN1\r\n");
    //loraDebugPrint(DEFAULT_DEBUGTIME);
    delay(DEFAULT_TIMEWAIT);
}

void LoRaE5Class::setReceiveWindowFirst(bool command) {
    if (command)
        sendCommand("AT+RXWIN1=ON\r\n");
    else
        sendCommand("AT+RXWIN1=OFF\r\n");
#if _DEBUG_SERIAL_
    loraDebugPrint(DEFAULT_DEBUGTIME);
#endif
    delay(DEFAULT_TIMEWAIT);
}
void LoRaE5Class::setReceiveWindowFirst(unsigned char channel,
                                         float frequency) {
    char cmd[32];

    //    if(channel > 16) channel = 16;

    memset(cmd, 0, 32);
    if (frequency == 0)
        sprintf(cmd, "AT+RXWIN1=%d,0\r\n", channel);
    else
        sprintf(cmd, "AT+RXWIN1=%d,%d.%d\r\n", channel, (short)frequency,
                short(frequency * 10) % 10);
    sendCommand(cmd);
    SerialUSB.print(cmd);
#if _DEBUG_SERIAL_
    loraDebugPrint(DEFAULT_DEBUGTIME *
                   4);  // this can have a lot of data to dump
#endif
    delay(DEFAULT_TIMEWAIT);
}

void LoRaE5Class::setReceiveWindowSecond(float frequency,
                                          _data_rate_t dataRate) {
    char cmd[32];

    memset(cmd, 0, 32);
    sprintf(cmd, "AT+RXWIN2=%d.%d,%d\r\n", (short)frequency,
            short(frequency * 10) % 10, dataRate);
    sendCommand(cmd);
#if _DEBUG_SERIAL_
    loraDebugPrint(DEFAULT_DEBUGTIME);
#endif
    delay(DEFAULT_TIMEWAIT);
}

void LoRaE5Class::setReceiveWindowSecond(float frequency,
                                          _spreading_factor_t spreadingFactor,
                                          _band_width_t bandwidth) {
    char cmd[32];

    memset(cmd, 0, 32);
    sprintf(cmd, "AT+RXWIN2=%d.%d,%d,%d\r\n", (short)frequency,
            short(frequency * 10) % 10, spreadingFactor, bandwidth);
    sendCommand(cmd);
#if _DEBUG_SERIAL_
    loraDebugPrint(DEFAULT_DEBUGTIME);
#endif
    delay(DEFAULT_TIMEWAIT);
}

void LoRaE5Class::setReceiveWindowDelay(_window_delay_t command,
                                         unsigned short _delay) {
    char cmd[32];

    memset(cmd, 0, 32);
    if (command == RECEIVE_DELAY1)
        sprintf(cmd, "AT+DELAY=RX1,%d\r\n", _delay);
    else if (command == RECEIVE_DELAY2)
        sprintf(cmd, "AT+DELAY=RX2,%d\r\n", _delay);
    else if (command == JOIN_ACCEPT_DELAY1)
        sprintf(cmd, "AT+DELAY=JRX1,%d\r\n", _delay);
    else if (command == JOIN_ACCEPT_DELAY2)
        sprintf(cmd, "AT+DELAY=JRX2,%d\r\n", _delay);
    sendCommand(cmd);
#if _DEBUG_SERIAL_
    loraDebugPrint(DEFAULT_DEBUGTIME);
#endif
    delay(DEFAULT_TIMEWAIT);
}

void LoRaE5Class::setClassType(_class_type_t type) {
    if (type == CLASS_A)
        sendCommand("AT+CLASS=A\r\n");
    else if (type == CLASS_C)
        sendCommand("AT+CLASS=C\r\n");
#if _DEBUG_SERIAL_
    loraDebugPrint(DEFAULT_DEBUGTIME);
#endif
    delay(DEFAULT_TIMEWAIT);
}

//
// set the JOIN mode to either LWOTAA or LWABP
// does a half-hearted attempt to check the results
//
bool LoRaE5Class::setDeviceMode(_device_mode_t mode) {
    char buffer[kLOCAL_BUFF_MAX];
    int timeout = 1;

    if (mode == LWABP)
        sendCommand("AT+MODE=LWABP\r\n");
    else if (mode == LWOTAA)
        sendCommand("AT+MODE=LWOTAA\r\n");
    else
        return false;

    memset(buffer, 0, kLOCAL_BUFF_MAX);
    readBuffer(buffer, kLOCAL_BUFF_MAX - 1, timeout);

#if _DEBUG_SERIAL_
    SerialUSB.print(buffer);
//    loraDebugPrint(DEFAULT_DEBUGTIME);
#endif
    delay(DEFAULT_TIMEWAIT);

    return strstr(
        buffer, "+MODE:");  // if it works, response is of form "+MODE: LWOTTA"
}

//
//  JOIN with the application
//
//  setDeviceMode should have been called before this.
bool LoRaE5Class::setOTAAJoin(_otaa_join_cmd_t command,
                               unsigned int timeout) {
    // char *ptr;
    short count;
    bool joined = false;

    if (command == JOIN)
        sendCommand("AT+JOIN\r\n");
    else if (command == FORCE)
        sendCommand("AT+JOIN=FORCE\r\n");
    else {
        SerialUSB.print("Bad command to setOTAAJoin\n");
        return false;
    }

#if _DEBUG_SERIAL_
//    loraDebugPrint(DEFAULT_DEBUGTIME);
#endif
    //    delay(DEFAULT_TIMEWAIT);

    while (true) {
        memset(_buffer, 0, BEFFER_LENGTH_MAX);
        count = readLine(_buffer, BEFFER_LENGTH_MAX, timeout);
        if (count == 0) continue;

            // !!! handle timeout
#if _DEBUG_SERIAL_
        SerialUSB.print(_buffer);
#endif
        if (strstr(_buffer, "+JOIN: Join failed")) continue;
        if (strstr(_buffer, "+JOIN: LoRaWAN modem is busy")) continue;
        if (strstr(_buffer, "+JOIN: NORMAL")) continue;
        if (strstr(_buffer, "+JOIN: FORCE")) continue;
        if (strstr(_buffer, "+JOIN: Start")) continue;
        if (strstr(_buffer, "+JOIN: Done")) break;
        if (strstr(_buffer, "+JOIN: No free channel")) break;
        if (strstr(_buffer, "+JOIN: Network joined")) {
            joined = true;
            continue;
        }
        if (strstr(_buffer, "+JOIN: NetID")) {
            joined = true;
            continue;
        }

        SerialUSB.print("Result didn't match anything I expected.\n");
    }

    SerialUSB.print("Done with Join\n");
    return joined;
}

void LoRaE5Class::setDeviceLowPower(void) {
    sendCommand("AT+LOWPOWER\r\n");
#if _DEBUG_SERIAL_
    loraDebugPrint(DEFAULT_DEBUGTIME);
#endif
    delay(DEFAULT_TIMEWAIT);
}

//
// Reset the LoRa module. Does not factory reset
//
void LoRaE5Class::setDeviceReset(void) {
    sendCommand("AT+RESET\r\n");
#if _DEBUG_SERIAL_
    loraDebugPrint(DEFAULT_DEBUGTIME);
#endif
    delay(DEFAULT_TIMEWAIT);
}

//
//  Factory reset the module.
//
void LoRaE5Class::setDeviceDefault(void) {
    sendCommand("AT+FDEFAULT=RISINGHF\r\n");
#if _DEBUG_SERIAL_
    loraDebugPrint(DEFAULT_DEBUGTIME);
#endif
    delay(DEFAULT_TIMEWAIT);
}

void LoRaE5Class::initP2PMode(unsigned short frequency,
                               _spreading_factor_t spreadingFactor,
                               _band_width_t bandwidth,
                               unsigned char txPreamble,
                               unsigned char rxPreamble, short power) {
    char cmd[64] = {
        0,
    };
    sprintf(cmd, "AT+TEST=RFCFG,%d,%d,%d,%d,%d,%d\r\n", frequency,
            spreadingFactor, bandwidth, txPreamble, rxPreamble, power);

    sendCommand("AT+MODE=TEST\r\n");
    delay(DEFAULT_TIMEWAIT);
    sendCommand(cmd);
    delay(DEFAULT_TIMEWAIT);
    sendCommand("AT+TEST=RXLRPKT\r\n");
    delay(DEFAULT_TIMEWAIT);
}

void LoRaE5Class::transferPacketP2PMode(char *buffer) {
    unsigned char length = strlen(buffer);
    unsigned long timerStart, timerEnd;
    sendCommand("AT+TEST=TXLRSTR,\"");
    for (int i = 0; i < length; i++) SerialLoRa.write(buffer[i]);
    sendCommand("\"\r\n");
}

void LoRaE5Class::transferPacketP2PMode(unsigned char *buffer,
                                         unsigned char length) {
    char temp[3] = {0};
    unsigned long timerStart, timerEnd;
    sendCommand("AT+TEST=TXLRPKT,\"");
    for (int i = 0; i < length; i++) {
        sprintf(temp, "%02x", buffer[i]);
        SerialLoRa.write(temp);
    }
    sendCommand("\"\r\n");
}

short LoRaE5Class::receivePacketP2PMode(unsigned char *buffer, short length,
                                         short *rssi, unsigned int timeout) {
    char *ptr;
    short number;

    while (SerialLoRa.available()) SerialLoRa.read();
    memset(_buffer, 0, BEFFER_LENGTH_MAX);
    readBuffer(_buffer, BEFFER_LENGTH_MAX, timeout);

    ptr = strstr(_buffer, "LEN");
    if (ptr)
        number = atoi(ptr + 4);
    else
        number = 0;

    if (number <= 0) return 0;

    ptr = strstr(_buffer, "RSSI:");
    if (ptr)
        *rssi = atoi(ptr + 5);
    else
        *rssi = -255;

    ptr = strstr(_buffer, "RX \"");
    if (ptr) {
        ptr += 4;
        for (short i = 0; i < number; i++) {
            char temp[3]      = {0};
            unsigned char tmp = '?', result = 0;

            temp[0] = *(ptr + i * 2);
            temp[1] = *(ptr + i * 2 + 1);

            for (unsigned char j = 0; j < 2; j++) {
                if ((temp[j] >= '0') && (temp[j] <= '9'))
                    tmp = temp[j] - '0';
                else if ((temp[j] >= 'A') && (temp[j] <= 'F'))
                    tmp = temp[j] - 'A' + 10;
                else if ((temp[j] >= 'a') && (temp[j] <= 'f'))
                    tmp = temp[j] - 'a' + 10;

                result = result * 16 + tmp;
            }

            if (i < length) buffer[i] = result;
        }
    }

    memset(_buffer, 0, BEFFER_LENGTH_MAX);

    return number;
}

// short LoRaE5Class::getBatteryVoltage(void)
// {
//     int battery;

//     pinMode(CHARGE_STATUS_PIN, OUTPUT);
//     digitalWrite(CHARGE_STATUS_PIN, LOW);
//     delay(DEFAULT_TIMEWAIT);
//     battery = (analogRead(BATTERY_POWER_PIN) * 3300 * 11) >> 10;
//     pinMode(CHARGE_STATUS_PIN, INPUT);

//     return battery;
// }

// ??? I think this essentially connects the serial port to the LoRa module.
// @@@ typing a "~" will exit
//
void LoRaE5Class::loraDebug(void) {
    char c;

    while (true) {
        if (SerialUSB.available()) {
            c = SerialUSB.read();
            if (c == '~') return;
            SerialLoRa.write(c);
        }
        if (SerialLoRa.available()) SerialUSB.write(SerialLoRa.read());
    }
}

#if _DEBUG_SERIAL_
//
//  timeout is the total amount of time allowed for collecting data
//  Would it make more sense if it was the time w/o a character?
void LoRaE5Class::loraDebugPrint(unsigned int timeout) {
    unsigned long timerStart, timerEnd;
    char c;

    timerStart = millis();

    while (1) {
        while (SerialLoRa.available()) {
            SerialUSB.write(c = SerialLoRa.read());
            if (c == '\n')
                return;  // !!! This won't work for commands that return
                         // multiple lines.
            timerStart = millis();
        }

        timerEnd = millis();
        //        if(timerEnd - timerStart > timeout)break;
        if (timerEnd - timerStart > timeout) break;
    }
}
#endif

void LoRaE5Class::debugPrint(const char *str) {
    SerialUSB.print(str);
}

void LoRaE5Class::sendCommand(const char *command) {
    SerialLoRa.print(command);
}

void LoRaE5Class::sendCommand(const __FlashStringHelper *command) {
    SerialLoRa.print(command);
}

short LoRaE5Class::readBuffer(char *buffer, short length,
                               unsigned int timeout) {
    short i = 0;
    unsigned long timerStart, timerEnd;

    timerStart = millis();

    while (1) {
        if (i < length) {
            while (SerialLoRa.available()) {
                char c      = SerialLoRa.read();
                buffer[i++] = c;
            }
        }

        timerEnd = millis();
        if ((timerEnd - timerStart) > timeout) break;
    }

    return i;
}

short LoRaE5Class::readLine(char *buffer, short length,
                             unsigned int timeout) {
    short i = 0;
    unsigned long timerStart, timerEnd;
    char c = '\n';

    timerStart = millis();

    while (1) {
        if (i < length - 1) {
            while (SerialLoRa.available()) {
                c           = SerialLoRa.read();
                buffer[i++] = c;
                if (c == '\n') break;
                timerStart = millis();
            }
        }
        if (c == '\n') break;  // @@@ barf
        timerEnd = millis();
        if (timerEnd - timerStart > timeout) break;
    }

    buffer[i] = 0;  // terminate the string
    return i;
}
short LoRaE5Class::waitForResponse(char *response, unsigned int timeout) {
    short len = strlen(response);
    short sum = 0;
    unsigned long timerStart, timerEnd;

    timerStart = millis();

    while (1) {
        if (SerialLoRa.available()) {
            char c = SerialLoRa.read();

            sum = (c == response[sum]) ? sum + 1 : 0;
            if (sum == len) break;
        }

        timerEnd = millis();
        if (timerEnd - timerStart > timeout) return -1;
    }

    return 0;
}

short LoRaE5Class::sendCommandAndWaitForResponse(char *command, char *response,
                                                  unsigned int timeout) {
    sendCommand(command);

    return waitForResponse(response, timeout);
}

LoRaE5Class lora;