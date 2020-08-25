#include "arduino_motor.h"

Arduino_Motor *__motor_az;
Arduino_Motor *__motor_el;

void setup() {
  // Start serial monitor
  Serial.begin(115200);

  
  // Create Azimuth motor instance
  // Arduino_Motor(int dir, int pwm, int sensor, int limit_fwd_rev, int span)
  __motor_az = new Arduino_Motor(22,4,30,24,360);
 
  // Create Elevation motor instance 
  // Arduino_Motor(int dir, int pwm, int sensor, int limit_fwd, int limit_rev, int span)
  __motor_el = new Arduino_Motor(23,5,31,25,26,90);

}

void loop() {

  // Test motor
  
  Serial.println("Setting motor speed to 40% at 12V");
  __motor_az->set_speed(40);
  __motor_az->set_backoff_speed(40);
  __motor_el->set_speed(40);
  __motor_el->set_backoff_speed(40);

  Serial.println("Calibrating azimuth motor...");
  __motor_az->calibrate();
  delay(1000);
  
  Serial.println("Calibrating elevation motor...");
  __motor_el->calibrate();
  delay(1000);
  
  Serial.println("Position azimuth to 180 degrees...");
  __motor_az->move_to_position(180);
  delay(1000);

  Serial.println("Position elevation to 45 degrees...");
  __motor_el->move_to_position(45);
  delay(1000);
  Serial.println("Position elevation to 90 degrees...");
  __motor_el->move_to_position(90);
  delay(1000);
  
  Serial.println("Move azimuth to home position...");
  __motor_az->move_to_home();
  delay(1000);

  Serial.println("Move elevation to home position...");
  __motor_el->move_to_home();
  
  delay(5000);
}
