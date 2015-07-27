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
 
//CONFIG
//These vars are used to configure the device 
bool save = true; //enable saving of usage times to memory / flash
int cyclesToSleep = 50; //set how many cycles to keep polling at high frequency with no use before sleeping
int delayWhenActive = 50; //set the delay between readings during active use
int delayWhenInActive = 800;  //set the delay between readings when use is below threshold 
int delayWhenBandDisconnected = 3000; //set the delay between readings when the band is disconnected
int sleepDelay = delayWhenActive; //initially set the delay to as being active

//set some initial values before we can take some readings
float minVal = 10;
float maxVal = 19;

//tracking state
bool firstRun = true; //first loop
bool sendMessages = true; //are we sendign messages
int zeroCount = 0; //count how may cycles the band has been inactive for
bool sleeping = false; //keep track of if the thing is sleeping or not.
bool bandConnected= true; //is there a rubber band connected?
int bandMax = 1500; //max likely resistance of a band. Anything over this and the band must be disconnected

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
  
  //set some initial values ??????
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


  // if the LED has been off for a number of cycles, go to inactive delay and record end of usage
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
  int LED=0;

  //if the reading is beyond the expect max, then the band must be disconnected.
  //send to sleep and det delay to disconnected delay
  if (reading > bandMax){
     bandConnected=false; 
     sleeping = true;
     sleepDelay = delayWhenBandDisconnected;
     LED=0;
    }
  else {
      bandConnected=true;
  
      //wait till we get a reading over 400 ?????  
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
      LED = setLEDColourFade(reading);
      //send a message over BLE
      sendMessage(reading, LED);
      
      //uses LED colour for scaling
      //LED is zero so increment counter
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
  }//end bandconencted test
  
  
  
  //echo out to serial
  Serial.print(" LED "); 
  Serial.print(LED);
  Serial.print(" reading "); 
  Serial.println(reading);
  Serial.print(" bandconnected:  "); 
  Serial.println(bandConnected);  
  
  
  /*
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

