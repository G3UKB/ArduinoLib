#include "arduino_motor.h"

Arduino_Motor *__motor;

void setup() {
  // Start serial monitor
  Serial.begin(115200);

  // Create UDP instance
  // Arduino_Motor(int dir, int pwm, int sensor, int limit_fwd, int limit_rev)
  __motor = new Arduino_Motor(22,4,30,24,25);
}

void loop() {

  // Test motor
  __motor->calibrate();
  delay(3000);
  __motor->move_to_position(180);
  delay(3000);
  __motor->move_to_home();

  delay(5000);
}
