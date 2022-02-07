/*This demo is written for the home automation, it works simply like sonoff switch, you can desing your on wifi switch using this sample code and hardware.
* This application is written for the esp8266/NodeMCU. 
*/

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>
#include <ArduinoOTA.h>

#define OTA

IPAddress APlocal_IP(192, 168, 4, 1);
IPAddress APgateway(192, 168, 4, 1);
IPAddress APsubnet(255, 255, 255, 0);

//Variables
int i = 0;
int statusCode;
String content;
bool reseteeprom = 0;
bool pinState[4];
 
 
//Function Decalration
void launchWeb(void);
void setupWiFi(void);
void resetEEPROM(void);
//Establishing Local server at port 80 whenever required
ESP8266WebServer server(80);
 
void setup()
{
  pinMode(D0,OUTPUT);
  pinMode(D1,OUTPUT);
  pinMode(D2,OUTPUT);
  pinMode(D3,OUTPUT);
  digitalWrite(D0, LOW);
  digitalWrite(D1, LOW);
  digitalWrite(D2, LOW);
  digitalWrite(D3, LOW);
 
  Serial.begin(115200); //Initialising if(DEBUG)Serial Monitor
  Serial.println();
  Serial.println("Disconnecting previously connected WiFi");
  WiFi.disconnect();
  EEPROM.begin(512); //Initialasing EEPROM
  delay(10);
  OTASetup("ESPRelayController", "admin1234");
  resetEEPROM();
  
  String esid;
  String epass = "";
  String ap_ssid;
  String ap_pass = "";
  if(EEPROM.read(0)>0 && EEPROM.read(32) > 0)
  {
    for (int i = 0; i < int(EEPROM.read(193)); ++i)
    {
      esid += char(EEPROM.read(i));
    }
    
    for (int i = 0; i < int(EEPROM.read(194)); ++i)
    {
      epass += char(EEPROM.read(i+32));
    }
  }

  Serial.print("Wifi SSID EEPROM: ");
  Serial.println(esid);
  Serial.print("Wifi Password EEPROM: ");
  Serial.println(epass);
  if(EEPROM.read(96)>0 && EEPROM.read(128) > 0)
  {
    for (int i = 0; i < int(EEPROM.read(195)); ++i)
    {
      ap_ssid += char(EEPROM.read(i+96));
    }
  
    for (int i = 0; i < int(EEPROM.read(196)); ++i)
    {
      ap_pass += char(EEPROM.read(i+128));
    }
  }
  else
  {
    ap_ssid = "MasterMind";
    ap_pass = "12345678";
  }
  
  Serial.print("AP SSID EEPROM: ");
  Serial.println(ap_ssid);
  
  Serial.print("AP Password EEPROM: ");
  Serial.println(ap_pass);
  
  setupWiFi(esid.c_str(), epass.c_str(), ap_ssid.c_str(), ap_pass.c_str());
  launchWeb();
  
  pinState[0] = bitRead(EEPROM.read(510), 0);
  pinState[1] = bitRead(EEPROM.read(510), 1);
  pinState[2] = bitRead(EEPROM.read(510), 2);
  pinState[3] = bitRead(EEPROM.read(510), 3);

  digitalWrite(D1, pinState[0]);
  digitalWrite(D2, pinState[1]);
  digitalWrite(D3, pinState[2]);
  digitalWrite(D0, pinState[3]);
  
}
void loop() 
  {
    ArduinoOTA.handle();
    server.handleClient();
    
  }
 
void launchWeb()
  {
    createWebServer();
    // Start the server
    server.begin();
    Serial.println("Server started");
  }
 
void setupWiFi(const char* wifiname, const char* wifipass, const char* apname, const char* appass)
  {
    // Stop any previous WIFI
    WiFi.disconnect();
  
    // Setting The Wifi Mode
    WiFi.mode(WIFI_AP_STA);
     
    // Starting the access point
    WiFi.softAPConfig(APlocal_IP, APgateway, APsubnet);                 // softAPConfig (local_ip, gateway, subnet)
    Serial.println(WiFi.softAP(apname, appass) ? "AP Ready" : "Ap Failed");                           // WiFi.softAP(ssid, password, channel, hidden, max_connection)     
    Serial.println("AP Name: " + String(apname));
    Serial.println("AP password: " + String(appass));
    // wait a bit
    delay(50);
    Serial.print("Connecting to ");
    Serial.println(wifiname);
  
    WiFi.begin(wifiname, wifipass);
  
    for (int i=0; i<30; ++i)
    {
      if(WiFi.status() == WL_CONNECTED) 
      {
        Serial.println("");
        Serial.println("Wifi Connected.");
        Serial.print("WiFi Connection IP : ");
        Serial.println(WiFi.localIP());
        break;
      }
      else
      {
        delay(500);
        Serial.print(".");
      }
    }
    if(WiFi.status() != WL_CONNECTED){
      Serial.println("");
      Serial.println("Unable to connect to WiFi.");
      Serial.println("Only Access Point Working.");
      WiFi.disconnect();
    }
    // printing the server IP address
    Serial.print("AccessPoint IP : ");
    Serial.println(WiFi.softAPIP());
  }
 
