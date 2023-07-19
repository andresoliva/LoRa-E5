//This a custom version of the Seed example for their Grove LoRa_E5 sensor
//That works with Seeeduino XIAO, Seeeduino XIAO expantion board and DHT11 Temperature and  Humidity Sensor
//But adaptated for the Arduino Nano 33 BLE Sense connected to a Grove LoRa_E5 sensor
//Original code; https://wiki.seeedstudio.com/Grove_LoRa_E5_New_Version/#software-code
//Working with Arduino 1.8.14 IDE and forward
//Modified by: Andres Oliva Trevisan
#include <Arduino.h>
#include <stdarg.h> //for using 
//--------------------------
#include "LoRaE5.h"   //main LoRa lib
//---------------------


#define PRINT_TO_USER //To allow printing 

//WARNING. DO NOT SET
#define SET_CUSTOM_DEVUI// COMMENTED.// DO NOT UNCOMMENT UNLESS YOU HAVE THE ORIGINAL ID//
//Information regarding the following parameters https://www.thethingsnetwork.org/docs/lorawan/architecture/

//2B7E151628AED2A6ABF7158809CF4F3C original //2B7E151628AED2A609CF4F3CABF71588
#ifdef SET_CUSTOM_DEVUI
#define LoRa_DEVEUI_CUSTOM       "2CF7F1C0440004A2" //Custom key for this App. You can generate one at https://www.thethingsnetwork.org/docs/devices/registration/
#endif
#define LoRa_APPKEY              "2B7E151628AED2A609CF4F3CABF71588" /*Custom key for this App*/
#define LoRa_FREQ_standard       EU868   /*International frequency band. see*/
#define LoRa_DR                  DR0     /*DR5=5.2kbps //data rate. see at https://www.thethingsnetwork.org/docs/lorawan/regional-parameters/    */
#define LoRa_DEVICE_CLASS        CLASS_A /*CLASS_A for power restriction/low power nodes. Class C for other device applications */
#define LoRa_PORT_BYTES          8       /*node Port for binary values to send*/
#define LoRa_PORT_STRING         7       /*node Port for string messagesto send*/
#define LoRa_POWER               14      /*node Tx (Transmition) power*/
#define LoRa_CHANNEL             2       /*Node selected Tx channel. Default is 0, we use 2 to show only to show how to set up*/
#define LoRa_ADR_FLAG            false   /*ADR(Adaptative Dara Rate) status flag (True or False)*/
/*Used only for P2P communication*/
#define Tx_delay_s               9.5     /*delay between transmitions expressed in seconds*/
/*Used only for P2P communication*/
#define LoRa_FREQ                868     /*Standard_freq_band*/
#define LoRa_RF_BW               BW125
#define LoRa_RF_SF               SF12
#define LoRa_Tx_preamble_size    10    /*Standard Amounts of btyes (15) used in Tx payload. Do Not change unless you read the docummentation*/
#define LoRa_Rx_preamble_size    15    /*Standard Amounts of btyes (10) used in Rx payload. Do Not change unless you read the docummentation*/

//DEMO PARAMETERS
//you can get LoRa_head_tx_time in https://avbentem.github.io/airtime-calculator/ttn/eu868/0

#define PAYLOAD_FIRST_TX      10   /*bytes to send into first packet*/
#define PAYLOAD_SECOND_TX     50   /*bytes to send. 50 is the max suggested to avoid the 64 bytes packet restriction de*/
#define Tx_and_ACK_RX_timeout 6000 /*6000 for SF12,3500 for SF11,3200 for SF11, 2000 for SF9/8/7. All examples consering 50 bytes payload and BW125*/     


unsigned char buffer_binary[128] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,16,17,18,19,20};
unsigned char buffer_p2p[128] = {0xef, 0xff, 0x55, 3, 4, 5, 6, 7, 8, 9,};
char buffer_char[50] = "I am sending this message to a LoRa Gateway.";/**/
char char_temp[256];//256 is ok size for recieving command data

unsigned int LoRa_bps=1;//5470bytes per second
float LoRa_head_tx_time=1; //46.3time in ms to transmit only the header of the packet

