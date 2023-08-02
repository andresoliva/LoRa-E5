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


/*COMMAND LIST AND EXAMPLES*/
//https://files.seeedstudio.com/products/317990687/res/LoRa-E5%20AT%20Command%20Specification_V1.0%20.pdf
#include "LoRa-E5.h"

const char *physTypeStr[10] = {"EU434",        "EU868", "US915", "US915HYBRID",
                               "AU915",        "AS923", "CN470", "KR920",
                               "CN470PREQUEL", "STE920"};

LoRaE5Class::LoRaE5Class(void) {
    lowpower_auto=false; /*LowPower Autoonmode enable*/
    adaptative_DR=true; /*Adatpative data rate. is true by default in the module*/
	txPower=14;//14 is default set up
	freq_band=0;
	uart_tx=0;
	uart_rx=0;
}
/*SERIAL CUSTOM MODE
link: https://forum.arduino.cc/t/third-serial-on-nano-33-ble-can-be-software/929047/10  */

void LoRaE5Class::initSerial(_baudrate_bps_supported baud_rate){
	/*proceed to init based if Tx and RX were provided during LoRa.init call*/
	 #ifdef COMMAND_PRINT_TO_USER
      SerialUSB.print("\r\nSerialLora baud rate set:");  // Serial.print(p_p_cmd, args);
      SerialUSB.print(baud_rate); /*print the command*/
	 #endif 
	if((uart_tx==0)and(uart_rx==0)){
	 /*Init UART with default pins if were not provided*/	
	 memcpy(&SerialLoRa,&SerialLoRa_native, sizeof(SerialLoRa));
     SerialLoRa.begin((unsigned int)baud_rate);   /*For software LoRa serial*/   
	}
	else{
	/*Init UART with the pins provided*/	
	  #if defined(ESP32)
        SerialLoRa.begin(baud_rate, SERIAL_8N1, uart_rx, uart_tx); //M5Stack ESP32 Camera Module Development Board 
      #else
        #ifdef PRINT_TO_USER 
        Serial.print("\r\nIMPORTANT: RUNNING serial port used with LoRa communication as a SoftwareSerial");/*to print the obtained characters*/
        #endif
        UART SerialLoRa_aux(uart_tx,uart_rx);//you have to call the constructor in this way
        memcpy(&SerialLoRa,&SerialLoRa_aux, sizeof(SerialLoRa));
        SerialLoRa.begin(baud_rate,SERIAL_8N1);   /*For software LoRa serial*/  
      #endif  
	}
}
/*first call of init inside the class*/
bool LoRaE5Class::init_first_call(void) { //For Hardware Serial
     unsigned int cmd_time;
	 bool ret=false;
	 initSerial(LORA_BAUDRATE_DEFAULT); //9600 is the default baud_rate115200
	 //test the differents baud rates in order to perform an automatic detection
      #ifdef PRINT_TO_USER 
        Serial.print("\r\nTesting connection with the device");/*to print the obtained characters*/
      #endif
	 lowpower_auto=true;/*Set here just if automode was disabled in a previous program*/
	 if(at_send_check_response("AT+\r\n","AT",DEFAULT_TIMEWAIT, NULL)==0){
	    SerialLoRa.end();
	    initSerial(BR_38400); 
	    if(at_send_check_response("AT+\r\n","AT",DEFAULT_TIMEWAIT, NULL)==0){
		   SerialLoRa.end();
	       initSerial(BR_115200); 
		   if(at_send_check_response("AT+\r\n","AT",DEFAULT_TIMEWAIT, NULL)==0){
		     ;;
		   }else{ret=true;}
	     }else{ret=true;}
	 }else{ret=true;}
	  #ifdef PRINT_TO_USER 
	    if(ret==true){Serial.print("\r\n GROOVE WIO-LoRa-E5 Detected");}/*to print the obtained characters*/
        if(ret==false){Serial.print("\r\nERROR: COULD NOT DETECT GROOVE WIO-LoRa-E5 module");}/*to print the obtained characters*/
      #endif
	 /*once the baud rate has been detected, work into that mode*/
	 setDeviceLowPowerAutomode(false);//first thing to do is to set automode to false to avoid issues with this mode
     /*Wake Up the LoRa module*/
     setDeviceWakeUp();/*if the module is not in sleep state, this command does nothing*/
return ret;
}
/*Set up serial port, remove auto low power and wake up the device*/
void LoRaE5Class::init(void) { //For Hardware Serial
      if(init_first_call()==false){while(1);};
}
/*Set up serial port, remove auto low power and wake up the device*/
void LoRaE5Class::init(uint8_t tx, uint8_t rx) {//for software serial
     /*Stores the pints into a class*/
	  uart_tx=tx;
	  uart_rx=rx;
      if(init_first_call()==false){while(1);};
}

unsigned int LoRaE5Class::readBuffer(char* buffer, unsigned int length, unsigned int timeout_ms){
char ch;
unsigned int index;
int startMillis;
//clean reception buffer after starting with reception:
strncpy(buffer," ",sizeof(recv_buf));//fill the content with white spaces
strlcpy(buffer," ",1);//Manually indicates end of string
#ifdef COMMAND_PRINT_TO_USER
SerialUSB.print("\r\nReading serial buffer");  // Serial.print(p_p_cmd, args);
#endif
//starting to read
startMillis = millis();// Starts meassuring time after the command was sended
delay(timeout_ms);//Sleep for this period until attempting to read
// ((millis() - startMillis) < timeout_ms) {//commented: not used at the moment
       while (SerialLoRa.available() > 0){ //check if they are characters to be read 
            ch = SerialLoRa.read();
            buffer[index++] = ch; //add the character
            if (index>=length){break;}//protect a buffer overflow
            }
  //     }//commented: not used at the moment 
return(index);//return the amount of bytes read    
}

