#include <Arduino.h>
#include "until.h"
float motors[4]; 

void setup() {
	Serial.begin(SERIAL_BAUDRATE);
	print("Initializing flix\n");
	
	setupMotors();
}
void loop(){
  
}