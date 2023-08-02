/*Example showing how use the functionalities of this library to stimate the transmition time of messages*/
/*Working with Arduino 1.8.14 IDE and forward*/
/*Created  by: Andres Oliva Trevisan*/
/*Repo: https://github.com/andresoliva/LoRa-E5/ */
#include <Arduino.h>
#include <stdarg.h> //for using 
//--------------------------
#include "LoRa-E5.h"   //main LoRa lib
//---------------------
/*USER OPTIONS*/
#define PRINT_TO_USER                   /*To allow the printing of characters using UART*/
#define PRINT_TO_USER_TIME_DIFFERENCE /*To allow the printing of time difference message*/
/*******************************************************************************************************/
/************************LORA SET UP*******************************************************************/
/*LoRa radio Init Parameters. Info:  https://www.thethingsnetwork.org/docs/lorawan/architecture/ */
#define LoRa_APPKEY              "2B7E151628AED2A609CF4F3CABF71588" /*Custom key for this App*/
#define LoRa_FREQ_standard       EU868   /*International frequency band. see*/
#define LoRa_DR                  DR4     /*DR5=5.2kbps //data rate. see at https://www.thethingsnetwork.org/docs/lorawan/regional-parameters/    */
#define LoRa_DEVICE_CLASS        CLASS_A /*CLASS_A for power restriction/low power nodes. Class C for other device applications */
#define LoRa_PORT                8       /*node Port for binary values to send, allowing the app to know it is recieving bytes*/
#define LoRa_POWER               14      /*Node Tx (Transmition) power*/
#define LoRa_CHANNEL             0       /*Node selected Tx channel. Default is 0, we use 2 to show only to show how to set up*/
#define LoRa_ADR_FLAG            false   /*ADR(Adaptative Dara Rate) status flag (True or False). Use False if your Node is moving*/
/*Time to wait for transmiting a packet again*/
#define Tx_delay_s               9.5     /*delay between transmitions expressed in seconds*/
/*Packet information*/
#define PAYLOAD_FIRST_TX         10   /*bytes to send into first packet*/
#define PAYLOAD_SECOND_TX        50   /*bytes to send. 50 is the max suggested to avoid the 64 bytes packet restriction de*/
#define Tx_and_ACK_RX_timeout    6000 /*6000 for SF12,4000 for SF11,3000 for SF11, 2000 for SF9/8/, 1500 for SF7. All examples consering 50 bytes payload and BW125*/     
/*Buffers used*/
unsigned char buffer_binary[128] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,16,17,18,19,20};
char char_temp[256];//256 is ok size for recieving command data

unsigned int LoRa_bps=0;   //stores bits per seconds transmited for the DR and LoRa_FREQ_standard setup. call lora.getbitRate(LoRa_bps,NULL);
float LoRa_head_tx_time=0; //stores time in ms to transmit only the header of the packet lora.getbitRate(NULL,&LoRa_head_tx_time);

/*******************************************************************/
/*defines dependensies to avoid a CRASH of the program*/
#ifdef PRINT_TO_USER_TIME_DIFFERENCE
  #ifndef PRINT_TO_USER
    #define PRINT_TO_USER
  #endif
#endif 
#ifdef LORA_DEBUG_AND_PRINT
  #ifndef PRINT_TO_USER
    #define PRINT_TO_USER
  #endif