/*function used to send command to lora modules*/
unsigned int LoRaE5Class::at_send_check_response(char* p_cmd, char* p_ack, unsigned int timeout_ms,char* p_response){
    #ifdef COMMAND_PRINT_TIME_MEASURE
    cmd_time[0]=0;//init the string len
    #endif 
    int tx_and_rx_ACK_time=0;//time to transmit and recieve message
    int ch;
    int num = 0;
    int i;
    int index = 0;
    int ret_val=0;//init with 0 as default return value
    int startMillis;
    /*clean reception buffer after starting with reception:*/
    strncpy(recv_buf," ",sizeof(recv_buf));//fill the content with white spaces
    strlcpy(recv_buf," ",1);//Manually indicates end of string
    /*clean the serial port before issuing the command*/
    while (SerialLoRa.available() > 0){ ch = SerialLoRa.read();}//clean the read buffer
    startMillis = millis();//DO NOT MOVE FROM HERE Starts meassuring time BEFORE the command was sended. 
	/*Send special chaarcter for compatibility with LOWPOWER=AUTOMODE*/
	if (lowpower_auto){SerialLoRa.write(0xff);SerialLoRa.write(0xff);SerialLoRa.write(0xff);SerialLoRa.write(0xff);};
	/*Send the comand*/
	if (!(p_cmd == NULL)){ SerialLoRa.print(p_cmd);} //sends command to Grove LoRa_E5 module
	       /*---------*/
    #ifdef COMMAND_PRINT_TO_USER
    if (!(p_cmd == NULL)){
      SerialUSB.print("\r\n--------Command sent:\r\n");  // Serial.print(p_p_cmd, args);
      SerialUSB.print(p_cmd); /*print the command*/
	  }
    #endif
    /*ensure a valid p_ack (pointer to command string expected response from the module) was provided*/
    if (p_ack == NULL) { 
      #ifdef COMMAND_PRINT_TO_USER
      SerialUSB.print("\r\nYou must specify the expected command response or use the \"AT_NO_ACK\" macro. Example: at_send_check_response(\"AT+*COMMAND CONTENT*\\r\\n\",AT_NO_ACK, 100,NULL)");
      #endif
      return ret_val;
    }                 
    /*Parse the response to the command. Also meassure the time to get the response*/
    while ((millis() - startMillis) < timeout_ms) {
       if (SerialLoRa.available() > 0){ //check if they are characters to be read 
           //we read one character at time because is the only way to get the ack tx time
            ch = SerialLoRa.read();
            recv_buf[index++] = ch; //add the character
            //begin with times callculation
              /*Reception ACK wait time*/
              if (tx_and_rx_ACK_time==0){
                if (strstr(recv_buf, "Wait ACK") != NULL){ tx_and_rx_ACK_time=millis();}
              }
              if(tx_and_rx_ACK_time>0){
                if (strstr(recv_buf, "ACK Received") != NULL){
                  tx_and_rx_ACK_time=millis() - tx_and_rx_ACK_time;
                  #ifdef COMMAND_PRINT_TIME_MEASURE
                  sprintf(cmd_time+strlen(cmd_time),"\r\nTime to Transmit message and Recieve ACK from TX message: %i ms.",tx_and_rx_ACK_time);
                  #endif 
                  tx_and_rx_ACK_time=-1;//indicates the program to stop this parsing 
                }
              }    
        /*check if the command sended was acknowledged properly*/
        if (strstr(recv_buf, p_ack) != NULL) {
            ret_val= millis() - startMillis;//returns command execution time
            break;/*goes outside of code*/
            }
          }
      else{/*If there are no characters to be read, delays 1 ms and tryes to read again*/
           delay(1);
           }  
      }/*End of While parsing loop*/
      if((strstr(AT_NO_ACK, p_ack) != NULL)){ret_val=millis() - startMillis;}//If this mode is selected, do not display     
      #ifdef COMMAND_PRINT_TO_USER
      SerialUSB.print("--------Command responses:\r\n");
      SerialUSB.print(recv_buf);
      SerialUSB.print("\r\n--------End of Commands responses");
      #endif
      #ifdef COMMAND_PRINT_TIME_MEASURE
      /*add the time used to print*/
      if (ret_val>0){sprintf(cmd_time+strlen(cmd_time),"\r\nTotal Command Time + Time to get ACK response: %i ms.",ret_val);}
      /*print the accumulated message*/
      if(strlen(cmd_time)>0){SerialUSB.print(cmd_time);}/*print if something was written*/
      #endif
      #ifdef COMMAND_PRINT_TO_USER
      if(ret_val==0){SerialUSB.print("\r\n!!Command Failed!! Did not get the expected \"Ok\" or \"ACK\" response from E5 module after sending the command.");}
      #endif
    /*---------------------------------------*/
    /*if a buffer was provided, copy the response to the buffer */
    if (not(p_response == NULL)) { strcpy(p_response, recv_buf);}  
    /*end of code: return cmd elapsed time in ms or 0 if did not work */
    return ret_val;
}


unsigned int LoRaE5Class::getVersion(char *buffer, unsigned int timeout) {
    return(at_send_check_response("AT+VER=?\r\n",AT_NO_ACK,timeout,buffer));
}

unsigned int LoRaE5Class::getId(char *buffer,_deviceID id, unsigned int timeout) {
	unsigned int time_cmd=0;
    if(id==DevAddr){time_cmd=at_send_check_response("AT+ID=DevAddr\r\n",AT_NO_ACK,timeout,buffer);}
    if(id==DevEui){time_cmd=at_send_check_response("AT+ID=DevEui\r\n",AT_NO_ACK,timeout,buffer);}
	if(id==AppEui){time_cmd=at_send_check_response("AT+ID=AppEui\r\n",AT_NO_ACK,timeout,buffer);}
    return(time_cmd);
}