void setup(void){
  SerialUSB.begin(115200);
  lora.init(); // lora.init(8,5);
  //for p2p
  //lora.initP2PMode(LoRa_FREQ, LoRa_RF_SF, LoRa_RF_BW, LoRa_Tx_preamble_number, LoRa_Rx_preamble_number, LoRa_POWER);
  //for gateway communication:
#ifdef SET_CUSTOM_DEVUI
  lora.setId(NULL, LoRa_DEVEUI_CUSTOM, NULL);//WARNING: If you run this command, you will change FOREVER the defaultd value that the fabricator has asigned to the module.
#endif
  /*First get device EUI for later printing*/
  lora.getId(char_temp,100); /*100 ms is more than enough to get a response from the module*/
  /*set up device. You must set up all your parameters BEFORE Joining.
    If you make any change, you should join again the network for proper working*/
  lora.setDeviceMode(LWOTAA);//LWOTAA or LWABP
  lora.setDataRate((_data_rate_t)LoRa_DR, (_physical_type_t)LoRa_FREQ_standard);
  lora.setKey(NULL, NULL, LoRa_APPKEY); /*Only App key is seeted when using OOTA*/
  lora.setClassType((_class_type_t) LoRa_DEVICE_CLASS); /*set device class*/
  lora.setPort(LoRa_PORT_BYTES);/*set the default port for transmiting data*/
  lora.setPower(LoRa_POWER); /*sets the Tx power*/
  lora.setChannel(LoRa_CHANNEL);/*selects the channel*/
  lora.setAdaptiveDataRate(LoRa_ADR_FLAG);/*Enables adaptative data rate*/
  //Tries to join the network
  while( (lora.setOTAAJoin(JOIN, 10000))==0);//will attempt to join network until the ends of time. https://www.thethingsnetwork.org/docs/lorawan/message-types/
  /*Now shows you the device actual DevEUI and AppEUI got at the begining
    NOTE: the AppEUI is the original one set by the developer in this example, even if you modify it*/
  #ifdef PRINT_TO_USER //defined in LoRaE5.h
  SerialUSB.print(char_temp);//to print the obtained characters
  #endif
  /*Get the bit Rate after join (because if Adaptative Data Rate is enable, DR could be changed after join )*/
  lora.getbitRate(&LoRa_bps,&LoRa_head_tx_time);
  /*POWER DOWN the LoRa module until next Tx (Transmition) cicle*/
  lora.setDeviceLowPower();
}

void loop(void)
{
  //lora.transferPacketP2PMode(buffer, 10);
  int time_tx1,time_tx2;
  /*Wake Up the LoRa module*/
  lora.setDeviceWakeUp();/*if the module is not in sleep state, this command does nothing*/
  /*-----------sending a string message*/
  lora.setPort(LoRa_PORT_STRING);/*set port configured in reception Gateway for expecting Strings*/
  lora.transferPacketWithConfirmed(buffer_char,Tx_and_ACK_RX_timeout);
  /*--------sending bytes message*/
  /*We send the same packet but with PAYLOAD_FIRST_TX and PAYLOAD_SECOND_TX bytes of payload using LoRa_FREQ_standard And
  Check https://avbentem.github.io/airtime-calculator/ttn/eu868/10 for knowing more how this stimations are made */
  lora.setPort(LoRa_PORT_BYTES);//set port for local reception
  time_tx1=lora.transferPacketWithConfirmed(buffer_binary,PAYLOAD_FIRST_TX,Tx_and_ACK_RX_timeout);
  time_tx2=lora.transferPacketWithConfirmed(buffer_binary,PAYLOAD_SECOND_TX,Tx_and_ACK_RX_timeout);
  /*Print---------- DEMO information to the USER*/
  #ifdef PRINT_TO_USER //defined in LoRaE5.h
   char_temp[0]='\0';
   sprintf(char_temp, "\r\nCalculated transmition time of messages with %i and %i bytes as payload: %.1f ms, %.1f ms.",PAYLOAD_FIRST_TX,PAYLOAD_SECOND_TX,
   LoRa_head_tx_time+(float)(PAYLOAD_FIRST_TX)*8*1000/LoRa_bps,  LoRa_head_tx_time+(float)(PAYLOAD_SECOND_TX)*8*1000/LoRa_bps);
   SerialUSB.print(char_temp);//to print the obtained characters
   char_temp[0]='\0';
   sprintf(char_temp, "\r\nCalculated diference between transmiting %i and %i bytes as payload: %.1f ms.",PAYLOAD_FIRST_TX,PAYLOAD_SECOND_TX,
   ((float)(PAYLOAD_SECOND_TX-PAYLOAD_FIRST_TX)*8)*1000/LoRa_bps);
   SerialUSB.print(char_temp);//to print the obtained characters
   char_temp[0]='\0';
   sprintf(char_temp, "\r\nMeasured diference between transmition  %i and %i bytes as payload: %i ms.",PAYLOAD_FIRST_TX,PAYLOAD_SECOND_TX,
   time_tx2-time_tx1);
   SerialUSB.print(char_temp);//to print the obtained characters
  #endif 
  /*POWER DOWN the LoRa module until next Tx Transmition (Tx) cicle*/
  lora.setDeviceLowPower();
  /*Sleeps until the next tx cicle*/
  delay((unsigned int)(Tx_delay_s*1000));/*Convert the value in seconds to miliseconds*/
}
