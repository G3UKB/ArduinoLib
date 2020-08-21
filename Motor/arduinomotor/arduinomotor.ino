#include "arduino_motor.h"

Arduino_Motor *__motor_az;
Arduino_Motor *__motor_el;

void setup() {
  // Start serial monitor
  Serial.begin(115200);

  
  // Create Azimuth motor instance
  // Arduino_Motor(int dir, int pwm, int sensor, int limit_fwd_rev)
  __motor_az = new Arduino_Motor(22,4,30,24);
 
  // Create Elevation motor instance 
  // Arduino_Motor(int dir, int pwm, int sensor, int limit_fwd, int limit_rev)
  __motor_el = new Arduino_Motor(23,5,31,25,26);

}

void loop() {

  // Test motor
  Serial.println("Calibrate");
  __motor_az->set_speed(100);
  __motor_az->set_backoff_speed(100);
  __motor_el->set_speed(100);
  __motor_el->set_backoff_speed(100);
  
  //__motor_az->calibrate();
  __motor_el->calibrate();
  /*
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
*/
  delay(5000);
  
}