unsigned int LoRaE5Class::setId(char *DevAddr, char *DevEUI, char *AppEUI) {
    unsigned int time_cmd=0;

    if (DevAddr) {
        memset(cmd, 0, 64);
        sprintf(cmd, "AT+ID=DevAddr,\"%s\"\r\n", DevAddr);
        time_cmd=+at_send_check_response(cmd,AT_NO_ACK,DEFAULT_TIMEWAIT*2,NULL);
    }
    if (DevEUI) {
        memset(cmd, 0, 64);
        sprintf(cmd, "AT+ID=DevEui,\"%s\"\r\n", DevEUI);
        time_cmd=+at_send_check_response(cmd,AT_NO_ACK,DEFAULT_TIMEWAIT*2,NULL);
    }
    if (AppEUI) {
        memset(cmd, 0, 64);
        sprintf(cmd, "AT+ID=AppEui,\"%s\"\r\n", AppEUI);
        time_cmd=+at_send_check_response(cmd,AT_NO_ACK,DEFAULT_TIMEWAIT*2,NULL);
    }
    return(time_cmd);
}
/*Set each of the keys used for communication
Session Key (NwkSKey) is used for interaction between the Node and the Network Server.
Application Session Key (AppSKey) is used for encryption and decryption of the payload.
The application key (AppKey) is only known by the device and by the application. Dynamically activated devices (OTAA) 
use the Application Key (AppKey) to derive the two session keys during the activation procedure
*/
unsigned int LoRaE5Class::setKey(char *NwkSKey, char *AppSKey, char *AppKey) {
    unsigned int time_cmd=0;
	cmd[0]='\0';//reset the string position
	cmd_resp_ack[0]='\0';//reset the string position
    if (NwkSKey) {
        sprintf(cmd, "AT+KEY=NWKSKEY,\"%s\"\r\n", NwkSKey);
		sprintf(cmd_resp_ack,"%s", NwkSKey);
        time_cmd=+at_send_check_response(cmd,cmd_resp_ack,DEFAULT_TIMEWAIT*2,NULL);
    }

    if (AppSKey) {
        sprintf(cmd, "AT+KEY=APPSKEY,\"%s\"\r\n", AppSKey);
		sprintf(cmd_resp_ack,"%s", AppSKey);
        time_cmd=+at_send_check_response(cmd,cmd_resp_ack,DEFAULT_TIMEWAIT*2,NULL);
    }

    if (AppKey) {
        sprintf(cmd, "AT+KEY= APPKEY,\"%s\"\r\n", AppKey);
	    sprintf(cmd_resp_ack,"%s", AppKey);
        time_cmd=+at_send_check_response(cmd,cmd_resp_ack,DEFAULT_TIMEWAIT*2,NULL);
    }
    return(time_cmd);
}
void LoRaE5Class::SF_BW_to_bitrate_txhead_time(_spreading_factor_t SF, _band_width_t BW) {
	unsigned char scale=4;
  /*set scale value based on BW*/
    if(BW==BW500){scale=4;}
    if(BW==BW250){scale=2;}
    if(BW==BW125){scale=1;}
  /*Set value of return based on SF and BW*/
    if(SF==SF12){//RX win: 0payload+43 ms aprox
      bitRate=250*scale;
      txHead_time=1155.1/scale;//RX WINDOW:1190 ms  dif with tx 35
      }
    if(SF==SF11){
      bitRate=440*scale;
      txHead_time=577.5/scale;//RX WINDOW:616 ms  dif with tx 39
      }
    if(SF==SF10){
      bitRate=980*scale;
      txHead_time=288.8/scale;//RX WINDOW:329 ms dif with tx 40
      }
    if(SF==SF9){
      bitRate=1760*scale;
      txHead_time=164.9/scale; //RX WINDOW:200 ms dif with tx 28.6
      }
    if(SF==SF8){
      bitRate=3125*scale;
      txHead_time=82.4/scale; //RX WINDOW:125 ms dif with tx 42.7
      }     
    if(SF==SF7){
      bitRate=5470*scale;
      txHead_time=46.3/scale;//RX WINDOW:88 ms dif with tx 41.7
      }
    /*especial case: 50 kbps with FSK modulation instead of LoRa*/  
    if(BW==BW50kbps){
      bitRate=50000;
      txHead_time=0;/*We use 0 because I could not find tis value in the web.*/
      }
}
unsigned int LoRaE5Class::getbitRate(unsigned int* pbitRate,float* ptxHead_time) {
    unsigned int time_cmd=0;
	_band_width_t BW=BWX;
	_spreading_factor_t SF=SFX;
    cmd[0]='\0';//reset the string position
    cmd_resp_ack[0]='\0';//reset the string position
    char substr[20];//16 is max size, we use 20 just in case.
    char* pstr;
    /*Send command to get bitRate*/
    sprintf(cmd, "AT+DR\r\n");
    time_cmd=+at_send_check_response(cmd,AT_NO_ACK,DEFAULT_TIMEWAIT,NULL);
    /*Example of expected response to be contained in recv_buf
    +DR: DR0 (ADR DR3)
    +DR: US915 DR3 SF7 BW125K  //adaptative data rate
    +DR: US915 DR0 SF10 BW125K //setted data rate*/
    //get substring with selected Dara Rate information
    pstr=strstr(recv_buf, "+DR:");//searchs for "+DR:"
    if(pstr!= NULL){
      //get the data 
      strncpy(substr,(pstr+5),5);//Copy "DR0 "
      if (substr[3]<48){//only 1 digit data rate: ex"DR9"
         substr[3]='\0';//add the end of the string
         }
         else{ substr[4]='\0';}//2 digit data rate:"DR15"
      pstr=strstr(recv_buf, substr);//Searchs for "DR0 " in first time
      pstr=strstr(pstr+4, substr);//Searchs for "DR0 " into second time
      if(pstr!= NULL){
             strncpy(substr,pstr,sizeof(substr)); //copy "DR0 SF10 BW125K" to buffer  
             substr[sizeof(substr)-1]='\0';//add the end of the string
            }
        }
    /*set scale value based on BW*/
    if(strstr(substr, "BW500")!= NULL){BW=BW500;}
    if(strstr(substr, "BW250")!= NULL){BW=BW250;}
    if(strstr(substr, "BW125")!= NULL){BW=BW125;}
	if(strstr(substr,"50kbps")!= NULL){BW=BW50kbps;}
    /*Set value of SF*/
    if(strstr(substr,"SF12")!= NULL){SF=SF12;}
    if(strstr(substr,"SF11")!= NULL){SF=SF12;}
    if(strstr(substr,"SF10")!= NULL){SF=SF12;}
    if(strstr(substr,"SF9")!= NULL){SF=SF12;}
    if(strstr(substr,"SF8")!= NULL){SF=SF12;}
    if(strstr(substr,"SF7")!= NULL){SF=SF12;}
	/*after obtaining the SF and BW, set the bitRate and txHead_time clas variables */  
	SF_BW_to_bitrate_txhead_time(SF,BW);  
    /*if pointers were provided, store the values     */
     if (!(pbitRate==NULL)){*pbitRate=bitRate;}//Stores the value if a pointer was provided
     if (!(ptxHead_time==NULL)){*ptxHead_time=txHead_time;}//Stores the value if a pointer was provided
    /*return the command time*/       
    return time_cmd;
}
unsigned int LoRaE5Class::setFrequencyBand(_physical_type_t physicalType){
    unsigned int time_cmd=0;
    cmd[0]='\0';//reset the string position
    cmd_resp_ack[0]='\0';//reset the string position
	if (physicalType>UNINIT){
       sprintf(cmd, "AT+DR= %s\r\n", physTypeStr[physicalType]);
       sprintf(cmd_resp_ack, "%s", physTypeStr[physicalType]);
	   time_cmd=at_send_check_response(cmd,cmd_resp_ack,2*DEFAULT_TIMEWAIT,NULL);
	 }
	 /*Stores frequency band if command was ok*/
	if(time_cmd){ /*if command response ok, store the band set value*/
      if (physicalType==EU434){freq_band=434;}
      if (physicalType==EU868){freq_band=868;}
      if (physicalType==US915 or physicalType==US915HYBRID){freq_band=915;}
      if (physicalType==AU915){freq_band=915;}
      if (physicalType==AS923){freq_band=923;}  
      if (physicalType==CN470){freq_band=470;}  
      if (physicalType==KR920){freq_band=920;}  
      if (physicalType==CN470PREQUEL){freq_band=470;}  
      if (physicalType==KR920){freq_band=920;}
      if (physicalType==STE920){freq_band=920;}     
      }

return(time_cmd);	 
}
							   

