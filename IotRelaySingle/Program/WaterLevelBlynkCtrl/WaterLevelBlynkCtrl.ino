/* This application is written to control relays/appliences using the blynk server.
 * This application is successfully deployed on the soler/UPS controller.
 * Developer: Parvaiz Ahmad.
 */

#define BLYNK_PRINT Serial // Enables Serial Monitor
#include <ArduinoOTA.h>
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
//#include <TimeLib.h>

#define OTA
BlynkTimer timer, sensorTimer;

//char auth[] = "UAoA8gqdJ_pGi9CAcKmhHzrpoD8lfxJP"; //Local Server on blynk.iot-cm.com

char auth[] = "pLQ0q4BEJx5YaJGA7WgHxlv90MK50uo_"; //Local Server on miniserver
bool overrideSensor = false; // Flag to indicate if app control overrides the sensor
const char ssid[] = "NETGEAR52";
const char pass[] = "Roshan1qaz123";


String newHostname = "WaterMotor";

void sendRSSI()
{
  int rssi = WiFi.RSSI();
  //rssi = map(rssi, 0, -110, 0, 100);
  //lrssi = 150 - (5/3)*rssi;
  
  Blynk.virtualWrite(V0,rssi);
}

void checkSensorState(){
  // If override is enabled, skip sensor control
  if (overrideSensor) {
    return;
  }

  // read pin 13 state if high set pin 2 high
  if(digitalRead(13) == HIGH)
  {
    Blynk.virtualWrite(V1,1);
    digitalWrite(D1,HIGH);
    Serial.println("Pump is on");
  }
  else
  {
    Blynk.virtualWrite(V1,0);
    digitalWrite(D1,LOW);
    Serial.println("Pump is off");
  }
}

BLYNK_CONNECTED() {
  Blynk.syncAll();
  Blynk.syncVirtual(V1);
}

BLYNK_APP_CONNECTED() {
  Blynk.syncAll();
}

BLYNK_WRITE(V1) 
{
  bool state = param[0].asInt();
  
  if(state && overrideSensor)
  {
    digitalWrite(D1,HIGH);
    Blynk.notify("Water Motor is on now.");
    Serial.println("Water Motor is on now.");
    Blynk.virtualWrite(V1,1);
  }
  else if(!state && overrideSensor)
  {
    digitalWrite(D1,LOW);
    Blynk.notify("Water Motor is off now.");
    Serial.println("Water Motor is off now.");
    Blynk.virtualWrite(V1,0);
  }
}

BLYNK_WRITE(V2) {
  bool state = param[0].asInt();
  if(state)
  {
    overrideSensor = true; // Enable override when the app sends a command
  }
  else if(!state)
  {
    overrideSensor = false; // Disable override when the app sends a command to turn off
  }
}

void setup()
{ 
  pinMode(D2, OUTPUT);
  pinMode(D1, OUTPUT);
  // set pin 13 to input mode pull down
  pinMode(13, INPUT);
  // See the connection status in Serial Monitor
  Serial.begin(115200);

  //Blynk.begin(auth, ssid, pass, IPAddress(49,229,108,138) ,8080); // blynk.iot-cm.com
  Blynk.begin(auth, ssid, pass, IPAddress(192,168,1,11) ,8080); // blynk.iot-cm.com
  
  Serial.printf("Default hostname: %s\n", WiFi.hostname().c_str());
  WiFi.hostname(newHostname.c_str());
  Serial.printf("New hostname: %s\n", WiFi.hostname().c_str());
  
  OTASetup("WaterMotor", "admin1234");
 
  timer.setInterval(1000L, sendRSSI);
  sensorTimer.setInterval(1000L, checkSensorState);
}

void loop()
{
  // All the Blynk Magic happens here...
  Blynk.run();
  ArduinoOTA.handle();
  timer.run();
  sensorTimer.run();
  // to avoid delay() function!
  
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
