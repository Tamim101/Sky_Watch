void setup() {
	Serial.begin(SERIAL_BAUDRATE);
	print("Initializing flix\n");
	
	setupMotors();
}