unsigned int LoRaE5Class::setDataRate(_data_rate_t dataRate,
                               _physical_type_t physicalType) {
    unsigned int time_cmd=0;

    if ((dataRate <= UNINIT)) {
        #ifdef COMMAND_PRINT_TO_USER
        SerialUSB.print("\r\n!!Unknown datarate\n");
        #endif
        return 0;
    }
    // set frequency band first if was no set. If you change the frequency band, a rejoin will be requested to send messages in OOTA mode
	if (freq_band==0){time_cmd=+setFrequencyBand(physicalType); }
    // then set data rate
    cmd[0]='\0';//reset the string position
    sprintf(cmd, "AT+DR=%d\r\n", dataRate);
    time_cmd=+at_send_check_response(cmd,AT_NO_ACK,DEFAULT_TIMEWAIT,NULL);

    return time_cmd;
}
unsigned int LoRaE5Class::setSpreadFactor(_spreading_factor_t SF, _band_width_t BW,_physical_type_t physicalType){
_data_rate_t DR=DRNONE; /*To ensure you are setting a supported DR*/
unsigned int time_cmd=0;
     // set frequency band first if was no set. If you change the frequency band, a rejoin will be requested to send messages in OOTA mode
	if (freq_band==0){time_cmd=+setFrequencyBand(physicalType); }
	/*now set the spread factor*/
if ((physicalType==CN470)or(physicalType==KR920)or
    (physicalType==CN470PREQUEL)){
   if (((SF==SF12))and(BW==BW125)){DR=DR0;}
   if (((SF==SF11))and(BW==BW125)){DR=DR1;}
   if (((SF==SF10))and(BW==BW125)){DR=DR2;}
   if (((SF==SF9))and(BW==BW125)){DR=DR3;}
   if (((SF==SF8))and(BW==BW125)){DR=DR4;}
   if (((SF==SF7))and(BW==BW125)){DR=DR5;}     
   }
if ((physicalType==EU868)or(physicalType==CN779)or
    (physicalType==EU434)or(physicalType==AS923)or
    (physicalType==IN865)or(physicalType==RU864)or
    (physicalType==STE920)){
   if (((SF==SF12))and(BW==BW125)){DR=DR0;}
   if (((SF==SF11))and(BW==BW125)){DR=DR1;}
   if (((SF==SF10))and(BW==BW125)){DR=DR2;}
   if (((SF==SF9))and(BW==BW125)){DR=DR3;}
   if (((SF==SF8))and(BW==BW125)){DR=DR4;}
   if (((SF==SF7))and(BW==BW125)){DR=DR5;}
   if (((SF==SF7))and(BW==BW250)){DR=DR6;}
   if (((SF==NULL))and(BW==BW50kbps)){DR=DR7;}      
   }
 if ((physicalType==US915)or(physicalType==US915HYBRID)or
     (physicalType==AU915)or(physicalType==US915OLD)){
   if (((SF==SF10))and(BW==BW125)){DR=DR0;}
   if (((SF==SF9))and(BW==BW125)){DR=DR1;}
   if (((SF==SF8))and(BW==BW125)){DR=DR2;}
   if (((SF==SF7))and(BW==BW125)){DR=DR3;}
   if (((SF==SF8))and(BW==BW500)){DR=DR4;}
   if (((SF==SF12))and(BW==BW500)){DR=DR8;}  
   if (((SF==SF11))and(BW==BW500)){DR=DR9;} 
   if (((SF==SF10))and(BW==BW500)){DR=DR10;} 
   if (((SF==SF9))and(BW==BW500)){DR=DR11;} 
   if (((SF==SF8))and(BW==BW500)){DR=DR12;}
   if (((SF==SF7))and(BW==BW500)){DR=DR13;}
   if(physicalType==US915){ /*special case*/
    if (((SF==SF7))and(BW==BW125)){DR=DR5;}
    if (((SF==SF8))and(BW==BW500)){DR=DR6;}
   }
}  

if(DR!=DRNONE){
    #ifdef COMMAND_PRINT_TO_USER
    SerialUSB.print("\r\nSetting Data Rate according to Spread factor (SF), BandWidth (BW) and LoRaWAN Standard Band Plan selected");
    #endif 
    time_cmd=setDataRate(DR,UNINIT); /*Set only DR.*/
	SF_BW_to_bitrate_txhead_time(SF,BW); /*update the variables values*/
   }
else{
    #ifdef COMMAND_PRINT_TO_USER
    SerialUSB.print("\r\n ERORR: The combination of Spread factor (SF) and BandWidth (BW) is not supported LoRaWAN Standard Band Plan.Therefore, no Data rate was seeted. Example: For EU868 there is ot Data Rate defined for a SF=12 and BW=500K");
    #endif 
    time_cmd=0;
    }
return(time_cmd);
}
                                  
unsigned int LoRaE5Class::setPower(short power) {
    unsigned int time_cmd=0;
    cmd_resp_ack[0]='\0';
    cmd[0]='\0';//reset the string position
    sprintf(cmd, "AT+POWER=%d\r\n", power);
    sprintf(cmd_resp_ack, "+POWER: %d", power);
    time_cmd=at_send_check_response(cmd,cmd_resp_ack,DEFAULT_TIMEWAIT,NULL);// returns 0 if the command was not ACK by Gateway.
    if (time_cmd>0){txPower=power;}//If command was ACK properly, we store the value
    return(time_cmd);
}

unsigned int LoRaE5Class::setPort(unsigned char port) {
    unsigned int time_cmd=0;
    cmd_resp_ack[0]='\0';
    cmd[0]='\0';//reset the string position
    sprintf(cmd, "AT+PORT=%d\r\n", port);
    sprintf(cmd_resp_ack, "+PORT: %d", port);
    time_cmd=at_send_check_response(cmd,cmd_resp_ack,DEFAULT_TIMEWAIT,NULL);// returns 0 if the command was not ACK by Gateway.
    return(time_cmd);
}

unsigned int LoRaE5Class::setAdaptiveDataRate(bool command) {
    unsigned int time_cmd=0;
    cmd_resp_ack[0]='\0';
    cmd[0]='\0';//reset the string position
    if (command){
        sprintf(cmd,"AT+ADR=ON\r\n");
        sprintf(cmd_resp_ack, "+ADR: ON");
        }
    else{
        sprintf(cmd,"AT+ADR=OFF\r\n");
        sprintf(cmd_resp_ack, "+ADR: OFF");
        }
    time_cmd=at_send_check_response(cmd,cmd_resp_ack,DEFAULT_TIMEWAIT,NULL);// returns 0 if the command was not ACK by Gateway.
    /*If command was acknowledged, update the adaptative_DR variable */
	if (time_cmd>0){adaptative_DR=command;}
	return(time_cmd);
}

