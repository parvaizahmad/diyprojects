/*
 * IoT base Energy Monitoring system.
 * 
 * Parvaiz Ahmad
 * Kindly subscribe my channel it will motivate me to make such projects. 
 * Channel Link: https://www.youtube.com/channel/UCceDc54QaYj6AbEmc13ebVg
 * Channel 2 Link: https://www.youtube.com/channel/UCJibXjBjHdav-hv_-LpnzjQ
 * 
 * You are free to use/modify this code the only condition is you have to subscribe my channel and turn the notifications on for future videos. :)
 */

#define BLYNK_PRINT Serial
#include <ArduinoOTA.h>
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <TimeLib.h>
#include <WidgetRTC.h>

// #define OTA  // Uncomment this if you want to setup OTA feature
const byte interruptPin = 4;
const int pulsesKWh = 1600;
double energyPerPulse = 0.000625; //This is KWh energy per pulse, 1600 pulses for 1KWh
unsigned long long int pulseCount = 0;
unsigned long long int totalPulseCount = 0;
unsigned long long int pulseCountPerDay = 0;
unsigned long long int startTime;
unsigned long long int totalTime;
float loadWatt = 0.0;
float powerMultiplier = 1000.000;
float totalEnergyComsumed = 0.0;
float energyComsumed = 0.0;
float energyComsumedPerDay = 0.0;
float compareEnergy = 0.0;
bool timerStart = false;

char auth[] = "BynkAuthToken";

const char ssid[] = "YourWiFiName";
const char pass[] = "WiFiPassword";

BlynkTimer timer, timer2;
WidgetRTC rtc;

BLYNK_CONNECTED() {
  //get data stored in virtual pin V0 from server
  Blynk.syncVirtual(V0); // For Total energy consuption
  Blynk.syncVirtual(V6); // For Energy ensumption per day
  rtc.begin();
  
}

// restoring counter from server
BLYNK_WRITE(V0)
{
  //restoring int value
  totalPulseCount = param[0].asInt();
}
BLYNK_WRITE(V6)
{
  //restoring int value
  pulseCountPerDay = param[0].asInt();
}

BLYNK_WRITE(V5)
{
  //restoring int value
  //totalPulseCount =  param[0].asInt();
}

void updateValues() {
  String currentTime = String(hour()) + ":" + minute() + ":" + second();
  String currentDate = String(day()) + " " + month() + " " + year();
  if(hour() == 0 && minute() == 0  &&  (second() > 0 || second() < 3)) {
    pulseCountPerDay = 0;
  }
  energyComsumedPerDay = pulseCountPerDay * energyPerPulse;
  energyComsumed = pulseCount * energyPerPulse;
  totalEnergyComsumed = totalPulseCount * energyPerPulse;
  if(totalTime == 0) 
  {
    loadWatt = 0;
  }
  else 
  {
    //loadWatt = 2 * ((powerMultiplier / totalTime ) * 1000);
    loadWatt = 1000.0 * ((3600000.0 / (float)pulsesKWh) / (float)totalTime);
  }
  
  Blynk.virtualWrite(V0, totalPulseCount);
  Blynk.virtualWrite(V1, loadWatt);
  Blynk.virtualWrite(V2, energyComsumed);
  Blynk.virtualWrite(V3, pulseCount);
  Blynk.virtualWrite(V4, totalEnergyComsumed);
  Blynk.virtualWrite(V6, pulseCountPerDay);
  Blynk.virtualWrite(V7, energyComsumedPerDay);
  Blynk.virtualWrite(V8, currentTime);
  Blynk.virtualWrite(V9, currentDate);
  Serial.println("Data Updated.");
} 

void calculateEnergy() {
  if(timerStart) {
      Serial.println("Resetting load. ");
      powerMultiplier = 0.000;
    }
    timerStart = true;
}

void setup() {
  Serial.begin(115200);
  Blynk.begin(auth, ssid, pass, "Server Address", 8080);
  OTASetup("OTASetup", "OTAPassword");
  attachInterrupt(digitalPinToInterrupt(interruptPin), pusleMeasure, RISING);
  setSyncInterval(10 * 60); // Sync interval in seconds (10 minutes)
  timer.setInterval(1000L, updateValues);
  timer2.setInterval(40000L, calculateEnergy);
}

void loop() {
  Blynk.run();
  ArduinoOTA.handle();
  timer.run();
  timer2.run();
}

ICACHE_RAM_ATTR void pusleMeasure() {
  totalTime = millis() - startTime;
  startTime = millis();
  pulseCount++;
  totalPulseCount++;
  pulseCountPerDay++;
  powerMultiplier = 1000.000;
  timerStart = false;
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
