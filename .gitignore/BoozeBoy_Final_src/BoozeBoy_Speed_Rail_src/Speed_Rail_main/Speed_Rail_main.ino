#include <WiFiClientSecure.h>
#include <Losant.h>

//Use your own compatible ssid, passwords, keys, etc. separately via header file 
#include "FinalAuthentication.h"
#include "HX711.h"

#define INIT_DELAY 2000

#define REPORT_INTERVAL 2000
#define REPORT_STANDARD_TIME_INCREMENT 100
#define SPEED_RAIL_SIZE 5
#define MIN_READING_THRESH 10 //grams less than this will be read as zero
#define NOISE_SPIKE_THRESH 50 //any increase above this indicates noise spike
#define NOISE_DELAY 1000 //incase of noise spike wait for the spike to leave

#define calibration_factor_s1 367.6414185 //366.5924072 
#define calibration_factor_s2 381.8397827 //381.8333130 
#define calibration_factor_s3 383.4008789 //382.7804565
#define calibration_factor_s4 369.7872314 //396.9192505 
#define calibration_factor_s5 391.1741333 //390.3388672 

#define D1  14
#define D2  26
#define D3  27
#define D4  25
#define D5  33

#define CLK   12

//unsigned long time;

void connect(void);
void reportWeight(String [], float []);
boolean noiseSpike(void);

WiFiClientSecure wifiClient;
LosantDevice device(LOSANT_DEVICE_ID);

HX711 speedrail[SPEED_RAIL_SIZE];
String sensorNames[SPEED_RAIL_SIZE] = {"S1","S2","S3","S4","S5"};
int dout[SPEED_RAIL_SIZE] = {D1, D2, D3, D4, D5};
float calFactor[SPEED_RAIL_SIZE] = {calibration_factor_s1, calibration_factor_s2,
calibration_factor_s3, calibration_factor_s4, calibration_factor_s5}; 

void setup() 
{
  Serial.begin(115200);
  delay(INIT_DELAY);
  connect(); 
  Serial.println("BoozeBoy: Smart Speed Rail demo");
  for(int i = 0; i < SPEED_RAIL_SIZE; i++)
  {
    speedrail[i].begin(dout[i], CLK);
    speedrail[i].set_scale(calFactor[i]); 
    speedrail[i].tare();
  }
}

int timeSinceLastRead = 0;

float readingsSum[SPEED_RAIL_SIZE] = {0.0, 0.0, 0.0, 0.0, 0.0};
int readingsCounter[SPEED_RAIL_SIZE] = {0, 0, 0, 0, 0};
float readingsAverage [SPEED_RAIL_SIZE] = {0.0, 0.0, 0.0, 0.0, 0.0};


float prevValueSnapshot[SPEED_RAIL_SIZE] = {0.0, 0.0, 0.0, 0.0, 0.0};
float current = 0.0;
boolean noiseFlag = false;

void loop() 
{
  bool toReconnect = false;

  if (WiFi.status() != WL_CONNECTED) 
  {
    Serial.println("Disconnected from WiFi");
    toReconnect = true;
  }

  if (!device.connected()) 
  {
    Serial.println("Disconnected from MQTT");
    Serial.println(device.mqttClient.state());
    toReconnect = true;
  }

  if (toReconnect) 
  {
    connect();
  }

  device.loop();
  
  for(int i = 0; i < SPEED_RAIL_SIZE; i++)
  {
     current = (float)abs(speedrail[i].get_units());
     
      if(noiseSpike(prevValueSnapshot[i], current, noiseFlag))
      {
        /*debugging prints
        Serial.println(current);
        Serial.println(prevValueSnapshot[i]);
        */
        Serial.println("Noise detected"); 
        delay(NOISE_DELAY);
        timeSinceLastRead += NOISE_DELAY;
        noiseFlag = true;
        i--;
      }
      else
      {
        /*debugging prints
        Serial.println(current);
        Serial.println(prevValueSnapshot[i]);
        */
        readingsSum[i] += current;
        readingsCounter[i]++;
        prevValueSnapshot[i] = current;
        noiseFlag = false;
      }
  }  
  
  if (timeSinceLastRead > REPORT_INTERVAL)
  {
    // Take the average reading over the last 15 seconds.

      for(int i = 0; i < SPEED_RAIL_SIZE; i++)
      {
        readingsAverage[i] = readingsSum[i]/((float)readingsCounter[i]);

        //Clear empty sensor noise floor
        if (readingsAverage[i] < MIN_READING_THRESH)
        {
          readingsAverage[i] = 0.0;
        }
      }
    
      //real-time check debugging prints
      for(int i = 0; i < SPEED_RAIL_SIZE; i++)
      {       
      Serial.print(sensorNames[i] + ": ");
      Serial.print(speedrail[i].get_units(), 3); //scale.get_units() returns a float
      Serial.print(" grams"); //You can change this to kg but you'll need to refactor the calibration_factor
      Serial.println();
      }
  
      reportWeight(sensorNames , readingsAverage);
      
     for(int i = 0; i < SPEED_RAIL_SIZE; i++)
     {
        readingsSum[i] = 0.0;
        readingsCounter[i] = 0; 
     }
     timeSinceLastRead = 0;
  } 
  delay(REPORT_STANDARD_TIME_INCREMENT);
  timeSinceLastRead += REPORT_STANDARD_TIME_INCREMENT;
}

void connect() 
{
  // Connect to Wifi.
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WIFI_SSID);

  WiFi.begin(WIFI_SSID, WIFI_PASS);

  while (WiFi.status() != WL_CONNECTED) 
  {
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
  while(!device.connected()) 
  {
    delay(500);
    Serial.println(device.mqttClient.state()); // HERE
    Serial.print(".");
  }
  Serial.println("Connected!");
  Serial.println("This device is now ready for use!");
}

void reportWeight(String * namesOfSensors, float * averageDataReadings) 
{
  String speedrailPayload; 
  for(int i = 0; i < SPEED_RAIL_SIZE; i++)
  {
    if(i == 0)
    {
      speedrailPayload += "{\"" + namesOfSensors[i] + "\": \"" + (String)averageDataReadings[i];
    }
    
    else if(i < SPEED_RAIL_SIZE - 1)
    {
      speedrailPayload += "\", \"" +namesOfSensors[i] + "\" : \"" + (String)averageDataReadings[i];
    }
  
    else
    {
      speedrailPayload += "\", \"" +namesOfSensors[i] + "\" : \"" + (String)averageDataReadings[i] + "\"}";
    }
  }
                        
    StaticJsonBuffer<200> jsonBuffer;
    JsonObject& root = jsonBuffer.parseObject(speedrailPayload);
    //debugging prints
    Serial.println(speedrailPayload);
    device.sendState(root);
  
}

boolean noiseSpike(float prevReading, float currentReading, boolean noiseFlagTriggered)
{
  
  if((abs(currentReading -  prevReading) > NOISE_SPIKE_THRESH) && !(noiseFlagTriggered))
  {
    return true;
  }
  return false;
}