unsigned int LoRaE5Class::getChannel(void) {
    unsigned int time_cmd=0;
    cmd_resp_ack[0]='\0';
    cmd[0]='\0';//reset the string position
    sprintf(cmd,"AT+CH\r\n");
    sprintf(cmd_resp_ack, "+ADR: ON");
    time_cmd=at_send_check_response(cmd,"+CH:",DEFAULT_TIMEWAIT,NULL);// returns 0 if the command was not ACK by Gateway.
    time_cmd=+at_send_check_response(cmd,cmd_resp_ack,DEFAULT_TIMEWAIT,NULL);// returns 0 if the command was not ACK by Gateway.
    return(time_cmd);
}
unsigned int LoRaE5Class::setChannel(unsigned char channel) {
    unsigned int time_cmd=0;
    cmd_resp_ack[0]='\0';
    cmd[0]='\0';//reset the string position
    sprintf(cmd, "AT+CH=%d\r\n", channel);
    sprintf(cmd_resp_ack, "+CH: %d", channel); //"+CH: %d" //to know is ok
    time_cmd=at_send_check_response(cmd,AT_NO_ACK,DEFAULT_TIMEWAIT,NULL);// returns 0 if the command was not ACK by Gateway.
    return(time_cmd);
}
unsigned int LoRaE5Class::setChannel(unsigned char channel, float frequency) {
    unsigned int time_cmd=0;
    cmd_resp_ack[0]='\0';
    cmd[0]='\0';//reset the string position
    if (frequency == 0){
        sprintf(cmd, "AT+CH=0,%d,0\r\n", channel);
        sprintf(cmd_resp_ack, "+CH: %d,DR:0\r\n", channel);
        }
    else{
        sprintf(cmd, "AT+CH=%d,%d.%d\r\n", channel, (short)frequency,
                short(frequency * 10) % 10);
       sprintf(cmd_resp_ack, "+CH: %d,DR:0\r\n", channel);
      }
    time_cmd=at_send_check_response(cmd,cmd_resp_ack,DEFAULT_TIMEWAIT,NULL);// returns 0 if the command was not ACK by Gateway.
    return(time_cmd);
}

unsigned int LoRaE5Class::setChannel(unsigned char channel, float frequency,
                              _data_rate_t dataRata) {
    unsigned int time_cmd=0;
    if (channel > 16) channel = 16;

    memset(cmd, 0, 32);
    sprintf(cmd, "AT+CH=%d,%d.%d,%d\r\n", channel, (short)frequency,
            short(frequency * 10) % 10, dataRata);
    time_cmd=at_send_check_response(cmd,AT_NO_ACK,DEFAULT_TIMEWAIT,NULL);// returns 0 if the command was not ACK by Gateway.
    if(time_cmd>0){freq_band=frequency;}
    return(time_cmd);
}

unsigned int LoRaE5Class::setChannel(unsigned char channel, float frequency,
                              _data_rate_t dataRataMin,
                              _data_rate_t dataRataMax) {
    unsigned int time_cmd=0;

    if (channel > 16) channel = 16;

    memset(cmd, 0, 32);
    sprintf(cmd, "AT+CH=%d,%d.%d,%d,%d\r\n", channel, (short)frequency,
            short(frequency * 10) % 10, dataRataMin, dataRataMax);
            
    time_cmd=at_send_check_response(cmd,AT_NO_ACK,DEFAULT_TIMEWAIT,NULL);// returns 0 if the command was not ACK by Gateway.
    return time_cmd;
}

unsigned int LoRaE5Class::transferPacket(char *buffer, unsigned int timeout) {
    unsigned char length = strlen(buffer);
    int i;
    unsigned int time_ret;
    #ifdef COMMAND_PRINT_TO_USER
    cmd[0]='\0';//reset the string
    sprintf(cmd,"\r\nSending %i characters to a LoRa Gateway",(int)length);
    SerialUSB.print(cmd);
    #endif
    cmd[0]='\0';//reset the string
    sprintf(cmd,"AT+MSG=\"%s\"\r\n",buffer);
    time_ret=at_send_check_response(cmd,"Done",timeout,NULL);
    return time_ret;
}

unsigned int LoRaE5Class::transferPacket(unsigned char *buffer, unsigned char length,
                                  unsigned int timeout) {
    int i;
    unsigned int time_ret;
    #ifdef COMMAND_PRINT_TO_USER
    cmd[0]='\0';//reset the string
    sprintf(cmd,"\r\nSending %i bytes to a LoRa Gateway",(int)length);
    SerialUSB.print(cmd);
    #endif
    cmd[0]='\0';//reset the string size
    sprintf(cmd+strlen(cmd),"AT+MSGHEX=\"");//name of the command
    for ( i = 0; i < length; i++) { sprintf(cmd+strlen(cmd), "%02x", buffer[i]);}//add the characters in hex format
    sprintf(cmd+strlen(cmd),"\"\r\n");//end of command
    time_ret=at_send_check_response(cmd,"Done",timeout,NULL);
    return time_ret;
}

unsigned int LoRaE5Class::transferPacketWithConfirmed(char *buffer,
                                               unsigned int timeout) {
    unsigned char length = strlen(buffer);
    int i;
    unsigned int time_ret;
    #ifdef COMMAND_PRINT_TO_USER
    cmd[0]='\0';//reset the string
    sprintf(cmd,"\r\nSending %i characters to a LoRa Gateway and waits for ACK",(int)length);
    SerialUSB.print(cmd);
    #endif
    cmd[0]='\0';//reset the string
    sprintf(cmd,"AT+CMSG=\"%s\"\r\n",buffer);
    time_ret=at_send_check_response(cmd,"Wait ACK",timeout,NULL);
	/*wait to Open reception Window*/
	time_ret=+RXWIN1_DELAY;
	/*turn off module to optimize performance*/
	delay(RXWIN1_DELAY);/**/
	time_ret=+at_send_check_response(NULL,"Done",timeout,NULL);
    return time_ret;
}

unsigned int LoRaE5Class::transferPacketWithConfirmed(unsigned char *buffer,
                                               unsigned char length,
                                               unsigned int timeout) {
    int i;
    unsigned int time_ret;
    #ifdef COMMAND_PRINT_TO_USER
    cmd[0]='\0';//reset the string
    sprintf(cmd,"\r\nSending %i bytes to a LoRa Gateway and waits for ACK",(int)length);
    SerialUSB.print(cmd);
    #endif
    cmd[0]='\0';//reset the string size
    sprintf(cmd+strlen(cmd),"AT+CMSGHEX=\"");//name of the command
    for ( i = 0; i < length; i++) { sprintf(cmd+strlen(cmd), "%02x", buffer[i]);}//add the characters in hex format
    sprintf(cmd+strlen(cmd),"\"\r\n");//end of command
    time_ret=at_send_check_response(cmd,"Wait ACK",timeout,NULL);
    /*wait to Open reception Window*/
	time_ret=+RXWIN1_DELAY;
	/*sleep the device until new reception window*/
	//setDeviceLowPower();
	delay(RXWIN1_DELAY);
	/*Awakes the device until new reception window*/
	//setDeviceWakeUp();
	time_ret=+at_send_check_response(NULL,"Done",timeout,NULL);
    return time_ret;
}

