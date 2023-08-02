/*Example mixing all the different lessons teached in the examples in this repository*/
/*Working with Arduino 1.8.14 IDE and forward*/
/*Created  by: Andres Oliva Trevisan*/
/*Repo: https://github.com/andresoliva/LoRa-E5/ */
#include <Arduino.h>
#include <stdarg.h> //for using 
//--------------------------
#include <LoRa-E5.h>   //main LoRa lib
//---------------------
/**USER OPTIONS*/
#define PRINT_TO_USER                 /*To allow the printing of characters using UART*/
#define LORA_DEBUG_AND_PRINT          lora_QUIET /*Enables the debug mode of the device and allow serial printing*//*******************************************************************************************************/
/************************LORA SET UP*******************************************************************/
/*LoRa radio Init Parameters. Info:  https://www.thethingsnetwork.org/docs/lorawan/architecture/ */
#define LoRa_APPKEY              "2B7E151628AED2A609CF4F3CABF71588" /*Custom key for this App*/
#define LoRa_FREQ_standard       EU868   /*International frequency band. see*/
#define LoRa_DEVICE_CLASS        CLASS_A /*CLASS_A for power restriction/low power nodes. Class C for other device applications */
#define LoRa_POWER               16      /*Node Tx (Transmition) power*/
#define LoRa_PORT                8     /*Node Tx (Transmition) port*/
#define LoRa_CHANNEL             0       /*Node selected Tx channel. Default is 0, we use 2 to show only to show how to set up*/
#define LoRa_ADR_FLAG            false   /*ADR(Adaptative Dara Rate) status flag (True or False). Use False if your Node is moving*/
/*FOR SETTING DATA RATE USING THE OTHER MODE*/
#define LoRa_SF                  SF8     /*DR5=5.2kbps //data rate. see at https://www.thethingsnetwork.org/docs/lorawan/regional-parameters/    */
#define LoRa_BW                  BW125    /*DR5=5.2kbps //data rate. see at https://www.thethingsnetwork.org/docs/lorawan/regional-parameters/    */
/*Time to wait for transmiting a packet again*/
#define Tx_delay_s               60.     /*delay between transmitions expressed in seconds*/
/*Packet information size: https://lora-developers.semtech.com/documentation/tech-papers-and-guides/lora-and-lorawan */
#define PAYLOAD_TX_51            51   /*bytes to send. 51 is the max suggested to avoid the 64 bytes packet restriction*/

#define Tx_and_ACK_RX_timeout    6000 /*6000 for SF12,4000 for SF11,3000 for SF11, 2000 for SF9/8/, 1500 for SF7. All examples consering 50 bytes payload and BW125*/     
/*Buffers used*/
unsigned char buffer_binary[128] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,16,17,18,19,20};
char char_temp[256];//256 is ok size for recieving command data

unsigned int LoRa_bps=0;   //stores bits per seconds transmited for the DR and LoRa_FREQ_standard setup. call lora.getbitRate(LoRa_bps,NULL);
float LoRa_head_tx_time=0; //stores time in ms to transmit only the header of the packet lora.getbitRate(NULL,&LoRa_head_tx_time);

/*******************************************************************/
/*defines dependensies to avoid a CRASH of the program*/
#ifdef LORA_DEBUG_AND_PRINT
  #ifndef PRINT_TO_USER
    #define PRINT_TO_USER
  #endif
#endif 
/*******************************************************************/
/*Seeps between different power values in steps of 2, as suportwed by E5*/
void LoRa_power_sweep(short power_max){
  /*This module supports power between 16 and 2 for that band*/
  for (int i = power_max; i >-1; i=i-2){
    lora.setPower(i); /*sets the Tx power*/  
    lora.transferPacketWithConfirmed(buffer_binary,PAYLOAD_TX_51,Tx_and_ACK_RX_timeout);
    delay(1000);/*sleeps 1000 ms*/
  }
}
/*Set up the LoRa module with the desired configuration */
void LoRa_setup(void){
  lora.setDeviceMode(LWOTAA);/*LWOTAA or LWABP. We use LWOTAA in this example*/
  lora.setFrequencyBand((_physical_type_t)LoRa_FREQ_standard); /*Set your Spread Factor and SW*/
  lora.setSpreadFactor(LoRa_SF,LoRa_BW ,(_physical_type_t)LoRa_FREQ_standard); /*Set your Spread Factor and SW*/
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
  //while(!Serial){delay(100);}
  Serial.begin(115200);/*Init Print Serial Port*/
  #endif
  /*Init the LoRa class after initing the serial print port */
  lora.init(D0,D1);//D0,D1/* call lora.init(Arduino_Tx_PIN,Arduino_Rx_PIN) to use software serial. Example: lora.init(D2,D3) */
  /*First get device EUI for later printing*/
  lora.getId(char_temp,DevEui); 
  /*Debug mode will give you additional info to know what is happening*/
  #ifdef LORA_DEBUG_AND_PRINT
  lora.Debug(LORA_DEBUG_AND_PRINT);//change acordingly
  #endif
  /*set up device. You must set up all your parameters BEFORE Joining.
   If you make any change (outside channel or port setup), you should join again the network for proper working*/
  LoRa_setup();
  /*Now shows you the device actual DevEUI and AppEUI got at the time you call the function */
  #ifdef PRINT_TO_USER 
  Serial.print("\r\nCurrent DevEui: ");/*to print the obtained characters*/
  Serial.print(char_temp);/*to print the obtained characters*/
  #endif
  /*set auto low power mode before setting baud rate and joining for optimal performance*/
  lora.setDeviceLowPowerAutomode(true);
  /*Set baud rate before joining*/
  lora.setDeviceBaudRate(BR_115200); /*Supported baud rates:BR_9600,BR_38400,BR_115200*
  /*Enters in a while Loop until the join process is completed*/ 
  while(lora.setOTAAJoin(JOIN, 10000)==0);//will attempt to join network until the ends of time. https://www.thethingsnetwork.org/docs/lorawan/message-types/
  /*Get the bit Rate after join (because if Adaptative Data Rate is enable, DR could be changed after join )*/
}
void loop(void)
{
  LoRa_power_sweep(LoRa_POWER);
    /*restore default baud rate*/
  lora.setDeviceBaudRate(BR_9600); /*Supported baud rates:BR_9600,BR_38400,BR_115200*/
  delay((unsigned int)(Tx_delay_s*1000));/*Convert the value in seconds to miliseconds*/
}
