/*
snap clicker
Show on LED
record history of usage to flash
read out history on request

myclasp.org.uk

Will Simm w.simm@lancs.ac.uk & Adrian Gradinar 2015

*/


#include <RFduinoBLE.h>
#include<stdlib.h>


//CONFIG
//These vars are used to configure the device 
char nameDevice[] = "SnapClicker";
bool save = true; //enable saving of usage times to memory / flash
bool serial=true; //serial comms
int radioInactiveTimeOut = 10000; // 60 secodns timeout
char patternBLEToMatch[5] = "BBAA"; // button press pattern that needs to be matched before BLE turned on 
                                    // Should end with "A"
int lightFlash = 25; // number ms the light flashes when clicked

//state
bool radioState = false;
bool communicating=false;
int lastUsedBLE = millis();
char patternBLE[5] = "WXYZ";


bool checkPattern(char button){
  patternBLE[0] = patternBLE[1];
  patternBLE[1] = patternBLE[2];
  patternBLE[2] = patternBLE[3];
  patternBLE[3] = button;
  Serial.println(patternBLE);
  if (strcmp(patternBLE, patternBLEToMatch)  == 0) 
  {
    return true;
  }

  return false;
}

//turn  on the radio and start advertising
void startAdvertising(){  
  if (!radioState){
    Serial.println(" turn radio on and start advertising ");
    RFduinoBLE.begin();
    radioState = true;
    //log time
    lastUsedBLE = millis();
    Serial.println(" ble begin ");
  }
  
}

// turn that radio off
void stopAdvertising(){
  if (radioState){
    Serial.println(" stop advertising and turn off radio");
    RFduinoBLE.end();
    radioState=false;
  }
}


//callback for button A press
int buttonACallback(uint32_t ulPin)
{
  digitalWrite(4, HIGH);  //blue
  delay(lightFlash);
  digitalWrite(4, LOW);

  //save the usage
  if(save){
    //saveUsage( millis() );
  }

  //check if the pattern is matched
  if (checkPattern('A')){
    //if the radio is not alreay on then turn it on
    if (!radioState){      
      startAdvertising();
      //return 1 so we break the infinite loop
      return 1;
    }
  }
  //drop back to the infinite loop
  return 0; 
}

// callback for button b poress
int buttonBCallback(uint32_t ulPin)
{
  digitalWrite(3, HIGH);  //green
  delay(lightFlash);
  digitalWrite(3, LOW);
  
  //save the usage
  if(save){
    //saveUsage( millis() );
  }
  //log the pattern
  checkPattern('B');
  return 0;  // don't exit RFduino_ULPDelay
}

void RFduinoBLE_onAdvertisement(bool start){}


void setup() {
  if (Serial) { 
    Serial.begin(9600);
  }
  RFduinoBLE.deviceName = nameDevice;
  
  //LED light outputs
  pinMode(2, OUTPUT); //red
  pinMode(3, OUTPUT); //green
  pinMode(4, OUTPUT);  //blue
  
  // callbacks for button presses
  pinMode(5, INPUT);  //   buttonA
  RFduino_pinWakeCallback(5, HIGH, buttonACallback);
  pinMode(6, INPUT);  //  buttonB
  RFduino_pinWakeCallback(6, HIGH, buttonBCallback);

  
}

void loop() {

  //if radios on and timed out then turn the radio off
  if (radioState && (millis()-lastUsedBLE > radioInactiveTimeOut)){
    Serial.println(" stop advertising ");
    stopAdvertising();
    //usually the device wil be in this delay
    RFduino_ULPDelay(INFINITE);
    RFduino_resetPinWake(5);
  }

  // else service the radio
  else if (radioState){
    //ELSE WE CAN SERVICE THE RADIO FLAGS
    Serial.println(" servicing radio flags ");
    RFduino_resetPinWake(5);
    RFduino_ULPDelay(SECONDS(0.5));
  }
  //this is only used the first time the device is on
  else
  {
    Serial.println(" else loop bit only used on first turn on ");
    RFduino_ULPDelay(INFINITE);
    RFduino_resetPinWake(5);
  }
    
}