short LoRaE5Class::receivePacket(char *buffer, short length, short *rssi,unsigned int timeout) {
    char *ptr;
    short number = 0;
    unsigned int time_ret;
    //call to recieve packet during a time window time window
    readBuffer(recv_buf,sizeof(recv_buf),timeout);
    //parse the packet content
    ptr = strstr(recv_buf, "RSSI ");
    if (ptr)
        *rssi = atoi(ptr + 5);
    else
        *rssi = -255;

    ptr = strstr(recv_buf, "RX: \"");
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

    ptr = strstr(recv_buf, "MACCMD: \"");
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

    return number;
}

unsigned int LoRaE5Class::transferProprietaryPacket(char *buffer,
                                             unsigned int timeout) {
    unsigned char length = strlen(buffer);
    int i;
    unsigned int time_ret;
    #ifdef COMMAND_PRINT_TO_USER
    cmd[0]='\0';//reset the string
    sprintf(cmd,"\r\nSending %i characters in LoRaWAN proprietary frames format to a LoRa Gateway",(int)length);
    SerialUSB.print(cmd);
    #endif
    cmd[0]='\0';//reset the string
    sprintf(cmd,"AT+PMSG=\"%s\"\r\n",buffer);
    time_ret=at_send_check_response(cmd,"Done",timeout,NULL);
    return time_ret; 
}

unsigned int LoRaE5Class::transferProprietaryPacket(unsigned char *buffer,
                                             unsigned char length,
                                             unsigned int timeout) {
    int i;
    unsigned int time_ret;
    #ifdef COMMAND_PRINT_TO_USER
    cmd[0]='\0';//reset the string
    sprintf(cmd,"\r\nSending %i bytes in LoRaWAN proprietary frames format to a LoRa Gateway",(int)length);
    SerialUSB.print(cmd);
    #endif
    cmd[0]='\0';//reset the string size
    sprintf(cmd+strlen(cmd),"AT+PMSGHEX=\"");//name of the command
    for ( i = 0; i < length; i++) { sprintf(cmd+strlen(cmd), "%02x", buffer[i]);}//add the characters in hex format
    sprintf(cmd+strlen(cmd),"\"\r\n");//end of command
    time_ret=at_send_check_response(cmd,"Done",timeout,NULL);
    return time_ret;
}

unsigned int LoRaE5Class::setUnconfirmedMessageRepeatTime(unsigned char time) {
    unsigned int time_ret=0;
    //ensure a proper value
    if (time > 15)
        time = 15;
    else if (time == 0)
        time = 1;

    cmd[0]='\0';
    cmd_resp_ack[0]='\0';
    sprintf(cmd, "AT+REPT=%d\r\n", time);
    sprintf(cmd_resp_ack, "+REPT=%d\r\n", time);
    time_ret=at_send_check_response(cmd,cmd_resp_ack,DEFAULT_TIMEWAIT,NULL);
    return time_ret;
}

unsigned int LoRaE5Class::setConfirmedMessageRetryTime(unsigned char time) {
    unsigned int time_ret=0;
    if (time > 15)
        time = 15;
    else if (time == 0)
        time = 1;
        
    cmd[0]='\0';
    cmd_resp_ack[0]='\0';
    sprintf(cmd, "AT+RETRY=%d\r\n", time);
    sprintf(cmd_resp_ack, "+RETRY=%d\r\n", time);
    time_ret=at_send_check_response(cmd,cmd_resp_ack,DEFAULT_TIMEWAIT,NULL);
    return time_ret;
}

unsigned int LoRaE5Class::getReceiveWindowFirst(void) {
    unsigned int time_ret=0;
    cmd[0]='\0';
    sprintf(cmd, "AT+RXWIN1TRY=%d\r\n", time);
    time_ret=at_send_check_response(cmd,AT_NO_ACK,DEFAULT_TIMEWAIT,NULL);
    return time_ret;
}

unsigned int LoRaE5Class::setReceiveWindowFirst(bool command) {
    unsigned int time_ret=0;
    cmd[0]='\0';
    if (command)
        sprintf(cmd, "AT+RXWIN1=ON");
    else
        sprintf(cmd, "AT+RXWIN1=OFF");
    time_ret=at_send_check_response(cmd,AT_NO_ACK,DEFAULT_TIMEWAIT,NULL);
    return time_ret;
}
unsigned int LoRaE5Class::setReceiveWindowFirst(unsigned char channel,
                                         float frequency) {
     unsigned int time_ret=0;
    //    if(channel > 16) channel = 16;
    cmd[0]='\0';
    if (frequency == 0)
        sprintf(cmd, "AT+RXWIN1=%d,0\r\n", channel);
    else
        sprintf(cmd, "AT+RXWIN1=%d,%d.%d\r\n", channel, (short)frequency,
                short(frequency * 10) % 10);
    time_ret=at_send_check_response(cmd,AT_NO_ACK,DEFAULT_TIMEWAIT,NULL);
    return time_ret;
}

unsigned int LoRaE5Class::setReceiveWindowSecond(float frequency,
                                          _data_rate_t dataRate) {
     unsigned int time_ret=0;
    cmd[0]='\0';
    sprintf(cmd, "AT+RXWIN2=%d.%d,%d\r\n", (short)frequency,
            short(frequency * 10) % 10, dataRate);
    time_ret=at_send_check_response(cmd,AT_NO_ACK,DEFAULT_TIMEWAIT,NULL);
    return time_ret;
}

unsigned int LoRaE5Class::setReceiveWindowSecond(float frequency,
                                          _spreading_factor_t spreadingFactor,
                                          _band_width_t bandwidth) {
    unsigned int time_ret=0;
    cmd[0]='\0';
    sprintf(cmd, "AT+RXWIN2=%d.%d,%d,%d\r\n", (short)frequency,
            short(frequency * 10) % 10, spreadingFactor, bandwidth);
    time_ret=at_send_check_response(cmd,AT_NO_ACK,DEFAULT_TIMEWAIT,NULL);
    return time_ret;
}

