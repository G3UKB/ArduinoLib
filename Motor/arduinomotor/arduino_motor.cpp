/*
  arduino_motor.cpp - Library for managing a DC motor with encoder
*/

#include "arduino_motor.h"

// ==============================================================
// PUBLIC

// Constants
const int PLUS = 0;
const int MINUS = 1;
const int TYPE_M_SW = 0;
const int TYPE_OPT = 1;

// Constructor
Arduino_Motor::Arduino_Motor(void (*func)(int position), int dir, int pwm, int sensor, int limit_fwd, int limit_rev, int span) {
  
  // We have micro-switches for forward and reverse
  __type = TYPE_M_SW;
  
  // Pin allocations
  __direction = dir;
  __pwm = pwm;
  __sensor = sensor;
  __limit_fwd = limit_fwd;
  __limit_rev = limit_rev;
  __span = span;
  __event_func = func;

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

Arduino_Motor::Arduino_Motor(void (*func)(int position), int dir, int pwm, int sensor, int limit_fwd_rev, int span) {
  
  // We have one optical switch for forward and reverse
  __type = TYPE_OPT;
  
  // Pin allocations
  __direction = dir;
  __pwm = pwm;
  __sensor = sensor;
  __limit_fwd_rev = limit_fwd_rev;
  // Set both to same pin
  __limit_fwd = __limit_fwd_rev;
  __limit_rev = __limit_fwd_rev;
  __span = span;
  __event_func = func;

  // Initialise pins
  pinMode(__direction, OUTPUT);
  pinMode(__pwm, OUTPUT);
  pinMode(__limit_fwd_rev, INPUT);
  pinMode(__sensor, INPUT);
  // Enable internal pullups on switches
  digitalWrite(__limit_fwd_rev, HIGH);
  digitalWrite(__sensor, HIGH);

  // Init vars
  __calibrated = false;
  // Default speeds
  __speed = 100;
  __backoff_speed = 100;
}

// ------------------------------------
// Set speed
 void Arduino_Motor::set_speed(int duty_cycle) {
    __speed = (int)(((float)duty_cycle/100.0) * 255);
 }

 void Arduino_Motor::set_backoff_speed(int duty_cycle) {
    __backoff_speed = (int)(((float)duty_cycle/100.0) * 255);;
 }

// ------------------------------------
// Set calibration
 void Arduino_Motor::set_cal(int num_pulses) {
  __num_pulses = num_pulses;
  __pulse_cnt = 0;
  __calibrated = true;
  __degrees = 0;
 }
 
// ------------------------------------
// Calibrate the motor
// Count number of pulses between limits
 int Arduino_Motor::calibrate() {
  volatile int num_pulses = 0;
  // This leaves the motor at 'home' which is 0 deg (fully reversed)
  // ready for forward 0-span deg.
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
  __pulses_per_degree = ((float)__num_pulses/(float)__span);
  
  Serial.print("Num pulses: ");
  Serial.println(__num_pulses);
  Serial.print("Pulses per degree: ");
  Serial.println(__pulses_per_degree);
  __pulse_cnt = 0;
  __calibrated = true;
  __degrees = 0;
  
  return __num_pulses;
 }

// ------------------------------------
// Move to home position
bool Arduino_Motor::move_to_home() {
  //----------------
  if(__calibrated) {
    // Run reverse at moderate speed until we hit reverse limit switch
    __reverse(__speed);
    __wait_rev_limit();
    __stop();
    delay(500);
    // Back off until reverse switch just releases
    __forward(__backoff_speed);
    __wait_not_rev_limit();
    __stop();
    delay(500);
    __degrees = 0;
    return true;
  }
  return false;
}

// ------------------------------------
// Move to given position
bool Arduino_Motor::move_to_position(int deg) {
  // Local context
  volatile int current_degrees;
  volatile int degrees_to_move;
  volatile int direction_to_move;
  volatile int num_pulses;
  volatile int pulses_to_move;
  volatile float pulses_per_degree;

  //----------------
  if(__calibrated) {
    // Have calibration
    // Parameter check and assign locals
    current_degrees = __degrees;
    pulses_per_degree = __pulses_per_degree;
    if (deg < 0 or deg > __span){
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
    num_pulses = pulses_to_move;
    //Serial.println(degrees_to_move);
    //Serial.println(pulses_to_move);
    if (direction_to_move == PLUS) {
      __forward(__speed);
      while(__test_not_fwd_limit()) {
        if(__read_sensor()) {
            pulses_to_move--;
            __do_event(current_degrees, deg, num_pulses, pulses_to_move);
            if (pulses_to_move <= 0)
              break;
        }
      }
    } else {
      __reverse(__speed);
      while(__test_not_rev_limit()) {
        if(__read_sensor()) {
            pulses_to_move--;
            __do_event(current_degrees, deg, num_pulses, pulses_to_move);
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
// Run forward at given speed
void Arduino_Motor::__forward(int fwd_speed) {
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
  // Wait for next pulse
  if (!digitalRead(__sensor)) {
    while (!digitalRead(__sensor)) {
    }
  }
  // Wait for end of pulse
  while (digitalRead(__sensor)) {
  }
    return true;
}

// ------------------------------------
// Calculate current degrees and dispatch status event
void Arduino_Motor::__do_event(int current_degrees, int deg, int num_pulses, int pulses_to_move) {
  int idegrees;
  
  if (deg > current_degrees) {
    // Moving forward
    idegrees = current_degrees + ((int)((float)(num_pulses - pulses_to_move) / __pulses_per_degree));
  } else {
    idegrees = current_degrees - ((int)((float)(num_pulses - pulses_to_move) / __pulses_per_degree));
  }
  __event_func(idegrees);
}
