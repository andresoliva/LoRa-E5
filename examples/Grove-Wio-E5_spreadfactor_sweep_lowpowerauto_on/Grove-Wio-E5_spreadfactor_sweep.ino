/*Example to send differents packes by changing the SpreadFactor and stimating power consumption of the module*/
/*Working with Arduino 1.8.14 IDE and forward*/
/*Created  by: Andres Oliva Trevisan*/
/*Repo: https://github.com/andresoliva/LoRa-E5/ */
#include <Arduino.h>
#include <stdarg.h> //for using 
//--------------------------
#include <LoRa-E5.h>   //main LoRa lib
//---------------------
/**USER OPTIONS*/
#define PRINT_TO_USER                   /*To allow the printing of characters using UART*/
#define PRINT_TO_USER_TIME_DIFFERENCE /*To allow the printing of time difference message*/
#define LORA_DEBUG_AND_PRINT          lora_QUIET /*Enables the debug mode of the device and allow serial printing*//*******************************************************************************************************/
/************************LORA SET UP*******************************************************************/
/*LoRa radio Init Parameters. Info:  https://www.thethingsnetwork.org/docs/lorawan/architecture/ */
#define LoRa_APPKEY              "2B7E151628AED2A609CF4F3CABF71588" ///*Custom key for this App*/
#define LoRa_FREQ_standard       EU868   /*International frequency band. see*/
#define LoRa_DEVICE_CLASS        CLASS_A /*CLASS_A for power restriction/low power nodes. Class C for other device applications */
#define LoRa_POWER               14      /*Node Tx (Transmition) power*/
#define LoRa_PORT                8     /*Node Tx (Transmition) port*/
#define LoRa_CHANNEL             0       /*Node selected Tx channel. Default is 0, we use 2 to show only to show how to set up*/
#define LoRa_ADR_FLAG            false   /*ADR(Adaptative Dara Rate) status flag (True or False). Use False if your Node is moving*/
/*FOR SETTING DATA RATE USING THE OTHER MODE*/
#define LoRa_SF                  SF7     /*DR5=5.2kbps //data rate. see at https://www.thethingsnetwork.org/docs/lorawan/regional-parameters/    */
#define LoRa_BW                  BW125    /*DR5=5.2kbps //data rate. see at https://www.thethingsnetwork.org/docs/lorawan/regional-parameters/    */
/*Time to wait for transmiting a packet again*/
#define Tx_delay_s               6000.     /*delay between transmitions expressed in seconds*/
/*Packet information size: https://lora-developers.semtech.com/documentation/tech-papers-and-guides/lora-and-lorawan */
#define PAYLOAD_TX_11            11  /*bytes to send. 11 bytes */
#define PAYLOAD_TX_51            51   /*bytes to send. 51 is the max suggested to avoid the 64 bytes packet restriction*/
#define PAYLOAD_TX_222           222  /*bytes to send. 222 */

#define Tx_and_ACK_RX_timeout    6000 /*6000 for SF12,4000 for SF11,3000 for SF11, 2000 for SF9/8/, 1500 for SF7. All examples consering 50 bytes payload and BW125*/     
/*Buffers used*/
unsigned char buffer_binary[255] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,16,17,18,19,20};
char char_temp[255];//256 is ok size for recieving command data

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
void LoRa_spread_factor_sweep(unsigned int payload_size){
  float power_consumption_mAh;
  for (unsigned int i = (unsigned int)SF7; i < ((unsigned int)SF12+1); ++i)
  {
  lora.setSpreadFactor((_spreading_factor_t)i,LoRa_BW ,(_physical_type_t)LoRa_FREQ_standard);
  lora.transferPacketWithConfirmed(buffer_binary, payload_size,Tx_and_ACK_RX_timeout); 
  delay(100);
  #if(defined(PRINT_TO_USER))/*ensure this to support the print of messages*/
  power_consumption_mAh=lora.getTransmissionPower(payload_size,600);
  char_temp[0]='\0';/*Reset string*/
  sprintf(char_temp,"\r\n\r\nTransmiting %i bytes with an Spread factor: SF%i. Spected power consumption:%.3f mAh.",payload_size,i,power_consumption_mAh);/*to print the obtained characters*/
  //sprintf(char_temp,"Bitrate%i bps:TimeToTxHead:%.3f .",lora.readbitRate(),lora.readtxHead_time());/*to print the obtained characters*/
  Serial.print(char_temp);/*to print the obtained characters*/
  #endif
  delay(400);/*Convert the value in seconds to miliseconds*/
  }
}
/*Set up the LoRa module with the desired configuration */
void LoRa_setup(void){
  lora.setDeviceMode(LWOTAA);/*LWOTAA or LWABP. We use LWOTAA in this example*/
  lora.setFrequencyBand((_physical_type_t)LoRa_FREQ_standard);
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
  digitalWrite(LED_PWR, LOW); //Turn off Arduino Led.
  #if (defined(PRINT_TO_USER)|defined(COMMAND_PRINT_TO_USER))/*ensure this to support the print of messages*/
  //while(!Serial){delay(100);}/*uncomment if you want an easy debug*/
  Serial.begin(115200);/*Init Print Serial Port*/
  #endif
  /*Init the LoRa class after initing the serial print port */
  lora.init();/* call lora.init(Arduino_Tx_PIN,Arduino_Rx_PIN) to use software serial. Example: lora.init(D2,D3) */
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
  /*set auto low power mode before setting baud rate and joining for optimal low power performance*/
  lora.setDeviceLowPowerAutomode(true);
  /*Set baud rate before joining for a better performance*/
  lora.setDeviceBaudRate(BR_115200); /*Supported baud rates:BR_9600,BR_38400,BR_115200*/
  /*Attempts to join*/
  while(lora.setOTAAJoin(JOIN, 10000)==0);//will attempt to join network until the ends of time. https://www.thethingsnetwork.org/docs/lorawan/message-types/
}

void loop(void)
{
  LoRa_spread_factor_sweep(PAYLOAD_TX_11);
  delay((unsigned int)(2.5*1000));/*Convert the value in seconds to miliseconds*/
  LoRa_spread_factor_sweep(PAYLOAD_TX_51);
  delay((unsigned int)(2.5*1000));/*Convert the value in seconds to miliseconds*/
  LoRa_spread_factor_sweep(PAYLOAD_TX_222);
  /*restore default baud rate agaib*/
  lora.setDeviceBaudRate(BR_9600); /*Supported baud rates:BR_9600,BR_38400,BR_115200*/
  delay((unsigned int)(Tx_delay_s*1000));/*Convert the value in seconds to miliseconds*/
}
