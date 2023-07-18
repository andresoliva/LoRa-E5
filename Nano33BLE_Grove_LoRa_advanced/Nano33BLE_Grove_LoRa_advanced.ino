//This a custom version of the Seed example for their Grove LoRa_E5 sensor
//That works with Seeeduino XIAO, Seeeduino XIAO expantion board and DHT11 Temperature and  Humidity Sensor
//But adaptated for the Arduino Nano 33 BLE Sense connected to a Grove LoRa_E5 sensor
//Original code; https://wiki.seeedstudio.com/Grove_LoRa_E5_New_Version/#software-code
//Working with Arduino 1.8.14 IDE and forward
//Modified by: Andres Oliva Trevisan
#include <Arduino.h>
#include <stdarg.h> //for using 

#define PRINT_TO_USER //To allow printing 

//WARNING. DO NOT SET
#define SET_CUSTOM_DEVUI// COMMENTED.// DO NOT UNCOMMENT UNLESS YOU HAVE THE ORIGINAL ID//
//Information regarding the following parameters https://www.thethingsnetwork.org/docs/lorawan/architecture/

//2B7E151628AED2A6ABF7158809CF4F3C original //2B7E151628AED2A609CF4F3CABF71588
#ifdef SET_CUSTOM_DEVUI
#define LoRa_DEVEUI_CUSTOM       "2CF7F1C0440004A2" //Custom key for this App. You can generate one at https://www.thethingsnetwork.org/docs/devices/registration/
#endif
#define LoRa_APPKEY              "2B7E151628AED2A609CF4F3CABF71588" //Custom key for this App
#define LoRa_FREQ_standard       EU868   //International frequency band. see
#define LoRa_DR                  DR5     //DR5=5.2kbps //data rate. see at https://www.thethingsnetwork.org/docs/lorawan/regional-parameters/
//SF7/BW125 at XX915/            DR3
//SF7/BW125 at EU868 (most)      DR5
#define LoRa_DEVICE_CLASS        CLASS_A //CLASS_A for power restriction/low power nodes. Class C other device
#define LoRa_PORT_BYTES          8       //node Port for binary values
#define LoRa_PORT_STRING         7       //node Port for string messages
#define LoRa_POWER               14      //node Tx power
#define LoRa_Tx_preamble_number  10
#define LoRa_Rx_preamble_number  15
#define LoRa_FREQ   868         //Standard_freq_band
#define LoRa_RF_BW  BW125
#define LoRa_RF_SF  SF12


//DEMO PARAMETERS
//you can get LoRa_head_tx_time in https://avbentem.github.io/airtime-calculator/ttn/eu868/0
#if LoRa_FREQ_standard==EU868
 #if LoRa_DR==DR0
   #define LoRa_bps          250  //bytes per second
   #define LoRa_head_tx_time 1,155.1​  //time in ms to transmit only the header of the packet
 #endif
 #if LoRa_DR==DR5
   #define LoRa_bps          5470  //bytes per second
   #define LoRa_head_tx_time 46.3  //time in ms to transmit only the header of the packet
 #endif
#endif
#define PAYLOAD_FIRST_TX  10 //bytes
#define PAYLOAD_SECOND_TX 100 //byest
#define Tx_and_ACK_RX_timeout   2000         //Standard_freq_band


#include "LoRaE5.h"
unsigned char buffer_binary[128] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,16,17,18,19,20};
unsigned char buffer_p2p[128] = {0xef, 0xff, 0x55, 3, 4, 5, 6, 7, 8, 9,};
char buffer_char[256] = "Hello, I am  a sending this string message using LoRa.";
char char_temp[256];//256 is ok size for recieving command data