#endif 
/*******************************************************************/
#ifdef PRINT_TO_USER_TIME_DIFFERENCE
void printTimeReceptiondifference(unsigned int time_tx1,unsigned int time_tx2){
   char_temp[0]='\0';
   sprintf(char_temp, "\r\nEstimated transmission time of messages with %i and %i bytes as payload: %.1f ms, %.1f ms.",PAYLOAD_FIRST_TX,PAYLOAD_SECOND_TX,
   lora.getTransmissionTime(PAYLOAD_FIRST_TX),  lora.getTransmissionTime(PAYLOAD_SECOND_TX));
   Serial.print(char_temp);//to print the obtained characters
   char_temp[0]='\0';
   sprintf(char_temp, "\r\nCalculated time difference between transmitting %i and %i bytes as payload: %.1f ms.",PAYLOAD_FIRST_TX,PAYLOAD_SECOND_TX,
    lora.getTransmissionTime(PAYLOAD_SECOND_TX) -lora.getTransmissionTime(PAYLOAD_FIRST_TX));
   Serial.print(char_temp);//to print the obtained characters
   char_temp[0]='\0';
   sprintf(char_temp, "\r\nMeasured difference between transmitting with %i and %i bytes as payload: %.1f ms.",PAYLOAD_FIRST_TX,PAYLOAD_SECOND_TX,
   time_tx2-time_tx1);
   Serial.print(char_temp);//to print the obtained characters
}
#endif
/*Set up the LoRa module with the desired configuration */
void LoRa_setup(void){
  lora.setDeviceMode(LWOTAA);/*LWOTAA or LWABP. We use LWOTAA in this example*/
  lora.setDataRate((_data_rate_t)LoRa_DR, (_physical_type_t)LoRa_FREQ_standard);
  lora.setKey(NULL, NULL, LoRa_APPKEY); /*Only App key is seeted when using OOTA*/
  lora.setClassType((_class_type_t) LoRa_DEVICE_CLASS); /*set device class*/
  lora.setPort(LoRa_PORT);/*set the default port for transmiting data*/
  lora.setPower(LoRa_POWER); /*sets the Tx power*/
  lora.setChannel(LoRa_CHANNEL);/*selects the channel*/
  lora.setAdaptiveDataRate(LoRa_ADR_FLAG);/*Enables adaptative data rate*/  
}
/**-----------------------------------------------------
 *-------------------------------------------------------*/
void setup(void){
  #if (defined(PRINT_TO_USER)|defined(COMMAND_PRINT_TO_USER))/*ensure this to support the print of messages*/
  Serial.begin(115200);/*Init Print Serial Port*/
  #endif
  /*Init the LoRa class after initing the serial print port */
  lora.init();/* call lora.init(Arduino_Tx_PIN,Arduino_Rx_PIN) to use software serial. Example: lora.init(D2,D3) */
  /*First get device EUI for later printing*/
  lora.getId(char_temp,100); /*100 ms is more than enough to get a response from the module*/
  /*set up device. You must set up all your parameters BEFORE Joining.
   If you make any change (outside channel or port setup), you should join again the network for proper working*/
  LoRa_setup();
  /*Now shows you the device actual DevEUI and AppEUI got at the time you call the function */
  #ifdef PRINT_TO_USER 
  Serial.print("\r\nCurrent DevEui: ");/*to print the obtained characters*/
  Serial.print(char_temp);/*to print the obtained characters*/
  #endif
  /*Enters in a while Loop until the join process is completed*/ 
  while(lora.setOTAAJoin(JOIN, 10000)==0);//will attempt to join network until the ends of time. https://www.thethingsnetwork.org/docs/lorawan/message-types/
  /*Get the bit Rate after join (because if Adaptative Data Rate is enable, DR could be changed after join )*/
  lora.getbitRate(&LoRa_bps,&LoRa_head_tx_time);
   /*POWER DOWN the LoRa module until next Tx (Transmition) cicle*/
  lora.setDeviceLowPower();
}
/*packet transfer example*/
static int time_tx1,time_tx2;
void loop(void)
{
  /*Wake Up the LoRa module*/
  lora.setDeviceWakeUp();/*if the module is not in sleep state, this command does nothing*/
  /*--------sending bytes message*/
  /*We send the same packet but with PAYLOAD_FIRST_TX and PAYLOAD_SECOND_TX bytes of payload using LoRa_FREQ_standard And
  Check https://avbentem.github.io/airtime-calculator/ttn/eu868/10 for knowing more how this stimations are made */
  time_tx1=lora.transferPacketWithConfirmed(buffer_binary,PAYLOAD_FIRST_TX,Tx_and_ACK_RX_timeout);
  time_tx2=lora.transferPacketWithConfirmed(buffer_binary,PAYLOAD_SECOND_TX,Tx_and_ACK_RX_timeout);
  /*Print DEMO information to the USER*/
  #ifdef PRINT_TO_USER_TIME_DIFFERENCE 
  printTimeReceptiondifference(time_tx1,time_tx2);
  #endif 
  /*POWER DOWN the LoRa module until next Tx Transmition (Tx) cicle*/
  lora.setDeviceLowPower();
  /*Sleeps until the next tx cicle*/
  delay((unsigned int)(Tx_delay_s*1000));/*Convert the value in seconds to miliseconds*/
}
