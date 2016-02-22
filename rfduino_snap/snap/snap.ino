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
bool serial=false; //serial comms
int radioInactiveTimeOut = 10000; // 60 secodns timeout
char patternBLEToMatch[5] = "BBAA"; // button press pattern that needs to be matched before BLE turned on 
                                    // Should end with "A"
int lightFlash = 25; // number ms the light flashes when clicked

//state
bool radioState = false;
bool communicating=false;
int lastUsedBLE = millis();
char patternBLE[5] = "WXYZ";
bool send_history = false;
bool battery=false;


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
  //digitalWrite(4, HIGH);  //blue
  //delay(lightFlash);
  //digitalWrite(4, LOW);


  bool buttonA=true;
  while (buttonA){
    if( digitalRead(5) ==HIGH ){
      //Serial.print("button on");
      digitalWrite(4, HIGH);  //blue
      
    }
    else {      
      Serial.print("button off");
      digitalWrite(4, LOW);
      buttonA=false;

    }
    //Serial.print("INFINITE DELAY");
    //delay(2000)  ;
  }



  //save the usage
  if(save){
    saveUsage( millis() );
  }

  Serial.print("check pattern");
  //check if the pattern is matched
  if (checkPattern('A')){
    Serial.print("pattern matched");
    //if the radio is not alreay on then turn it on
    if (!radioState){      
      Serial.print("start adv");
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
  /*digitalWrite(3, HIGH);  //green
  delay(lightFlash);
  digitalWrite(3, LOW);
  
  //save the usage
  if(save){
    saveUsage( millis() );
  }*/


/*//digitalWrite(3, HIGH);  //green
  analogWrite(3, ledBrightness); 
  delay(lightFlash);
  digitalWrite(3, LOW);
  
  //save the usage
  if(save){
    saveUsage( millis() );
  }*/


 //save the usage
  if(save){
    saveUsage( millis() );
  }
  bool buttonB=true;
  while (buttonB){
    if( digitalRead(6) ==HIGH ){
      //Serial.print("button on");
      digitalWrite(3, HIGH);  //green
      
    }
    else {      
      Serial.print("button off");
      digitalWrite(3, LOW);
      buttonB=false;

    }
    //Serial.print("INFINITE DELAY");
    //delay(2000)  ;
  }

  //save the usage
  if(save){
    saveUsage( millis() );
  }



  
  //log the pattern
  checkPattern('B');
  return 0;  // don't exit RFduino_ULPDelay
}

void RFduinoBLE_onAdvertisement(bool start){}

void RFduinoBLE_onConnect(){
  Serial.println("BLE connected");
}

void RFduinoBLE_onDisconnect(){
  Serial.println("BLE disconnected");
}

void RFduinoBLE_onReceive(char *data, int len){
  Serial.println("BLE message recieved");
  Serial.print("flag recieved, value:");
  Serial.println(String(data[0]));
  readFlag(data[0]);
}

void readFlag(int flag){

  switch(flag){
    case 0:
      //set Snapino's mode to PASSIVE
      Serial.println(" legacy mode passive");
      sendMessage("Mode passive");
      break;
    case 1:
      //set Snapino's mode to ACTIVE
        Serial.println(" legacy mode active");
      break;
    case 2:      
      //then send historic values as pairs
      if(!send_history){
        Serial.println("  history");
        RFduino_ULPDelay(1000); //take a second break so we don't confuse the phone between mode
        send_history = true;
      }
      break;
    case 3:
      if (!battery){
        battery = true;
      }
      break;
    default:
      //why are we sending unaccounted flags?!?
      Serial.print("Doh...do something else! unaccounted flag");
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


//http://www.arduino-hacks.com/float-to-string-float-to-character-array-arduino/
//function to extract decimal part of float used by battery level
long getDecimal(float val)
{
 int intPart = int(val);
 long decPart = 1000*(val-intPart); //I am multiplying by 1000 assuming that the foat values will have a maximum of 3 decimal places
                                   //Change to match the number of decimal places you need
 if(decPart>0)return(decPart);           //return the decimal part of float number if it is available 
 else if(decPart<0)return((-1)*decPart); //if negative, multiply by -1
 else if(decPart=0)return(00);           //return 0 if decimal part of float number is not available
}

//send battery level
void sendBattery(){
  analogReference(VBG); // Sets the Reference to 1.2V band gap           
  analogSelection(VDD_1_3_PS);  //Selects VDD with 1/3 prescaling as the analog source
  int sensorValue = analogRead(1); // the pin has no meaning, it uses VDD pin
  float batteryVoltage = sensorValue * (3.6 / 1023.0); // convert value to voltage
  Serial.print("Battery Voltage: ");
  Serial.println(batteryVoltage); 
  
  String stringVal = String(int(batteryVoltage))+ "."+String(getDecimal(batteryVoltage)); 
  sendMessage(stringVal);
}


void serviceFlags(){ 

  if (battery){
    lastUsedBLE = millis();
    sendBattery();
    lastUsedBLE = millis();
    battery=false;
  }
  
  
  if(send_history){
    //log time
    lastUsedBLE = millis();
    sendHistoryBLE();
    //log time
    lastUsedBLE = millis();
  }

  
}

void setup() {
  if (serial) { 
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

  //Serial.print("min flash page");
  //Serial.print(findMinFlashPage(251));

  
}

void loop() {
  //Serial.println(" sleepy ");
//RFduino_ULPDelay(SECONDS(0.5));

  //if radios on and timed out then turn the radio off
  if (radioState && (millis()-lastUsedBLE > radioInactiveTimeOut)){
    Serial.println(" stop advertising ");
    stopAdvertising();
    //most of the time the device will be in this delay

    RFduino_resetPinWake(5); ///THIS
    Serial.println(" radio off sleep ");
    RFduino_ULPDelay(INFINITE);
    Serial.println(" delay, really? Should never see this radio off ");
    RFduino_resetPinWake(5);
  }

  // else service the radio
  else if (radioState){
    //ELSE WE CAN SERVICE THE RADIO FLAGS
    //Serial.println(" servicing radio flags ");
    serviceFlags();
    RFduino_resetPinWake(5);
    Serial.println(" sleepy service radio ");
    RFduino_ULPDelay(SECONDS(0.5));
  }
  //this is only used the first time the device is on
  else
  {
    RFduino_resetPinWake(5); // when the loop is broken by the correct pattern, need to cancle the wake
    Serial.println(" else loop bit only used on first turn on ");
    RFduino_ULPDelay(INFINITE);
    Serial.println(" delay, really? Should never see this else loop");
    RFduino_resetPinWake(5); // when the loop is broken by the correct pattern, need to cancle the wake
  }
  //Serial.print("main loop");
    //    RFduino_ULPDelay(INFINITE);
    //Serial.println(" delay, really? Should never see this  main loop");
}

