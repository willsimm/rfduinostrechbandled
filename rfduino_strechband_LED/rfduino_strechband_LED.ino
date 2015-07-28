/*
Read resistance of strech sensor and report it over BLE
Show on LED
record history of usage to flash
read out history on request

myclasp.org.uk

Will Simm w.simm@lancs.ac.uk & Adrian Gradinar 2015

*/

#include <RFduinoBLE.h>
#include<stdlib.h>


//int live_stream_value = 0;
bool mode_active = false;
bool send_history = false;

//int val = 0;
//int connection_led = 2;
//int data_was_requesed_led = 3;
//enum modes {ACTIVE, PASSIVE};




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
float minVal =500;
float maxVal =510;

//tracking state
bool firstRun = true; //first loop
bool sendMessages = true; //are we sendign messages
int zeroCount = 0; //count how may cycles the band has been inactive for
bool sleeping = false; //keep track of if the thing is sleeping or not.
bool bandConnected= true; //is there a rubber band connected?
int bandMax = 1500; //max likely resistance of a band. Anything over this and the band must be disconnected






void setup() {
  Serial.begin(9600);
  RFduinoBLE.deviceName = "Snapino";

  //pinMode(connection_led, OUTPUT);
  //pinMode(data_was_requesed_led, OUTPUT);
  
  RFduinoBLE.begin();
  
  
   //output pins
  pinMode(3, OUTPUT); // sensor power
  digitalWrite(3, LOW);
  pinMode(4, OUTPUT); //LED Pin
  
  //set some initial values ??????
//  minVal = readStretch();
  //maxVal = minVal+10;
  
  /*
  Serial.print(" min flash ");
  Serial.println(findMinFlashPage(251));
  */
  
  if(save){
    //start of usage
    saveUsage( millis() );
  }
  
  
  //fill 4 pages of flash for testing
  fillFlashMemory(4);
  
}

