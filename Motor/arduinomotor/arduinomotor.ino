#include "arduino_motor.h"

Arduino_Motor *__motor;

void setup() {
  // Start serial monitor
  Serial.begin(115200);

  // Create UDP instance
  // Arduino_Motor(int dir, int pwm, int sensor, int limit_fwd, int limit_rev)
  //__motor = new Arduino_Motor(22,4,30,24,25);
  __motor = new Arduino_Motor(22,4,30,24);
}

void loop() {

  // Test motor
  Serial.println("Calibrate");
  __motor->calibrate();
  delay(2000);
  Serial.println("Position 90");
  __motor->move_to_position(90);
  delay(2000);
  Serial.println("Position 180");
  __motor->move_to_position(180);
  delay(2000);
  Serial.println("Position 270");
  __motor->move_to_position(270);
  delay(2000);
  Serial.println("Home");
  __motor->move_to_home();

  delay(5000);
  
}