void setup(void){
  SerialUSB.begin(115200);
  lora.init();
  //for p2p
  //lora.initP2PMode(LoRa_FREQ, LoRa_RF_SF, LoRa_RF_BW, LoRa_Tx_preamble_number, LoRa_Rx_preamble_number, LoRa_POWER);
  //for gateway communication:
#ifdef SET_CUSTOM_DEVUI
  lora.setId(NULL, LoRa_DEVEUI_CUSTOM, NULL);//WARNING: If you run this command, you will change FOREVER the defaultd value that the fabricator has asigned to the module.
#endif
  //get device EUI for later printing
  lora.getId(char_temp,200); //200 ms is enough to get a response from the module
  //set up device
  lora.setDeviceMode(LWOTAA);//LWOTAA or LWABP
  lora.setDataRate((_data_rate_t)LoRa_DR, (_physical_type_t)LoRa_FREQ_standard);
  lora.setKey(NULL, NULL, LoRa_APPKEY); //Only App key is seeted when using OOTA
  lora.setClassType((_class_type_t) LoRa_DEVICE_CLASS); //set device class
  lora.setPort(LoRa_PORT_BYTES);
  lora.setPower(LoRa_POWER); //sets the Tx power
  //Tries to join the network
  while( (lora.setOTAAJoin(JOIN, 5000))==0);//will attempt to join network until the ends of time //https://www.thethingsnetwork.org/docs/lorawan/message-types/
  //Now shows you the device actual DevEUI and AppEUI got at the begining
  //NOTE: the AppEUI is the original one set by the developer in this example, even if you modify it
  #ifdef PRINT_TO_USER //defined in LoRaE5.h
  SerialUSB.print(char_temp);//to print the obtained characters
  #endif
}

void loop(void)
{
  // setup();
  //lora.transferPacketP2PMode(buffer, 10);
  int time_tx1,time_tx2;
  /*Wake Up the LoRa module*/
  lora.setDeviceWakeUp();//wake up the device if sleep
  /*sending a string message*/
  lora.setPort(LoRa_PORT_STRING);//set port for local reception
  lora.transferPacketWithConfirmed(buffer_char,2000);
  /*sending bytes message*/
  lora.setPort(LoRa_PORT_BYTES);//set port for local reception
  /*We send the same packet but with 10 and 100 bytes of payload. Using SF7/BW125Khz (default for EU868),
  Command Time + Time to get ACK response should be around 128.7 ms
  Check https://avbentem.github.io/airtime-calculator/ttn/eu868/10 for more info */
  
  time_tx1=lora.transferPacketWithConfirmed(buffer_binary,PAYLOAD_FIRST_TX,Tx_and_ACK_RX_timeout);
  time_tx2=lora.transferPacketWithConfirmed(buffer_binary,PAYLOAD_SECOND_TX,Tx_and_ACK_RX_timeout);
  #ifdef PRINT_TO_USER //defined in LoRaE5.h
  char_temp[0]='\0';
  sprintf(char_temp, "\r\nCalculated transmition time of messages with %i and %i bytes as payload: %.1f ms,%.1f ms.",PAYLOAD_FIRST_TX,PAYLOAD_SECOND_TX,
  LoRa_head_tx_time+(float)(PAYLOAD_FIRST_TX)*8*1000/LoRa_bps,  LoRa_head_tx_time+(float)(PAYLOAD_SECOND_TX)*8*1000/LoRa_bps);
  SerialUSB.print(char_temp);//to print the obtained characters
  char_temp[0]='\0';
  sprintf(char_temp, "\r\nCalculated diference between transmiting %i and %i bytes as payload: %.1f ms.",PAYLOAD_FIRST_TX,PAYLOAD_SECOND_TX,
  ((float)(PAYLOAD_SECOND_TX-PAYLOAD_FIRST_TX)*8)*1000/LoRa_bps);
  SerialUSB.print(char_temp);//to print the obtained characters
  char_temp[0]='\0';
  sprintf(char_temp, "\r\nMeasured estimated diference between transmition time: %i ms.", time_tx2-time_tx1);
  SerialUSB.print(char_temp);//to print the obtained characters
  #endif 
  /*POWER DOWN the LoRa module*/
  lora.setDeviceLowPower();
  delay(10000);
}
