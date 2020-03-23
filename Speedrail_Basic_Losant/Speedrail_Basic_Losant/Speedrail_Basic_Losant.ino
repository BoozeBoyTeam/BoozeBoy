#include <WiFiClientSecure.h>
#include <Losant.h>

#include "HX711.h"

//Use your own compatible ssid, passwords, keys, etc. separately via header file 
#include "Authentication.h"

#define calibration_factor_s1 366.5924072 
#define calibration_factor_s2 381.8333130 
#define calibration_factor_s3 382.7804565 
#define calibration_factor_s4 396.9192505 //severly inaccurate
#define calibration_factor_s5 390.3388672 

#define D1  14
#define D2  26
#define D3  27
#define D4  25
#define D5  33

#define CLK   12

void connect(void);
void reportWeight(float, float, float, float, float);

/* WiFi credentials. No longer displayed here. Create seperate header file.
const char* WIFI_SSID = "XXX";
const char* WIFI_PASS = "XXX";

// Losant credentials.
const char* LOSANT_DEVICE_ID = "XXX";
const char* LOSANT_ACCESS_KEY = "XXX";
const char* LOSANT_ACCESS_SECRET = "XXX";
*/

WiFiClientSecure wifiClient;

LosantDevice device(LOSANT_DEVICE_ID);

HX711 s1, s2, s3, s4, s5;

void setup() {
  
  Serial.begin(115200); //9600
   //delay(5000); //esp8266 serial connection delay
      delay(2000);
      connect(); 
 
  Serial.println("HX711 scale demo");

  s1.begin(D1, CLK);
    s2.begin(D2, CLK);
      s3.begin(D3, CLK);
        s4.begin(D4, CLK);
          s5.begin(D5, CLK);
          
   //delay(10000); //esp8266 serial connection delay
   
  s1.set_scale(calibration_factor_s1); 
    s2.set_scale(calibration_factor_s2);
      s3.set_scale(calibration_factor_s3);
        s4.set_scale(calibration_factor_s4);
          s5.set_scale(calibration_factor_s5);
   
   //delay(10000); //esp8266 serial connection delay
   
  s1.tare(); //Assuming there is no weight on the scale at start up, reset the scale to 0
    s2.tare();
      s3.tare();
        s4.tare();
          s5.tare();
          
   //delay(10000); //esp8266 serial connection delay

  Serial.println("Readings:");
}

    int timeSinceLastRead = 0;
    
    float weightSumS1 = 0.0;
      float weightSumS2 = 0.0;
        float weightSumS3 = 0.0;
          float weightSumS4 = 0.0;
            float weightSumS5 = 0.0;
            
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

  weightSumS1 += (float)(abs(s1.get_units()));
    weightSumS2 += (float)(abs(s2.get_units()));
      weightSumS3 += (float)(abs(s3.get_units()));
        weightSumS4 += (float)(abs(s4.get_units()));
          weightSumS5 += (float)(abs(s5.get_units()));
      
  weightCount++;
   //to report every 15 seconds
   if (timeSinceLastRead > 2000) {
    // Take the average reading over the last 15 seconds.
    float averageWeightS1 = weightSumS1/((float)weightCount);
      float averageWeightS2 = weightSumS2/((float)weightCount);
        float averageWeightS3 = weightSumS3/((float)weightCount);
          float averageWeightS4 = weightSumS4/((float)weightCount);
            float averageWeightS5 = weightSumS5/((float)weightCount);

    // The tmp36 documentation requires the -0.5 offset, but during
    // testing while attached to the Feather, all tmp36 sensors
    // required a -0.52 offset for better accuracy.


    Serial.print("Scale 1: ");
    Serial.print(s1.get_units(), 3); //scale.get_units() returns a float
    Serial.print(" grams"); //You can change this to kg but you'll need to refactor the calibration_factor
    Serial.println();

    Serial.print("Scale 2: ");
    Serial.print(s2.get_units(), 3); //scale.get_units() returns a float
    Serial.print(" grams"); //You can change this to kg but you'll need to refactor the calibration_factor
    Serial.println();

    Serial.print("Scale 3: ");
    Serial.print(s3.get_units(), 3); //scale.get_units() returns a float
    Serial.print(" grams"); //You can change this to kg but you'll need to refactor the calibration_factor
    Serial.println();

    Serial.print("Scale 4: ");
    Serial.print(s4.get_units(), 3); //scale.get_units() returns a float
    Serial.print(" grams"); //You can change this to kg but you'll need to refactor the calibration_factor
    Serial.println();

    Serial.print("Scale 5: ");
    Serial.print(s5.get_units(), 3); //scale.get_units() returns a float
    Serial.print(" grams"); //You can change this to kg but you'll need to refactor the calibration_factor
    Serial.println();

    reportWeight(averageWeightS1, averageWeightS2, averageWeightS3, averageWeightS4, averageWeightS5);
     /* reportWeight(averageWeightS2);
        reportWeight(averageWeightS3);
          reportWeight(averageWeightS4);
            reportWeight(averageWeightS5); */

    timeSinceLastRead = 0;
    
    weightSumS1 = 0.0;
      weightSumS2 = 0.0;
          weightSumS3 = 0.0;
              weightSumS4 = 0.0;
                  weightSumS5 = 0.0;
    
    weightCount = 0;
  }
  
/*testing
 Serial.print("Reading: ");
    Serial.print(scale.get_units(), 3); //scale.get_units() returns a float
    Serial.print(" grams"); //You can change this to kg but you'll need to refactor the calibration_factor
    Serial.println();
*/
    
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

void reportWeight(float weightS1, float weightS2, float weightS3, float weightS4, float weightS5) 
{

  String speedrail =  "{\"Bottle_1\" : \"" + (String)weightS1 
                      + "\", \"Bottle_2\" : \"" + (String)weightS2
                      + "\", \"Bottle_3\" : \"" + (String)weightS3 
                      + "\", \"Bottle_4\" : \"" + (String)weightS4
                      + "\", \"Bottle_5\" : \"" + (String)weightS5 
                      + "\"}";  
                      
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(speedrail);
  
  device.sendState(root);
  
  //for debugging only
  Serial.println(speedrail);
  
}

/*
float weight
*/
/*
{

  "bottle_1" : weightS1,
  "bottle_2" : weightS2,
  "bottle_3" : weightS3,
  "bottle_4" : weightS4,
  "bottle_5" : weightS5,
  
  
}
*/
