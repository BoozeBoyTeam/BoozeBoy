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
float calFactors[RAIL_SIZE] = {396.9192505,          //sensor 1
                               396.9192505,          //sensor 2
                               396.9192505,          //sensor 3
                               396.9192505,          //sensor 4
                               396.9192505};         //sensor 5
/*******************************************************************************/

/******************************** Default Zero Offset **************************/
float zOffset[RAIL_SIZE] = {0,         //sensor 1
                           0,          //sensor 2
                           0,          //sensor 3
                           0,          //sensor 4
                           0};         //sensor 5
/*******************************************************************************/

/*************************** Calibration Functions *****************************/
void speedrail_calibration(void);

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
void speedrail_init(void);
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

  speedrail_calibration(); //recalibration occurs at push of button
  
  speedrail_init();

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

  weightSum += (float)(abs(speedrail[0].get_units()));
  weightCount++;
   //to report every 15 seconds
   if (timeSinceLastRead > 2000) {
    // Take the average reading over the last 15 seconds.
    float averageWeight = weightSum/((float)weightCount);

    // The tmp36 documentation requires the -0.5 offset, but during
    // testing while attached to the Feather, all tmp36 sensors
    // required a -0.52 offset for better accuracy.


    Serial.print("Reading: ");
    Serial.print(speedrail[0].get_units(), 3); //scale.get_units() returns a float
    Serial.print(" grams"); //You can change this to kg but you'll need to refactor the calibration_factor
    Serial.println();


    reportWeight(averageWeight);

    timeSinceLastRead = 0;
    weightSum = 0.0;
    weightCount = 0;
  }
  
//testing
 Serial.print("Reading: ");
    Serial.print(speedrail[0].get_units(), 3); //scale.get_units() returns a float
    Serial.print(" grams"); //You can change this to kg but you'll need to refactor the calibration_factor
    Serial.println();

    
  delay(100);
  timeSinceLastRead += 100;
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
void reportWeight(float weight, int tag = 0) // tag default  to 0 for testing 
{
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  root["weight"] = weight;
  root["tag"] = tag;
  device.sendState(root);
}


/** ########IN PROGRESS##########
 * 
 */ 
void scale_calibration()
{
    Serial.println("Beginning Speedrail Calibration...");
    delay(1000);
    Serial.println("Please make sure all bottles have been removed from the speedrail.");
    delay(10000);
    Serial.println("Taring speedrail now.");
    delay(1000);

    scale_start();
    
    long zero_factor = speedrail[0].read_average(); //Get a baseline reading
    Serial.print("Zero factor: "); //This can be used to remove the need to tare the scale. Useful in permanent scale projects.
    Serial.println(zero_factor);
  
    Serial.println("Please place the known weight on the scale.");
    
    delay(5000);
  
    Serial.println("Calibration in progress...");

    while (condition)
    {
      speedrail[]0.set_scale(calibration_factor); //Adjust to this calibration factor
      
      set_calibration_factor(find_calibration_factor(20,0)); // 113 is the known weight and 0 is the decimal places wanted for acccuracy  
      
  
      Serial.println("Calibration complete.");
      Serial.print("Calibration Factor: ");
      Serial.println(get_calibration_factor(),4); //4 is the decimal point places wanted for display
    }
}

void set_calibration_factor(float calibrationFactor)
{
  calibration_factor = calibrationFactor;
}
  
float find_calibration_factor(float knownMeasurement, int decimalPlaces)
{
    int counter = 0;
    float decimalPlacesLinear = pow(10, (-1 * (decimalPlaces + 1)));
    float scaleOutput = 0.00;
    float equalityError = 0.00;
    
    while(counter < VALIDATION_THRESHOLD)
    {
    speedrail[0].set_scale(calibration_factor); //Adjust to this calibration factor
    
    scaleOutput = fabs(speedrail[0].get_units());
    equalityError = fabs(scaleOutput - knownMeasurement);

    /**********************************************************DEBUGGING************************************************************/
    Serial.print("Reading: ");
    Serial.print(scaleOutput,1); //units to 3 decimal points
    Serial.print(" grams"); //Change this to grams and re-adjust the calibration factor if you follow SI units like a sane person
    Serial.print(" calibration_factor: ");
    Serial.print(get_calibration_factor(),7); //7 is precision required
    Serial.print(" counter: ");
    Serial.print(counter);
    Serial.println();
    /*******************************************************************************************************************************/
    
    if(equalityError <= decimalPlacesLinear)
    {
      counter++;
    }
    else if(scaleOutput > knownMeasurement)
    {
      lowerBound = calibration_factor;
      calibration_factor = (lowerBound + upperBound)/2;
      counter = 0;
    }
    else
    {
      upperBound = calibration_factor;
      calibration_factor = (lowerBound + upperBound)/2;
      counter = 0;
    }
          
  }
  return calibration_factor;
}

float get_calibration_factor(void)
{
  return calibration_factor;
}

void speedrail_init()
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
  for(int i = 0; i < RAIL_SIZE; i++)
  {

  }
}
