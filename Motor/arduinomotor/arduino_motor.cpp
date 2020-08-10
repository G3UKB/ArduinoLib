/*
  arduino_motor.cpp - Library for managing a DC motor with encoder
*/

#include "arduino_motor.h"

// ==============================================================
// PUBLIC

// Constants
const int PLUS = 0;
const int MINUS = 1;

// Constructor
Arduino_Motor::Arduino_Motor(int dir, int pwm, int sensor, int limit_fwd, int limit_rev) {

  // Pin allocations
  __direction = dir;
  __pwm = pwm;
  __sensor = sensor;
  __limit_fwd = limit_fwd;
  __limit_rev = limit_rev;

  // Initialise pins
  pinMode(__direction, OUTPUT);
  pinMode(__pwm, OUTPUT);
  pinMode(__limit_fwd, INPUT);
  pinMode(__limit_rev, INPUT);
  pinMode(__sensor, INPUT);
  // Enable internal pullups on switches
  digitalWrite(__limit_fwd, HIGH);
  digitalWrite(__limit_rev, HIGH);
  digitalWrite(__sensor, HIGH);

  // Init vars
  __calibrated = false;
  // Default speeds
  __speed = 100;
  __backoff_speed = 100;
}

// ------------------------------------
// Set speed
 void Arduino_Motor::set_speed(int new_speed) {
    __speed = new_speed;
 }

 void Arduino_Motor::set_backoff_speed(int new_speed) {
    __backoff_speed = new_speed;
 }
// ------------------------------------
// Calibrate the motor
// Count number of pulses between limits
 bool Arduino_Motor::calibrate() {
  int num_pulses;
  // This leaves the motor at 'home' which is 0 deg (fully reversed)
  // ready for forward 0-359 deg.
  // We want the normal travel to avoid the limit switches as they cause over-travel
  // so we count between just released limit switches

  //----------------
  // Run forward at moderate speed until we hit forward limit switch
  __forward(__speed);
  __wait_fwd_limit();
  __stop();
  delay(500);
  // Back off until forward switch just releases
  __reverse(__backoff_speed);
  __wait_not_fwd_limit();
  __stop();
  delay(500);

  //----------------
  // Now start counting
  // Reset counters
  __pulse_cnt = 0;
  // Run reverse at slowish speed until we hit reverse limit switch
  __reverse(__speed);
  // Wait for reverse limit switch and accumulate pulse count
  while(__test_not_rev_limit()) {
    if(__read_sensor()) {
      __pulse_cnt++;
    }
  }
  __stop();
  delay(500);

  //----------------
  // Remember initial total number of pulses and reset counters
  // This is from just released forward switch to activated reverse switch
  num_pulses = __pulse_cnt;
  __pulse_cnt = 0;

  //----------------
  // Now back off until reverse switch releases
  __forward(__backoff_speed);
  // Wait for reverse limit switch off and accumulate pulse count
  while(__test_rev_limit()) {
    if(__read_sensor()) {
      __pulse_cnt++;
    }
  }
  __stop();
  delay(500);
  
  //----------------
  // Subtract the number of pulses we backed off by
  // Save total number of pulses between just released switches
  __num_pulses = num_pulses - __pulse_cnt;
  __pulses_per_degree = ((float)__num_pulses/360.0);
  
  Serial.print("Num pulses: ");
  Serial.println(__num_pulses);
  __pulse_cnt = 0;
  __calibrated = true;
  
  return true;
 }

// ------------------------------------
// Move to home position
bool Arduino_Motor::move_to_home() {
  //----------------
  if(__calibrated) {
    // Run forward at moderate speed until we hit forward limit switch
    __forward(__speed);
    __wait_fwd_limit();
    __stop();
    delay(500);
    // Back off until forward switch just releases
    __reverse(__backoff_speed);
    __wait_not_fwd_limit();
    __stop();
    delay(500);
  }
  return true;
}

