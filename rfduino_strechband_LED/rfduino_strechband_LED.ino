/*
Read resistance of strech sensor and report it over BLE
Show on LED
Will Simm w.simm@lancs.ac.uk 

*/

#include <RFduinoBLE.h> 
//#include <SPI.h>
//#include <SD.h>
#include<stdlib.h>




// the value of the 'other' resistor in the divider with the strech sensor
#define SERIESRESISTOR 10000    
 
//set some initial values
float minVal = 10;
float maxVal = 19;

//set the polling frequency for active use and inactive use
int delayWhenActive = 50;
int delayWhenInActive = 800;
int sleepDelay = delayWhenActive;

bool save = true;

bool firstRun = true;
bool sendMessages = true;
//count how may cycles the band has been inactive for
int zeroCount = 0;
int cyclesToSleep = 50;

//keep track of if the thing is sleeping or not.
bool sleeping = false;

void setup() {
  // this is the data we want to appear in the advertisement
  // (if the deviceName and advertisementData are too long to fix into the 31 byte
  // ble advertisement packet, then the advertisementData is truncated first down to
  // a single byte, then it will truncate the deviceName)
  RFduinoBLE.advertisementData = "temp";

  // start the BLE stack
  RFduinoBLE.begin();
  
  Serial.begin(9600);
  
  //output pins
  pinMode(3, OUTPUT); // sensor power
  digitalWrite(3, LOW);
  pinMode(4, OUTPUT); //LED Pin
  
  //set some initial values NEEDED?
  minVal = readStretch();
  maxVal = minVal+10;
  
  /*
  Serial.print(" min flash ");
  Serial.println(findMinFlashPage(251));
  */
  
  if(save){
    //start of usage
    saveUsage( millis() );
  }
  
}

void loop() {
    // sample once per second
  RFduino_ULPDelay( sleepDelay );

  // if the LED has been off for a number of cycles, go to long delay and record end of usage
  if (zeroCount > cyclesToSleep){
     sleepDelay = delayWhenInActive;
    if(save && !sleeping){     
       //end of usage - knock the time back by the timeout period
       saveUsage( millis() - (delayWhenActive*cyclesToSleep) );
       sleeping = true;
    }
  }

  //RFduinoBLE.sendFloat(readStretch());
  //read the sensor
  float reading = readStretch();
  
  //wait till we get a reading over 400 NOT NEEDED?  
  float threshold = 400;
  //if its the first run take a resting reading and use as min value
  if (firstRun == true){
    if (reading > threshold){
       minVal = reading;
       firstRun = false; 
    }
  }
  
  //update the min / max bounds if needed
  if (reading > maxVal){
   maxVal=reading; 
  }
  if (reading < minVal){
    minVal = reading;
  }
  //set the LED
  int LED = setLEDColourFade(reading);
  //send a message over BLE
  sendMessage(reading, LED);
  
  if (LED == 0){
    zeroCount++; 
  }
  else{ 
    //bring out of long delay and record start of usage
    sleepDelay = delayWhenActive;
    zeroCount = 0;
    
    if (sleeping){
      sleeping=false;
       if(save){
        saveUsage( millis() );
       }
    }
    
  }
  
  
  
  
  //echo out to serial
  Serial.print(" LED "); 
  Serial.println(LED);
  /*Serial.print(" reading "); 
  Serial.print(reading);
  Serial.print(" max "); 
  Serial.print(maxVal);
  Serial.print(" min "); 
  Serial.println(minVal);
  Serial.print(" zerocount "); 
  Serial.print(zeroCount);  
  Serial.print(" ram "); 
  Serial.println(ramUsed());  
    Serial.print(" flash "); 
  Serial.println(flashUsed());  
      Serial.print(" usageindex "); 
  Serial.println(usageIndex);*/

}

//Write out the value to the LED
void setLEDColour(int LED){
    analogWrite(4, LED); // green
}


int setLEDColourFade(float reading){
  int LED;
  // add a fudge or deadzone of minimum strech
  float fudge = minVal + 100;
  if (fudge >= maxVal){
   fudge = maxVal;
  } 
  //scale the reading to the 255 of the LED PWM
  if (reading > fudge){
     LED = (int) (reading - fudge) / (maxVal-fudge) *255 ;
  }
  else {
    LED=0;
  }
  
  setLEDColour(LED);
  return LED;
  
}

//send a BLE message only if the band is being stretched, send one zero and then hold if not
void sendMessage(float reading, int LED){
  if (LED >0){
    RFduinoBLE.sendFloat(reading);
    sendMessages = true;
  }
  if (sendMessages && (LED == 0)){
    RFduinoBLE.sendFloat(0);
    sendMessages = false;
  }
}

//read the stretch sensor
float readStretch(){
  //Turn on sensor
  digitalWrite(3, HIGH);
  //read in current strech from ADC
  float reading = analogRead(2);
  //turn off sensor
  digitalWrite(3, LOW);
 
  // convert the value to resistance
  reading = (1023 / reading)  - 1;
  reading = SERIESRESISTOR / reading;
  
  return reading;
   
}
/*
//record the usage
void recordUsage(){ 
   usage[usageIndex] = millis();
   usageIndex++; 
   if (usageIndex > 256){
    //write out to flash
    
     usageIndex=0; 
   }
}*/