void loop() {
  //if history has been requested then get on with that
  if(send_history){
    sendHistoryBLE();
  }
  //else do the normal loop
  else{
    //set the variable sleep delay
    RFduino_ULPDelay( sleepDelay );
    
    
    // if the LED has been off for a number of cycles, go to inactive delay and record end of usage
    if (zeroCount > cyclesToSleep){
     
       sleepDelay = delayWhenInActive;
      if( !sleeping){     
         //end of usage - knock the time back by the timeout period
         saveUsage( millis() - (delayWhenActive*cyclesToSleep) );
         sleeping = true;
      }
    }
    
    //read the sensor
    float reading = readStretch();
    int LED=0;
  
    //if the reading is beyond the expect max, then the band must be disconnected.
    //send to sleep and det delay to disconnected delay
    //reset params
    if (reading > bandMax){
      
       if (bandConnected){
         bandConnected=false; 
         Serial.println(" band disconnected "); 
       }
       
       if ( !sleeping){
         saveUsage( millis());
         sleeping = true;
         sleepDelay = delayWhenBandDisconnected;
         LED=0;
         setLEDColour(LED);
       }
      }
    else {
        if (!bandConnected){
          // band has been connected!
          bandConnected=true;
          Serial.println(" band connected "); 
          resetParams();
        }
    
        //wait till we get a reading over 400 ?????  
        float threshold = 400;
        //if its the first run take a resting reading and use as min value
        if (firstRun == true){
          Serial.println(" first run ");
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
        if(mode_active){
          //transmitLiveStream();
          sendLiveMessage(reading, LED);
        }
        
        
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
    
    
    /*
    //echo out to serial
    Serial.print(" LED "); 
    Serial.print(LED);
    Serial.print(" reading B"); 
    Serial.println(reading);
    Serial.print(" bandconnected:  "); 
    Serial.println(bandConnected);    
    */
    }// end history check 
}



void RFduinoBLE_onAdvertisement(bool start){}

void RFduinoBLE_onConnect(){
  //turnOnLed(connection_led);
}

void RFduinoBLE_onDisconnect(){
  //turnOffLed(connection_led);
  //turnOffLed(data_was_requesed_led);
  mode_active = false;
}

void RFduinoBLE_onReceive(char *data, int len){
  //turnOffLed(connection_led);
  //turnOnLed(data_was_requesed_led);
  Serial.print("flag recieved, value:");
  Serial.println(String(data[0]));
  readFlag(data[0]);
}
/*
void turnOffLed(int led){
  digitalWrite(led, LOW);
}

void turnOnLed(int led){
  digitalWrite(led, HIGH);
}*/

void readFlag(int flag){

  switch(flag){
    case 0:
      //set Snapino's mode to PASSIVE
      mode_active = false;
      Serial.println(" mode passive");
      sendMessage("Mode passive");
      break;
    case 1:
      //set Snapino's mode to ACTIVE
      Serial.println(" mode active");
      sendMessage("Mode active");
      //send float readings 
      
      
      mode_active = true;
      break;
    case 2:      
      //ends live data streaming
      //send current time twice as a pair
      //then send historic values as pairs
      //send a pair of zeros to finish
       Serial.println(" mode history");
      send_history = true;  
      break;
    default:
      //why are we sending unaccounted flags?!?
      Serial.print("Doh...do something else!");
    break;
  }
}

void sendMessage(String message){
  int len = message.length() + 1;
  char dataToSend[len];
  message.toCharArray(dataToSend, len);
  while(!RFduinoBLE.send(dataToSend, len-1)){
    RFduino_ULPDelay(100);
    //val++;
  }
}


/* NOT USED sendMessage() instead
void transmitLiveStream(){
  RFduinoBLE.sendInt(live_stream_value++);
}*/

void sendHistoryBLE(){
  Serial.println("History requested!");
  //first send current timestamp
   String toSend = String(-1) + "/" + String(millis());
   sendMessage(toSend);
   
   //now read out the contents of all the flash pages we've written
   readOutWholeFlash();
   //and whats left in the RAM buffer
   readOutRAMValues();
  
  /*
   for (int c = 0; c < 100000; c++){
     toSend = String(random(10000,50000)) +  "/" + String(random(10000,50000));
     sendMessage(toSend);
   }*/
   
   
   //finish with current timestamp
   toSend = String(-2) + "/" + String(millis());
   sendMessage(toSend);
   send_history = false;
   //Serial.println(val);
   Serial.println("History sent!");
}






//Write out the value to the LED
void setLEDColour(int LED){
    analogWrite(4, LED); // green
}


int setLEDColourFade(float reading){
  
  /*Serial.print("reading");
  Serial.println(reading);
  Serial.print("minVal");
  Serial.println(minVal);
  Serial.print("maxal");
  Serial.println(maxVal);  */
  
  
  
  
  
  int LED;
  // add a fudge or deadzone of minimum strech
  float fudge = minVal + 100;
  //Serial.print("fusdge");
  //Serial.println(fudge);
  if (fudge >= maxVal){
   fudge = maxVal;
  } 
  //Serial.print("fusdge");
  //Serial.println(fudge);
  //scale the reading to the 255 of the LED PWM
  if (reading > fudge){
     LED = (int) (reading - fudge) / (maxVal-fudge) *255 ;
  }
  else {
    //Serial.println("LED 0");
    LED=0;
  }
  
  setLEDColour(LED);
  return LED;
  
}

//send a BLE message only if the band is being stretched, send one zero and then hold if not
void sendLiveMessage(float reading, int LED){
  Serial.println("senlive");
  Serial.print("reading");
  Serial.println(reading);
  Serial.print("led");
  Serial.println(LED);
  if (LED >0){
    Serial.println("sending live");
    RFduinoBLE.sendFloat(reading);
    //RFduinoBLE.sendInt(LED);
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

void resetParams(){
  
 //set some initial values before we can take some readings
 minVal =500;
 maxVal =510;

//tracking state
 firstRun = true; //first loop 
  
  
}
