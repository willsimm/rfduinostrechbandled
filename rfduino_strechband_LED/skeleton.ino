#include <RFduinoBLE.h>

int live_stream_value = 0;
bool mode_active = false;

int connection_led = 2;
int data_was_requesed_led = 3;
//enum modes {ACTIVE, PASSIVE};

void setup() {
  Serial.begin(9600);
  RFduinoBLE.deviceName = "Snapino";

  pinMode(connection_led, OUTPUT);
  pinMode(data_was_requesed_led, OUTPUT);
  
  RFduinoBLE.begin();
}

void loop() {
  if(mode_active){
    transmitLiveStream();
  }
}

void RFduinoBLE_onAdvertisement(bool start){}

void RFduinoBLE_onConnect(){
  turnOnLed(connection_led);
}

void RFduinoBLE_onDisconnect(){
  turnOffLed(connection_led);
  mode_active = false;
}

void RFduinoBLE_onReceive(char *data, int len){
  turnOffLed(connection_led);
  turnOnLed(data_was_requesed_led);
  readFlag(data[0]);
}

void turnOffLed(int led){
  digitalWrite(led, LOW);
}

void turnOnLed(int led){
  digitalWrite(led, HIGH);
}

void readFlag(int flag){

  switch(flag){
    case 0:
      //set Snapino's mode to PASSIVE
      mode_active = false;
      sendMessage("Mode passive");
      break;
    case 1:
      //set Snapino's mode to ACTIVE
      sendMessage("Mode active");
      //send float readings 
      
      
//      mode_active = true;
      break;
    case 2:
      //active time was requested - NOW I tell thee!!!
      sendMessage(""+10000);
      
      //ends live data streaming
      //send current time twice as a pair
      //then send historic values as pairs
      //send a pair of zeros to finish
      
      sendHistory();
  
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
  RFduinoBLE.send(dataToSend, len-1);
}

void transmitLiveStream(){
  RFduinoBLE.sendInt(live_stream_value++);
}





void sendHistory(){
   String toSend ="";   
   toSend = String(-1) +  "/" + String( millis() );      
   sendMessage(toSend);  
   for (int c=0; c<500; c++){    
     toSend = String(random(1000,50000)) +  "/" + String( random(1000,50000) );      
   } 
   toSend = String(-1) +  "/" + String( millis() );  
}