unsigned int LoRaE5Class::setReceiveWindowDelay(_window_delay_t command,
                                         unsigned short _delay) {
    unsigned int time_ret=0;
    cmd[0]='\0';
    if (command == RECEIVE_DELAY1)
        sprintf(cmd, "AT+DELAY=RX1,%d\r\n", _delay);
    else if (command == RECEIVE_DELAY2)
        sprintf(cmd, "AT+DELAY=RX2,%d\r\n", _delay);
    else if (command == JOIN_ACCEPT_DELAY1)
        sprintf(cmd, "AT+DELAY=JRX1,%d\r\n", _delay);
    else if (command == JOIN_ACCEPT_DELAY2)
        sprintf(cmd, "AT+DELAY=JRX2,%d\r\n", _delay);
        
    time_ret=at_send_check_response(cmd,AT_NO_ACK,DEFAULT_TIMEWAIT,NULL);
    return time_ret;
}

unsigned int LoRaE5Class::setClassType(_class_type_t type) {
    unsigned int time_cmd=0;
    cmd[0]='\0';//reset the string position
    cmd_resp_ack[0]='\0';//reset the string position
    char class_X;
    if (type == CLASS_C){
        class_X='C';
        }
    else{ if (type == CLASS_A){
           class_X='A';
           }
           else { 
              #ifdef COMMAND_PRINT_TO_USER
              SerialUSB.print( "\r\nInvalid value for setClassType");
              #endif 
              return 0;
             } 
          }    
    sprintf(cmd, "AT+CLASS=%c\r\n", class_X);
    sprintf(cmd_resp_ack, "CLASS: %c", class_X);//must return the BW in ks
    time_cmd=+at_send_check_response(cmd,cmd_resp_ack,DEFAULT_TIMEWAIT,NULL);
}

//
// set the JOIN mode to either LWOTAA or LWABP
//
unsigned int LoRaE5Class::setDeviceMode(_device_mode_t mode) {
    int timeout = 1000;
    unsigned int time_cmd=0;//Returned time to succesfully execute a command. 
    cmd[0]='\0';//reset the string
    if (mode == LWABP){
        sprintf(cmd,"AT+MODE=LWABP\r\n");
        time_cmd=at_send_check_response(cmd,"+MODE: LWABP",timeout,NULL);
      }
    else if (mode == LWOTAA){
        sprintf(cmd,"AT+MODE=LWOTAA\r\n");
        time_cmd=at_send_check_response(cmd,"+MODE: LWOTAA",timeout,NULL);
       }
    else {
              #ifdef COMMAND_PRINT_TO_USER
                 SerialUSB.print("\r\nBad command to setDeviceMode.");   
               #endif  
    }
    return time_cmd;//Returned time to succesfully execute a command. 0 if the command was not ACK by Gateway.
}

//
//  JOIN with the application
//
//  setDeviceMode should have been called before this.
unsigned int LoRaE5Class::setOTAAJoin(_otaa_join_cmd_t command,
                               unsigned int timeout) {
    unsigned int time_cmd=0;//Returned time to succesfully execute a command. 
    if (command == JOIN){
        sprintf(cmd,"AT+JOIN\r\n");
        time_cmd=at_send_check_response(cmd,"+JOIN: Network joined",timeout,NULL);
        }
    else if (command == FORCE){
        sprintf(cmd,"AT+JOIN\r\n");
        time_cmd=at_send_check_response(cmd,"+JOIN: Network joined",timeout,NULL);
       }
    else {
          #ifdef COMMAND_PRINT_TO_USER
            SerialUSB.print("Bad command to setOTAAJoin\n");
           #endif 
           }
    return time_cmd;//Returned time to succesfully execute a command. 0 if the command was not ACK by Gateway.
}

unsigned int LoRaE5Class::setDeviceBaudRate(_baudrate_bps_supported baud_rate ) {
    unsigned int time_cmd=0;//Returned time to succesfully execute a command. 
    cmd[0]='\0';//reset the string position
	cmd_resp_ack[0]='\0';//reset the string position
    sprintf(cmd,"AT+UART=BR, %i\r\n",baud_rate);
	sprintf(cmd_resp_ack,"%i",baud_rate);
	bool mode=lowpower_auto;
	lowpower_auto=true;//set temporaly to true in order to ensure a proper working with the device
    time_cmd=at_send_check_response(cmd,cmd_resp_ack,DEFAULT_TIMEWAIT,NULL);
	setDeviceReset();
    lowpower_auto=mode;//return to original value
	SerialLoRa.end();//end before initing a serial port
	initSerial(baud_rate);
	delay(10);//delay 6 mseconds for next command
    return(time_cmd);
}
unsigned int LoRaE5Class::setDeviceLowPower(unsigned int time_to_wakeup_ms) {
    unsigned int time_cmd=0;//Returned time to succesfully execute a command. 
    cmd[0]='\0';//reset the string position
	if (time_to_wakeup_ms==0){sprintf(cmd,"AT+LOWPOWER\r\n");}
	else                   {sprintf(cmd,"AT+LOWPOWER=%i\r\n",time_to_wakeup_ms);}
    time_cmd=at_send_check_response(cmd,"+LOWPOWER: SLEEP",DEFAULT_TIMEWAIT,NULL);
    return(time_cmd);
}
unsigned int LoRaE5Class::setDeviceLowPowerAutomode(bool mode) {
    unsigned int time_cmd=0;//Returned time to succesfully execute a command. 
    cmd[0]='\0';//reset the string position
	if (mode){sprintf(cmd,"AT+LOWPOWER=AUTOON\r\n");}
	else     {sprintf(cmd,"AT+LOWPOWER=AUTOOFF\r\n");}
	if (mode){time_cmd=at_send_check_response(cmd,"AUTOON",DEFAULT_TIMEWAIT,NULL);}
	else     {lowpower_auto=true;
		      time_cmd=at_send_check_response(cmd,"AUTOOFF",DEFAULT_TIMEWAIT,NULL);
			  lowpower_auto=false;}
	if (time_cmd>0){lowpower_auto=mode;}
    return(time_cmd);
}
unsigned int LoRaE5Class::setDeviceWakeUp(void) {
    unsigned int time_cmd=0;//Returned time to succesfully execute a command. 
    cmd[0]='\0';//reset the string position
    sprintf(cmd,"AT+\r\n");
    time_cmd=at_send_check_response(cmd,"WAKEUP",DEFAULT_TIMEWAIT,NULL);
    return(time_cmd);
}
//
// Reset the LoRa module. Does not factory reset
//
unsigned int LoRaE5Class::setDeviceReset(void) {
    unsigned int time_cmd;
    cmd[0]='\0';//reset the string position
    sprintf(cmd,"AT+RESET\r\n");
    time_cmd=at_send_check_response(cmd,"+RESET: OK",DEFAULT_TIMEWAIT,NULL);
    return(time_cmd);
}

//
//  Factory reset the module.
//
unsigned int LoRaE5Class::setDeviceDefault(void) {
    cmd[0]='\0';//reset the string position
    unsigned int time_cmd;
    sprintf(cmd,"AT+FDEFAULT=RISINGHF\r\n");
    time_cmd=at_send_check_response(cmd,"+FDEFAULT: OK",DEFAULT_TIMEWAIT,NULL);
    return(time_cmd);
}

