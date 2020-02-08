/*
 Example using the SparkFun HX711 breakout board with a scale
 By: Nathan Seidle
 SparkFun Electronics
 Date: November 19th, 2014
 License: This code is public domain but you buy me a beer if you use this and we meet someday (Beerware license).

 This example demonstrates basic scale output. See the calibration sketch to get the calibration_factor for your
 specific load cell setup.

 This example code uses bogde's excellent library:"https://github.com/bogde/HX711"
 bogde's library is released under a GNU GENERAL PUBLIC LICENSE

 The HX711 does one thing well: read load cells. The breakout board is compatible with any wheat-stone bridge
 based load cell which should allow a user to measure everything from a few grams to tens of tons.
 Arduino pin 2 -> HX711 CLK
 3 -> DAT
 5V -> VCC
 GND -> GND

 The HX711 board can be powered from 2.7V to 5V so the Arduino 5V power should be fine.

 ---------------------------------------------------------------------------------------------------------------------

 Modified By: Wally Proenza
 Booze Boy Prototype
 Date: December 19th, 2019

*/
/**
   Copyright (c) 2016 Losant IoT. All rights reserved.
   https://www.losant.com
*/

#include <WiFiClientSecure.h>
#include <Losant.h>

#include "HX711.h"

#define calibration_factor 396.9192505 //1M value was obtained using the SparkFun_HX711_Calibration sketch

#define DOUT  14
#define CLK   12

void connect(void);
void reportWeight(float);

// WiFi credentials.
const char* WIFI_SSID = "xxxx";
const char* WIFI_PASS = "xxxx";

// Losant credentials.
const char* LOSANT_DEVICE_ID = "5e2e1088a490d00006beca85"; //5e2e1088a490d00006beca85
const char* LOSANT_ACCESS_KEY = "fafe0226-f43f-4331-91e3-ec0c5b4512f3";
const char* LOSANT_ACCESS_SECRET = "7d79d64f4ee8f13ad8ba3d7ac6ccfc2f532b277841e839dd807f3ef6be0a2bf5";

WiFiClientSecure wifiClient;

LosantDevice device(LOSANT_DEVICE_ID);

HX711 scale;

void setup() {
  Serial.begin(115200); //9600
   //delay(5000); //esp8266 serial connection delay
      delay(2000);
      connect(); 
  Serial.println("HX711 scale demo");

  scale.begin(DOUT, CLK);
   //delay(10000); //esp8266 serial connection delay
  scale.set_scale(calibration_factor); //This value is obtained by using the SparkFun_HX711_Calibration sketch
   //delay(10000); //esp8266 serial connection delay
  scale.tare(); //Assuming there is no weight on the scale at start up, reset the scale to 0
   //delay(10000); //esp8266 serial connection delay

  Serial.println("Readings:");
}

    int timeSinceLastRead = 0;
    float weightSum = 0.0;
    int weightCount = 0;

void loop() {
  bool toReconnect = false;

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Disconnected from WiFi");
    toReconnect = true;
  }

  if (!device.connected()) {
    Serial.println("Disconnected from MQTT");
    Serial.println(device.mqttClient.state());
    toReconnect = true;
  }

  if (toReconnect) {
    connect();
  }

  device.loop();

  weightSum += (float)(abs(scale.get_units()));
  weightCount++;
   //to report every 15 seconds
   if (timeSinceLastRead > 2000) {
    // Take the average reading over the last 15 seconds.
    float averageWeight = weightSum/((float)weightCount);

    // The tmp36 documentation requires the -0.5 offset, but during
    // testing while attached to the Feather, all tmp36 sensors
    // required a -0.52 offset for better accuracy.


    Serial.print("Reading: ");
    Serial.print(scale.get_units(), 3); //scale.get_units() returns a float
    Serial.print(" grams"); //You can change this to kg but you'll need to refactor the calibration_factor
    Serial.println();


    reportWeight(averageWeight);

    timeSinceLastRead = 0;
    weightSum = 0.0;
    weightCount = 0;
  }
  
//testing
 Serial.print("Reading: ");
    Serial.print(scale.get_units(), 3); //scale.get_units() returns a float
    Serial.print(" grams"); //You can change this to kg but you'll need to refactor the calibration_factor
    Serial.println();

    
  delay(100);
  timeSinceLastRead += 100;
  
/*
  Serial.print("Reading: ");
  Serial.print(scale.get_units(), 3); //scale.get_units() returns a float
  Serial.print(" grams"); //You can change this to kg but you'll need to refactor the calibration_factor
  Serial.println();
*/
}

void connect() {

  // Connect to Wifi.
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WIFI_SSID);

  WiFi.begin(WIFI_SSID, WIFI_PASS);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  Serial.println();
  Serial.print("Connecting to Losant...");

  device.connectSecure(wifiClient, LOSANT_ACCESS_KEY, LOSANT_ACCESS_SECRET);

//debugging
  while(!device.connected()) {
    delay(500);
    Serial.println(device.mqttClient.state()); // HERE
    Serial.print(".");
}
/*
  while (!device.connected()) {
    delay(500);
    Serial.print(".");
  }
*/
  Serial.println("Connected!");
  Serial.println("This device is now ready for use!");
}

void reportWeight(float weight) {
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  root["weight"] = weight;
  device.sendState(root);
}
