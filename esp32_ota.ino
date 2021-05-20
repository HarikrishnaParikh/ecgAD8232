#include <WiFi.h>
#include <HTTPClient.h>
#include <HTTPUpdate.h>
#include <WiFiClientSecure.h>
#include "cert.h"
const char * ssid = "M31";
const char * password = "tyye5587";
String FirmwareVer = "1.0";
#define FW_Ver_URL "https://raw.githubusercontent.com/programmer131/ESP8266_ESP32_SelfUpdate/master/esp32_ota/bin_version.txt"
#define FW_bin_URL "https://raw.githubusercontent.com/programmer131/ESP8266_ESP32_SelfUpdate/master/esp32_ota/fw.bin"
void connect_wifi();
void FW_Update();
int FW_Ver_Chk();
void IRAM_ATTR isr();

struct Pin 
{ const uint8_t pin;
  uint32_t P_cnt;
  bool sttus;
}Update_pin = {0, 0, false};

void setup() 
{ pinMode(Update_pin.pin, INPUT);
  attachInterrupt(Update_pin.pin, isr, RISING);
  Serial.begin(115200);
  Serial.print("Active firmware version:");
  Serial.println(FirmwareVer);
  pinMode(2, OUTPUT);
  connect_wifi();
}

void loop() 
{ if (Update_pin.sttus && Update_pin.P_cnt)        //update by button press 
  { Serial.println("Firmware update Starting..");
    FW_Update();
    Update_pin.sttus = false;
    Update_pin.P_cnt=0;
  }
  if(Serial.read()=='e')
    FW_Update();
  digitalWrite(2,HIGH);
  delay(500);
  digitalWrite(2,LOW);
  delay(500);
}

void connect_wifi() 
{ Serial.println("Waiting for WiFi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
}

void FW_Update(void) 
{ static int num=0;
  if (FW_Ver_Chk()) 
  { Serial.println("UPDATING FIRMWARE");
    WiFiClientSecure client;
    client.setCACert(rootCACertificate);
    httpUpdate.setLedPin(2, LOW);
    t_httpUpdate_return retvalue = httpUpdate.update(client, FW_bin_URL);
    switch (retvalue) 
    { case 0: Serial.print("HTTP_UPDATE_FAILD Error...(");      //HTTP_UPDATE_FAILD
              Serial.print(httpUpdate.getLastError(),DEC);
              Serial.print("): ");
              Serial.println(httpUpdate.getLastErrorString().c_str());
              break;
      case 1: Serial.println("HTTP_UPDATE_NO_UPDATES");         //HTTP_UPDATE_NO_UPDATES:
              break;
      case 2: Serial.println("HTTP_UPDATE_OK");                 //HTTP_UPDATE_OK
              break;
    }
  }
}

int FW_Ver_Chk(void) 
{ String payload;
  int httpCode;
  String fwurl = "";
  fwurl += FW_Ver_URL;
  fwurl += "?";
  fwurl += String(rand());
  Serial.println(fwurl);
  WiFiClientSecure * client = new WiFiClientSecure;
  if (client) 
  { client -> setCACert(rootCACertificate);
    HTTPClient https;
    if (https.begin( * client, fwurl)) 
    { Serial.print("[HTTPS] GET...\n");         // HTTPS      
      delay(100);                               // start connection and send HTTP header
      httpCode = https.GET();
      delay(100);
      if (httpCode == HTTP_CODE_OK)             // if version received
      { payload = https.getString();            // save received version
      } 
      else 
      { Serial.print("error in downloading version file:");
        Serial.println(httpCode);
      }
      https.end();
    }
    delete client;
  }
  if (httpCode == HTTP_CODE_OK) // if version received
  { payload.trim();
    if (payload.equals(FirmwareVer)) 
    { Serial.printf("\nDevice already on latest firmware version:%s\n", FirmwareVer);
      return 0;
    } 
    else 
    { Serial.println(payload);
      Serial.println("New firmware detected");
      return 1;
    }
  } 
  return 0;  
}

void IRAM_ATTR isr() 
{ Update_pin.P_cnt += 1;
  Update_pin.sttus = true;
}
