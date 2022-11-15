//Libraries
#include "SPI.h" //https://www.arduino.cc/en/reference/SPI
#include "MFRC522.h" //https://github.com/miguelbalboa/rfid
#include "WiFi.h"
#include "WiFiClientSecure.h"
#include "SpotifyArduino.h"
#include "SpotifyArduinoCert.h"
#include "ArduinoJson.h"
//Constants
#define SS_PIN 5
#define RST_PIN 0
#define SPOTIFY_MARKET "IE"
#define SPOTIFY_REFRESH_TOKEN "AQDQM2u2ESs-XlItHApXTIF7WSQhsXC9krgu3xDjy95ppQB2kj6lqFHYaaWuZq13Ym19iOF-C5z9Datg7mogx-pdlEIsEpZ_AOospqhNk9FmuDDGlF2Xd4MHLG5mPT6oWgE"

char ssid[] = "your network ssid";         // your network SSID (name)
char password[] = "your network password"; // your network password

char clientId[] = "8c8df7b2f63847fc8621c6eb197b4985";     // Your client ID of your spotify APP
char clientSecret[] = "ccd115c9fe5347178cd464d1f9aced65"; // Your client Secret of your spotify APP (Do Not share this!)


//Parameters
const int ipaddress[4] = {103, 97, 67, 25};
//Variables
byte nuidPICC[4] = {0, 0, 0, 0};
MFRC522::MIFARE_Key key;
MFRC522 rfid = MFRC522(SS_PIN, RST_PIN);
char currentDevice[41];

typedef struct {
  uint32_t ID;
  char* spotifyURI;
  char* albumName;
} Card;

const Card cardArray[] {
  {0xA38A4A83, "spotify:album:3mH6qwIy9crq0I9YQbOuDf", "Blonde"},
  {0x7068F9AB, "spotify:album:6tkjU4Umpo79wwkgPMV3nZ", "Goodbye & Good Riddance"},
  {0x7067A815, "spotify:album:34GQP3dILpyCN018y2k61L", "BALLADS 1"},
};

WiFiClientSecure client;
SpotifyArduino spotify(client, clientId, clientSecret, SPOTIFY_REFRESH_TOKEN);

void playAlbum(char* albumURI)
{
    char body[100];
    sprintf(body, "{\"context_uri\" : \"%s\"}", albumURI);
    if (spotify.playAdvanced(body))
    {
        Serial.println("sent!");
    }
}

bool getDeviceCallback(SpotifyDevice device, int index, int numDevices) {
  if (device.isActive) {
    strncpy(currentDevice, device.id, sizeof(currentDevice));
    currentDevice[sizeof(currentDevice) - 1] = '\0';
    return false;
  } else {
    strncpy(currentDevice, "0", sizeof(currentDevice));
    currentDevice[sizeof(currentDevice) - 1] = '\0';
    if (index == numDevices - 1)
    {
      return false;
    } else {
      return true;
    }
  }


  return false;
}

void setup() {
 //Init Serial USB
 Serial.begin(115200);
 
 SPI.begin();
 rfid.PCD_Init();
 
 // Set WiFi to station mode and disconnect from an AP if it was Previously
 // connected
 WiFi.mode(WIFI_STA);
 WiFi.disconnect();
 delay(100);
 
 // Attempt to connect to Wifi network:
 Serial.print("Connecting Wifi: ");
 Serial.println(ssid);
 
 WiFi.begin(ssid, password);
 while (WiFi.status() != WL_CONNECTED)
 {
  Serial.print(".");
  delay(500);
 }
 
 Serial.println("");
 Serial.println("WiFi connected");
 Serial.println("IP address: ");
 IPAddress ip = WiFi.localIP();
 Serial.println(ip);
 
 client.setCACert(spotify_server_cert);
 
 Serial.println("Refreshing Access Tokens");
 if (!spotify.refreshAccessToken())
 {
  Serial.println("Failed to get access tokens");
  return;
 }
 
}
void loop() {
 readRFID();
}
void readRFID(void ) { /* function readRFID */
 ////Read RFID card
 for (byte i = 0; i < 6; i++) {
   key.keyByte[i] = 0xFF;
 }
 // Look for new 1 cards
 if ( ! rfid.PICC_IsNewCardPresent())
   return;
 // Verify if the NUID has been readed
 if (  !rfid.PICC_ReadCardSerial())
   return;
 // Store NUID into nuidPICC array
 for (byte i = 0; i < 4; i++) {
   nuidPICC[i] = rfid.uid.uidByte[i];
 }
 Serial.print(F("RFID In dec: "));
 printHex(rfid.uid.uidByte, rfid.uid.size);
 Serial.println();

 for(uint8_t i = 0; i < sizeof(cardArray)/sizeof(Card); ++i) {
  if (*(uint32_t*)nuidPICC == cardArray[i].ID) {
    
    Serial.println(cardArray[i].albumName);
    
//    int status = spotify.getDevices(getDeviceCallback);
//    Serial.println(currentDevice);
//    if (status == 200) {
    playAlbum(cardArray[i].spotifyURI);
    spotify.toggleShuffle(true);
    spotify.setRepeatMode(repeat_context);
      
//      if(strcmp(currentDevice, "5d43a71bc3b1560d3c658bf652a7e3cbc39246e7") != 0) {
//        Serial.println("TRANSFERRING PLAYBACK!");
//        
//        spotify.transferPlayback("5d43a71bc3b1560d3c658bf652a7e3cbc39246e7", true);
//        spotify.toggleShuffle(true);
//        spotify.setRepeatMode(repeat_context);
//      }
//    }
  }
 }
 
 
 
 // Halt PICC
 rfid.PICC_HaltA();
 // Stop encryption on PCD
 rfid.PCD_StopCrypto1();
}

/**
   Helper routine to dump a byte array as hex values to Serial.
*/
void printHex(byte *buffer, byte bufferSize) {
 for (byte i = 0; i < bufferSize; i++) {
   Serial.print(buffer[i] < 0x10 ? " 0" : " ");
   Serial.print(buffer[i], HEX);
 }
}
