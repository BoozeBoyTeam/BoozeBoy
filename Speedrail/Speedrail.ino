#include <WiFiClientSecure.h>
#include <Losant.h>
#include "HX711.h"

#define RAIL_SIZE 5
#define CLK 12

#define RANGE_BOUND 100000.0
#define VALIDATION_THRESHOLD 50

const char* SENSOR_ID[] = {"S1", "S2", "S3","S4","S5"};
const int DOUT[RAIL_SIZE] = {14, 27, 26, 25, 33};

/******************************** Default Calibration **************************/
float CALFACTOR[RAIL_SIZE] = {396.9192505,
                              396.9192505, 
                              396.9192505, 
                              396.9192505, 
                              396.9192505};
/*******************************************************************************/

/*************************** Calibration Functions *****************************/
void set_calibration_factor(float calibrationFactor);
float find_calibration_factor(float knownMeasurement, int decimalPlaces);
float get_calibration_factor(void);  
/*******************************************************************************/

/********************************* WIFI Functions ******************************/
void connect(void);
/*******************************************************************************/

/************************* Losant Reporting Functions **************************/
void reportWeight(float weight, int tag = 0);
void reportWeight(float weight[]);
void scale_start(void);
void read_values(void);
/*******************************************************************************/

// WiFi credentials.
const char* WIFI_SSID = "xxxx";
const char* WIFI_PASS = "xxxx";

// Losant credentials.
const char* LOSANT_DEVICE_ID = "xxxx"; 
const char* LOSANT_ACCESS_KEY = "xxxx";
const char* LOSANT_ACCESS_SECRET = "xxxx";

WiFiClientSecure wifiClient;

LosantDevice device(LOSANT_DEVICE_ID);
HX711 speedrail[5];

int timeSinceLastRead = 0;
float weightSum = 0.0;
int weightCount = 0;


/*********************** Setup Function *******************************************/
void setup() 
{ 
  Serial.begin(115200); 
  delay(2000); //gives 
  Serial.println("HX711 scale demo");
  
  connect(); 

  scale_calibration(); //prompt for recalibration
  
  scale_start();

  for(int i = 0; i < RAIL_SIZE; i++)
  {
    speedrail[i].begin(DOUT[i], CLK);
  }
  
  scale.set_scale(calibration_factor); //This value is obtained by using the SparkFun_HX711_Calibration sketch
  scale.tare(); //Assuming there is no weight on the scale at start up, reset the scale to 0

  Serial.println("Readings:");

}
/*******************************************************************************/


/****************************** Loop Function **********************************/
void loop() 
{
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
/*******************************************************************************/

/**
 * WIFI CONNECT FUNCTION
 * CONNECTS TO GIVEN SSID AND PASSWORD
 * REPORTS IN SERIAL MONTIOR CONNECTION STATUS
 */

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
  
  /*
  while (!device.connected()) {
    delay(500);
    Serial.print(".");
  }
  */
  
  Serial.println("Connected!");
  Serial.println("This device is now ready for use!");
}


/** FUNCTION TO CREATE JSON AND PUSH TO LOSANT 
 *  WEIGHT = CURRENT WEIGHT VALUE OF A LOAD cell
 *  TAG = LOAD CELL LOCATION IN THE RAIL
 */
void reportWeight(float weight, int tag = 0)// tag default  to 0 for testing 
(
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  root["weight"] = weight;
  root["tag"] = tag;
  device.sendState(root);
}


/** ########IN PROGRESS##########
 * 
 */ 
void scale_start()
{
   
  for(int i = 0; i < RAIL_SIZE; i++)
  {
    speedrail[i].begin(DOUT[i],CLK);
    speedrail[i].set_scale(CALFACTOR[i]);
    speedrail[i].tare();
  }
}
/** ########IN PROGRESS##########
 * report an array of weights
 */
void reportWeight(float weight[])
{

}
/** ########IN PROGRESS##########
 *  read an array of values
 */
void read_values()
{
  for(int i = 0; i< RAIL_SIZE; i++)
  {

  }
}
