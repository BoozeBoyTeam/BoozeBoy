/*
 Example using the SparkFun HX711 breakout board with a scale
 By: Nathan Seidle
 SparkFun Electronics
 Date: November 19th, 2014
 
 License: This code is public domain but you buy me a beer if you use this and we meet someday (Beerware license).
 
 This is the calibration sketch. Use it to determine the calibration_factor that the main example uses. It also
 outputs the zero_factor useful for projects that have a permanent mass on the scale in between power cycles.
 
 Setup your scale and start the sketch WITHOUT a weight on the scale
 
 Use this calibration_factor on the example sketch
 
 This example assumes pounds (lbs). If you prefer kilograms, change the Serial.print(" lbs"); line to kg. The
 calibration factor will be significantly different but it will be linearly related to lbs (1 lbs = 0.453592 kg).
 
 This example code uses bogde's excellent library:"https://github.com/bogde/HX711"
 bogde's library is released under a GNU GENERAL PUBLIC LICENSE

 Arduino pin 2 -> HX711 CLK
 3 -> DOUT
 5V -> VCC
 GND -> GND

 Most any pin on the Arduino Uno will be compatible with DOUT/CLK.

 The HX711 board can be powered from 2.7V to 5V so the Arduino 5V power should be fine.
 
---------------------------------------------------------------------------------------------------------------------

 Modified By: Wally Proenza
 Booze Boy Prototype
 Date: December 19th, 2019

*/
//branch test b

  #include "HX711.h"
  #include <math.h>
  
  
  #define RANGE_BOUND 100000.0
  #define VALIDATION_THRESHOLD 50
  
  #define DOUT  14
  #define CLK  12
  
  HX711 scale;
  
  //Function names use underscore notation
  void set_calibration_factor(float calibrationFactor);
  float find_calibration_factor(float knownMeasurement, int decimalPlaces);
  float get_calibration_factor(void);  
  void restart(void);
  
  /* These were made into local variables inside find_calibration_factor()
  int counter = 0;
  */
  float lowerBound = 0.0; //-1 * RANGE_BOUND;
  float upperBound = RANGE_BOUND;
  float calibration_factor = (lowerBound + upperBound)/2;
  
 void setup() 
  {
    Serial.begin(115200);

    Serial.println("Remove all weight from the scale now.");
    delay(3000);
    
    Serial.println("HX711 calibration sketch");
  
    Serial.println("Zeroing out the scale...");
    scale.begin(DOUT, CLK);
    scale.set_scale();
    scale.tare(); //Reset the scale to 0
    
    long zero_factor = scale.read_average(); //Get a baseline reading
    Serial.print("Zero factor: "); //This can be used to remove the need to tare the scale. Useful in permanent scale projects.
    Serial.println(zero_factor);
  
    Serial.println("Please place the known weight on the scale.");
    
    delay(5000);
  
    Serial.println("Calibration in progress...");
  
  }

  void loop()
  {
    scale.set_scale(calibration_factor); //Adjust to this calibration factor
    
    set_calibration_factor(find_calibration_factor(20,0)); // 113 is the known weight and 0 is the decimal places wanted for acccuracy  
    

    Serial.println("Calibration complete.");
    Serial.print("Calibration Factor: ");
    Serial.println(get_calibration_factor(),4); //4 is the decimal point places wanted for display
    
   // restart();
    
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
      scale.set_scale(calibration_factor); //Adjust to this calibration factor
      
      scaleOutput = fabs(scale.get_units());
      equalityError = fabs(scaleOutput - knownMeasurement);

         /*******************************************************DEBUGGING***************************************************************/
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

  void restart()
  {
    char restart = 0;

    Serial.println("Enter any character other than 0 to restart calibration: ");
    
    while(restart == 0)
    {
      if(Serial.available() && (Serial.read() != '0'))
      {
        restart = 1;
        lowerBound = -1 * RANGE_BOUND;
        upperBound = RANGE_BOUND;
        calibration_factor = (lowerBound + upperBound)/2 +1;
        Serial.println("Restarting...");
        setup();
      }
    }
  }
