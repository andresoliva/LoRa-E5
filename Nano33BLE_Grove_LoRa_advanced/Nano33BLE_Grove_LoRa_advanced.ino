//This a custom version of the Seed example for their Grove LoRa_E5 sensor
//That works with Seeeduino XIAO, Seeeduino XIAO expantion board and DHT11 Temperature and  Humidity Sensor
//But adaptated for the Arduino Nano 33 BLE Sense connected to a Grove LoRa_E5 sensor
//Original code; https://wiki.seeedstudio.com/Grove_LoRa_E5_New_Version/#software-code
//Working with Arduino 1.8.14 IDE and forward
//Modified by: Andres Oliva Trevisan
#include <Arduino.h>
#include <stdarg.h> //for using 
//WARNING. DO NOT SET
#define SET_CUSTOM_DEVUI// COMMENTED.// DO NOT UNCOMMENT UNLESS YOU HAVE THE ORIGINAL ID//
//Information regarding the following parameters https://www.thethingsnetwork.org/docs/lorawan/architecture/

//2B7E151628AED2A6ABF7158809CF4F3C original //2B7E151628AED2A609CF4F3CABF71588
#ifdef SET_CUSTOM_DEVUI
#define LoRa_DEVEUI_CUSTOM       "2CF7F1C0440004A1" //Custom key for this App. You can generate one at https://www.thethingsnetwork.org/docs/devices/registration/
#endif
#define LoRa_APPKEY              "2B7E151628AED2A609CF4F3CABF71588" //Custom key for this App
#define LoRa_FREQ_standard       EU868   //International frequency band. see
#define LoRa_DEVICE_CLASS        CLASS_A //CLASS_A for power restriction/low power nodes. Class C other device
#define LoRa_PORT                '8'     //node Tx power
#define LoRa_POWER               14     //node Tx power
#define LoRa_DR                  DR5   //DR5=5.2kbps //data rate. see at https://www.thethingsnetwork.org/docs/lorawan/regional-parameters/
#define LoRa_Tx_preamble_number  10
#define LoRa_Rx_preamble_number  15


#define LoRa_FREQ   868         //Standard_freq_band
#define LoRa_RF_BW  BW125
#define LoRa_RF_SF  SF7




#include "LoRaE5.h"
unsigned char buffer[128] = {0xef, 0xff, 0x55, 3, 4, 5, 6, 7, 8, 9,};
 char buffer2[256] = "Hello, running test";

void setup(void){
  char chat_temp[256];//256 is ok size for recieving command data
  SerialUSB.begin(115200);
  lora.init();
  //for p2p
  //lora.initP2PMode(LoRa_FREQ, LoRa_RF_SF, LoRa_RF_BW, LoRa_Tx_preamble_number, LoRa_Rx_preamble_number, LoRa_POWER);
  //for gateway communication:
#ifdef SET_CUSTOM_DEVUI
  lora.setId(NULL, LoRa_DEVEUI_CUSTOM, NULL);//WARNING: If you run this command, you will change FOREVER the defaultd value that the fabricator has asigned to the module.
#endif
  //get device EUI for later printing
  lora.getId(chat_temp,sizeof(chat_temp),3000); 
  //set up device
  lora.setDeviceMode(LWOTAA);//LWOTAA or LWABP
  lora.setDataRate((_data_rate_t)LoRa_DR, (_physical_type_t)LoRa_FREQ_standard);
  lora.setKey(NULL, NULL, LoRa_APPKEY); //Only App key is seeted when using OOTA
  lora.setClassType((_class_type_t) LoRa_DEVICE_CLASS); //set device class
  lora.setPort((unsigned char)LoRa_PORT);
  lora.setPower(LoRa_POWER); //sets the Tx power
  //Tries to join the network
  lora.setOTAAJoin(JOIN, 10000); //https://www.thethingsnetwork.org/docs/lorawan/message-types/
  //Now shows you the device actual DevEUI and AppEUI got at the begining
  //NOTE: the AppEUI is the original one set by the developer in this example, even if you modify it
  SerialUSB.print(chat_temp);

}

void loop(void)
{
  // setup();
  SerialUSB.println("Send string.");
  //lora.transferPacketP2PMode(buffer, 10);
  lora.transferPacketWithConfirmed(buffer2);
  //lora.transferPacketWithConfirmed(buffer,2000);
  SerialUSB.println("Send hex.");
  delay(4000);
}
