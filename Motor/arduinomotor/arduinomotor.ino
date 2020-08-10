#include "arduino_motor.h"

Arduino_Motor *_motor;

void setup() {
  // Start serial monitor
  Serial.begin(115200);

  // Create UDP instance
  _motor = new Arduino_Motor();
}

void loop() {

  // Test motor
  
}