unsigned int LoRaE5Class::initP2PMode(unsigned short frequency,
                               _spreading_factor_t spreadingFactor,
                               _band_width_t bandwidth,
                               unsigned char txPreamble,
                               unsigned char rxPreamble, short power) {
    unsigned int time_cmd=0;//Returned time to succesfully execute a command.  
    //set device in test mode
    cmd[0]='\0';//reset the string position
    sprintf(cmd, "AT+MODE=TEST\r\n");
    time_cmd=+at_send_check_response(cmd,"TEST",DEFAULT_TIMEWAIT,NULL);
    //set device test mode configuration
    cmd[0]='\0';//reset the string position
    sprintf(cmd, "AT+TEST=RFCFG,%d,%d,%d,%d,%d,%d\r\n", frequency,
            spreadingFactor, bandwidth, txPreamble, rxPreamble, power);
    time_cmd=+at_send_check_response(cmd,AT_NO_ACK,DEFAULT_TIMEWAIT,NULL);
    //allow the reception of messages
    cmd[0]='\0';//reset the string position
    sprintf(cmd, "AT+TEST=RXLRPKT\r\n");
    time_cmd=+at_send_check_response(cmd,"RXLRPKT",DEFAULT_TIMEWAIT,NULL);
    return(time_cmd);
}

unsigned int LoRaE5Class::transferPacketP2PMode(char *buffer) {
    unsigned int time_ret;
    unsigned char length=strlen(buffer);
    #ifdef COMMAND_PRINT_TO_USER
    cmd[0]='\0';//reset the string
    sprintf(cmd,"\r\nSending %i characters to a another LoRa End Node",(int)length);
    SerialUSB.print(cmd);
    #endif
    cmd[0]='\0';//reset the string
    sprintf(cmd,"AT+TXLRSTR=\"%s\"\r\n",buffer);
    time_ret=at_send_check_response(cmd,"Done",DEFAULT_TIMEWAIT,NULL);
    return time_ret;
}

unsigned int LoRaE5Class::transferPacketP2PMode(unsigned char *buffer,
                                         unsigned char length) {
    int i;
    unsigned int time_ret;
    #ifdef COMMAND_PRINT_TO_USER
    cmd[0]='\0';//reset the string
    sprintf(cmd,"\r\nSending %i bytes to a another LoRa End Node",(int)length);
    SerialUSB.print(cmd);
    #endif
    cmd[0]='\0';//reset the string size
    sprintf(cmd+strlen(cmd),"AT+TEST=TXLRPKT,\"");//name of the command
    for ( i = 0; i < length; i++) { sprintf(cmd+strlen(cmd), "%02x", buffer[i]);}//add the characters in hex format
    sprintf(cmd+strlen(cmd),"\"\r\n");//end of command
    time_ret=at_send_check_response(cmd,"Done",DEFAULT_TIMEWAIT,NULL);
    return time_ret;    
}

short LoRaE5Class::receivePacketP2PMode(unsigned char *buffer, short length,
                                         short *rssi, unsigned int timeout) {
    char *ptr;
    short number;
    unsigned int time_ret=0;
    char ch;
    //clean reception buffer after starting with reception:
    strncpy(recv_buf," ",sizeof(recv_buf));//fill the content with white spaces
    strlcpy(recv_buf," ",1);//Manually indicates end of string
    /*clean the serial port before issuing the command*/
    while (SerialLoRa.available() > 0){ ch = SerialLoRa.read();}//clean the read buffer
    /*parse the content of the rx message to get information*/
    ptr = strstr(recv_buf, "LEN");
    if (ptr)
        number = atoi(ptr + 4);
    else
        number = 0;

    if (number <= 0) return 0;

    ptr = strstr(recv_buf, "RSSI:");
    if (ptr)
        *rssi = atoi(ptr + 5);
    else
        *rssi = -255;

    ptr = strstr(recv_buf, "RX \"");
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

    return number;
}
/*return transmition time in ms*/
float LoRaE5Class::getTransmissionTime(unsigned int payload_size){
  return(txHead_time+((float)(payload_size)*8*1000/bitRate));
  }
/********************************************************************************/  
float LoRaE5Class::getTransmissionPower(unsigned int payload_size, float tx_period_s){
  float tx_power_consumption,tx_time,rx_time;
  /*Get the power used*/
  if (freq_band>800){
    if (txPower==16)              {tx_power_consumption=TXPOWER_16dBm_mA;}
	if (txPower==14)              {tx_power_consumption=TXPOWER_14dBm_mA;}
	if (txPower==12)              {tx_power_consumption=TXPOWER_12dBm_mA;}
	if (txPower==10)              {tx_power_consumption=TXPOWER_10dBm_mA;}
	if (txPower== 8)              {tx_power_consumption=TXPOWER_08dBm_mA;}
	if (txPower== 6)              {tx_power_consumption=TXPOWER_06dBm_mA;}
	if (txPower== 4)              {tx_power_consumption=TXPOWER_04dBm_mA;}
	if (txPower== 2)              {tx_power_consumption=TXPOWER_02dBm_mA;}
	if (txPower== 0)              {tx_power_consumption=TXPOWER_00dBm_mA;}
    }
    /*Get the Tx time in ms*/
     tx_time=getTransmissionTime(payload_size);
	 /*Stimate rx timein ms*/
	 rx_time=txHead_time+50;//*add 50 ms additional to txheadtime. This was meassured during testing.
    /*perform the stimation*/
	/*Considers the time to Tx, time to Rx and power consuptiom while the module sleeps*/
   return(((tx_power_consumption*tx_time+ RXPOWER_mA*(rx_time))/1000)*(1/tx_period_s)+SLEEPPOWER_mA);//returns a value in mAh
  }

unsigned int LoRaE5Class::Debug(_debug_level value){
   unsigned int time_cmd;
   char val[8];
   if (value==lora_DEBUG){sprintf(val,"DEBUG");}
   if (value==lora_INFO){sprintf(val,"INFO");}
   if (value==lora_WARN){sprintf(val,"WARN");}
   if (value==lora_FATAL){sprintf(val,"FATAL");}
   if (value==lora_PANIC){sprintf(val,"PANIC");}
   if (value==lora_QUIET){sprintf(val,"QUIET");}
   //***********
   cmd[0]='\0';//reset the string position
   sprintf(cmd, "AT+LOG=%s\r\n",val);
   time_cmd=+at_send_check_response(cmd,AT_NO_ACK,DEFAULT_TIMEWAIT,NULL);
   return(time_cmd);
}         

unsigned int LoRaE5Class::readbitRate(void){
        return(bitRate);
      }

      
float LoRaE5Class::readtxHead_time(void){
      return(txHead_time);
      }            
      
LoRaE5Class lora;
