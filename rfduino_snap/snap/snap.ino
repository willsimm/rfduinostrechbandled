#include <RFduinoBLE.h>
#include<stdlib.h>

bool serial=true;







int buttonACallback(uint32_t ulPin)
{
  digitalWrite(4, HIGH);  //blue
  delay(250);
  digitalWrite(4, LOW);

  //check for advertise pattern?

  return 0;  // don't exit RFduino_ULPDelay
}


int buttonBCallback(uint32_t ulPin)
{
  digitalWrite(3, HIGH);  //green
  delay(250);
  digitalWrite(3, LOW);
  
  return 0;  // don't exit RFduino_ULPDelay
}




void setup() {
  if (Serial) { 
    Serial.begin(9600);
  }
  
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
  RFduino_ULPDelay(INFINITE);
  
}