// ------------------------------------
// Move to given position
bool Arduino_Motor::move_to_position(int deg) {
  // Local context
  int current_degrees;
  int degrees_to_move;
  int direction_to_move;
  int pulses_to_move;
  float pulses_per_degree;

  //----------------
  if(__calibrated) {
    // Have calibration
    // Parameter check and assign locals
    current_degrees = __degrees;
    pulses_per_degree = __pulses_per_degree;
    if (deg < 0 or deg > 360){
      return false;
    }

    //----------------
    // Set direction
    if (current_degrees < deg)
      direction_to_move = PLUS;
    else
      direction_to_move = MINUS;

    //----------------
    // Move to new position  
    degrees_to_move = abs(current_degrees - deg);
    pulses_to_move = (int)(pulses_per_degree * (float)degrees_to_move);
    if (direction_to_move == PLUS) {
      __forward(__speed);
      while(__test_not_fwd_limit()) {
        if(__read_sensor()) {
            pulses_to_move--;
            if (pulses_to_move <= 0)
              break;
        }
      }
    } else {
      __reverse(__speed);
      while(__test_not_rev_limit()) {
        if(__read_sensor()) {
            pulses_to_move--;
            if (pulses_to_move <= 0)
              break;
        }
      }
    }
    __stop();
    //----------------
    // Set new position 
    __degrees = deg;
  } else {
    // Not calibrated
    return false;
  }
  return true;
}

// ==============================================================
// PRIVATE

// ------------------------------------
// Run forward at geven speed
void Arduino_Motor::__forward(int fwd_speed) {
  Serial.print("Forward: ");
  Serial.println(fwd_speed);
  digitalWrite(__direction, HIGH);
  analogWrite(__pwm, fwd_speed);
}

// ------------------------------------
// Run reverse at given speed
void Arduino_Motor::__reverse(int rev_speed) {
  digitalWrite(__direction, LOW);
  analogWrite(__pwm, rev_speed);
}

// ------------------------------------
// Stop motor
void Arduino_Motor::__stop() {
  analogWrite(__pwm, 0);
}

// ------------------------------------
// Test forward limit switch
bool Arduino_Motor::__test_fwd_limit() {
  if (!digitalRead(__limit_fwd)) {
    return true;
  }
  return false;
}

// ------------------------------------
// Wait for forward limit switch
void Arduino_Motor::__wait_fwd_limit() {
  while (digitalRead(__limit_fwd)) {
    delay (1);
  }
}

// ------------------------------------
// Test reverse limit switch
bool Arduino_Motor::__test_rev_limit() {
  if (!digitalRead(__limit_rev)) {
    return true;
  }
  return false;
}

// ------------------------------------
// Wait for reverse limit switch
void Arduino_Motor::__wait_rev_limit() {
  while (digitalRead(__limit_rev)) {
    delay (1);
  }
}

// ------------------------------------
// Test not forward limit switch
bool Arduino_Motor::__test_not_fwd_limit() {
  if (digitalRead(__limit_fwd)) {
    return true;
  }
  return false;
}

// ------------------------------------
// Wait for forward limit switch to release
void Arduino_Motor::__wait_not_fwd_limit() {
  while (!digitalRead(__limit_fwd)) {
    delay (1);
  }
}

// ------------------------------------
// Test not reverse limit switch
bool Arduino_Motor::__test_not_rev_limit() {
  if (digitalRead(__limit_rev)) {
    return true;
  }
  return false;
}

// ------------------------------------
// Wait for reverse limit switch to release
void Arduino_Motor::__wait_not_rev_limit() {
  while (!digitalRead(__limit_rev)) {
    delay (1);
  }
}

// ------------------------------------
// Read sensor
bool Arduino_Motor::__read_sensor() {
  if(digitalRead(__sensor)) {
    while (digitalRead(__sensor)) {
      delayMicroseconds(10);
    }
    return true;
  }
  return false;
}