void createWebServer()
  {
      server.on("/wifi_setting", []() {
        String qsid = server.arg("ssid");
        String qpass = server.arg("pass");
        if (qsid.length() > 0 && qpass.length() > 0) {
          Serial.println("clearing eeprom");
          for (int i = 0; i < 96; ++i) {
            EEPROM.write(i, 0);
          }
          EEPROM.write(193, 0);
          EEPROM.write(194, 0);
          Serial.println(qsid);
          Serial.println("");
          Serial.println(qpass);
          Serial.println("");
   
          Serial.println("writing eeprom ssid:");
          for (int i = 0; i < qsid.length(); ++i)
          {
            EEPROM.write(i, qsid[i]);
            Serial.print("Wrote: ");
            Serial.println(qsid[i]);
          }
          Serial.println("writing eeprom pass:");
          for (int i = 0; i < qpass.length(); ++i)
          {
            EEPROM.write(32 + i, qpass[i]);
            Serial.print("Wrote: ");
            Serial.println(qpass[i]);
          }
          EEPROM.write(193, qsid.length());
          EEPROM.write(194, qpass.length());
          EEPROM.commit();
   
          content = "{\"Success\":\"saved to eeprom... reset to boot into new wifi\"}";
          statusCode = 200;
          
        } else {
          content = "{\"Error\":\"404 not found\"}";
          statusCode = 404;
          Serial.println("Sending 404");
        }
        server.sendHeader("Access-Control-Allow-Origin", "*");
        server.send(statusCode, "application/json", content);
        delay(1000);
        ESP.restart();
   
      });
  
      server.on("/ap_setting", []() {
        String ap_ssid = server.arg("ap_ssid");
        String ap_pass = server.arg("ap_pass");
        if (ap_ssid.length() > 0 && ap_pass.length() > 0) {
          Serial.println("clearing eeprom");
          for (int i = 96; i < 192; ++i) {
            EEPROM.write(i, 0);
          }
          EEPROM.write(195, 0);
          EEPROM.write(196, 0);
          Serial.println(ap_ssid);
          Serial.println("");
          Serial.println(ap_pass);
          Serial.println("");
   
          Serial.println("writing eeprom ssid:");
          for (int i = 0; i < ap_ssid.length(); ++i)
          {
            EEPROM.write(96 + i, ap_ssid[i]);
            Serial.print("Wrote: ");
            Serial.println(ap_ssid[i]);
          }
          Serial.println("writing eeprom pass:");
          for (int i = 0; i < ap_pass.length(); ++i)
          {
            EEPROM.write(128 + i, ap_pass[i]);
            Serial.print("Wrote: ");
            Serial.println(ap_pass[i]);
          }
          EEPROM.write(195, ap_ssid.length());
          EEPROM.write(196, ap_pass.length());
          EEPROM.commit();
   
          content = "{\"Success\":\"saved to eeprom... reset to boot into new wifi\"}";
          statusCode = 200;
          
        } else {
          content = "{\"Error\":\"404 not found\"}";
          statusCode = 404;
          Serial.println("Sending 404");
        }
        server.sendHeader("Access-Control-Allow-Origin", "*");
        server.send(statusCode, "application/json", content);
        delay(1000);
        ESP.restart();
      });

      server.on("/wifi_setting_reset", []() 
      {
        reseteeprom = 1;
        server.send(200, "text/plain", "");
        resetEEPROM();
        delay(500);
        ESP.restart();
      });
      server.on("/D1_on", []() 
      {
        digitalWrite(D1, HIGH);
        Serial.println("D1 is high");
        byte EEPROMbyte = EEPROM.read(510);
        bitWrite(EEPROMbyte, 0, 1);
        EEPROM.write(510, EEPROMbyte);
        EEPROM.commit();
        server.send(200, "text/plain", "");
      });
      server.on("/D1_off", []() 
      {
        digitalWrite(D1, LOW);
        Serial.println("D1 is low");
        byte EEPROMbyte = EEPROM.read(510);
        bitWrite(EEPROMbyte, 0, 0);
        EEPROM.write(510, EEPROMbyte);
        EEPROM.commit();
        server.send(200, "text/plain", "");
      });

      
      server.on("/D2_on", []() 
      {
        digitalWrite(D2, HIGH);
        Serial.println("D2 is high");
        byte EEPROMbyte = EEPROM.read(510);
        bitWrite(EEPROMbyte, 1, 1);
        EEPROM.write(510, EEPROMbyte);
        EEPROM.commit();
        server.send(200, "text/plain", "");
      });
      server.on("/D2_off", []() 
      {
        digitalWrite(D2, LOW);
        Serial.println("D2 is low");
        byte EEPROMbyte = EEPROM.read(510);
        bitWrite(EEPROMbyte, 1, 0);
        EEPROM.write(510, EEPROMbyte);
        EEPROM.commit();
        server.send(200, "text/plain", "");
      });

      
      server.on("/D3_on", []() 
      {
        digitalWrite(D3, HIGH);
        Serial.println("D3 is high");
        byte EEPROMbyte = EEPROM.read(510);
        bitWrite(EEPROMbyte, 2, 1);
        EEPROM.write(510, EEPROMbyte);
        EEPROM.commit();
        server.send(200, "text/plain", "");
      });
      server.on("/D3_off", []() 
      {
        digitalWrite(D3, LOW);
        Serial.println("D3 is low");
        byte EEPROMbyte = EEPROM.read(510);
        bitWrite(EEPROMbyte, 2, 0);
        EEPROM.write(510, EEPROMbyte);
        EEPROM.commit();
        server.send(200, "text/plain", "");
        
      });

      server.on("/D0_on", []() 
      {
        digitalWrite(D0, HIGH);
        Serial.println("D0 is high");
        byte EEPROMbyte = EEPROM.read(510);
        bitWrite(EEPROMbyte, 3, 1);
        EEPROM.write(510, EEPROMbyte);
        EEPROM.commit();
        server.send(200, "text/plain", "");
      });
      server.on("/D0_off", []() 
      {
        digitalWrite(D0, LOW);
        Serial.println("D0 is low");
        byte EEPROMbyte = EEPROM.read(510);
        bitWrite(EEPROMbyte, 3, 0);
        EEPROM.write(510, EEPROMbyte);
        EEPROM.commit();
        server.send(200, "text/plain", "");
        
      });

      server.on("/get_ip", []() 
      {
        IPAddress ip_address;
        if (WiFi.status() == WL_CONNECTED)
        {
          ip_address = WiFi.localIP();
        }
        else if(WiFi.status() != WL_CONNECTED)
        {
          ip_address = WiFi.softAPIP();
        }
        
        String IP = String(ip_address[0]) + String(".") + String(ip_address[1]) + String(".") + String(ip_address[2]) + String(".") + String(ip_address[3]);
        server.send(200, "text/plain", IP);
      });
      server.on("/status", []() 
      {
        String Status = String(pinState[3]) + ":" + String(pinState[0]) + ":" + String(pinState[1]) + ":" + String(pinState[2]) + "status";
        server.send(200, "text/plain", Status);
      });
  }

  void resetEEPROM(void)
  {
    
    if(EEPROM.read(511) != 0 || reseteeprom == 1)
    {
      Serial.println("Resetting EEPROM");
      pinMode(LED_BUILTIN, OUTPUT);
      for (int i = 0; i < 512; ++i) 
      {
        EEPROM.write(i, 0);
        digitalWrite(LED_BUILTIN, HIGH);
        delay(20);
        digitalWrite(LED_BUILTIN, LOW);
      }
      EEPROM.commit();
    }
    else return;
  }

#ifdef OTA
void OTASetup(char host[], char passwrd[])
{
  ArduinoOTA.setHostname(host);
  ArduinoOTA.setPassword(passwrd);
  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else { // U_SPIFFS
      type = "filesystem";
    }

    // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
    Serial.println("Start updating " + type);
  });

  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });

  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });

  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      Serial.println("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      Serial.println("End Failed");
    }
  });
  
  // initialize OTA capability
  ArduinoOTA.begin();
}
#endif
