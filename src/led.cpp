#define BLINK_PERIOD 500000
#ifndef LED_BUILTIN
#define LED_BUILTIN 2  // for esp32 dev module
#endif 
void setupLED(){
    pinMode(LED_BUILTIN, OUTPUT);

}
void setLED(bool on){
    static bool state = false;
    if(on==state){
        return;
    }
    digitalWrite(LED_BUILTIN,on ? HIGH : LOW);
    state = on ;

}
void blinkLED(){
    setLED(micros() / BLINK_PERIOD % 2);
}