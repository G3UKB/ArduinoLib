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
Arduino_Motor::Arduino_Motor(int t, void (*func)(int position), int dir, int pwm, int sensor, int limit_fwd, int limit_rev, int span) {
  
  // Set type
  __type = t;
  
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
  __abort = false;
}

Arduino_Motor::Arduino_Motor(int t, void (*func)(int position), int dir, int pwm, int sensor, int limit_fwd_rev, int span) {
  
  // Set type
  __type = t;
  
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
  __abort = false;
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
  __pulses_per_degree = ((float)__num_pulses/(float)__span);
  __pulse_cnt = 0;
  __calibrated = true;
  __degrees = 0;
 }

// ------------------------------------
// Calibrate the motor
// Count number of pulses between limits
 int Arduino_Motor::calibrate() {

  int cal, cal_fwd, cal_rev;

  // We do two runs, counting forward and counting reverse
  // These are usually different by maybe 70 pulses on a full 360.
  // We take the average of the runs and use as the count.
  cal_fwd = calibrate_fwd();
  if (cal_fwd == -1) return -1;
  delay(500);
  cal_rev = calibrate_rev();
  if (cal_rev == -1) return -1;
  
  // Use average
  cal = (cal_fwd + cal_rev)/2;
  __pulse_cnt = 0;
  __calibrated = true;
  __degrees = 0;
  __num_pulses = cal;
  __pulses_per_degree = ((float)__num_pulses/(float)__span);
  __event_func(0);
  Serial.print("Pulses: fwd, rev, final, per-degree");
  Serial.println(cal_fwd);
  Serial.println(cal_rev);
  Serial.println(cal);
  Serial.println(__pulses_per_degree);
  return cal;
 }

 // ------------------------------------
// Calibrate the motor in the forward direction
// Count number of pulses between limits
 int Arduino_Motor::calibrate_fwd() {
  volatile int num_pulses = 0;
  // This leaves the motor at 'max' which is 360/90 deg (fully forward)
  // We want the normal travel to avoid the limit switches as they cause over-travel
  // so we count between just released limit switches

  //----------------
  // Run reverse at moderate speed until we hit reverse limit switch
  __reverse(__speed);
  if (!__wait_rev_limit()) {
    __stop();
    return -1;
  }
  __stop();
  delay(500);
  // Back off until reverse switch just releases
  __forward(__backoff_speed);
  if (!__wait_not_rev_limit()) {
    __stop();
    return -1;
  }
  __stop();
  delay(500);

  //----------------
  // Now start counting
  // Reset counters
  __pulse_cnt = 0;
  // Run forward at slowish speed until we hit forward limit switch
  __forward(__speed);
  // Wait for reverse limit switch and accumulate pulse count
  while(__test_not_fwd_limit()) {
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

  //Serial.print("Initial num fwd pulses: ");
  //Serial.println(num_pulses);
  
  //----------------
  // Now back off until forward switch releases
  __reverse(__backoff_speed);
  // Wait for reverse limit switch off and accumulate pulse count
  while(__test_fwd_limit()) {
    if(__read_sensor()) {
      __pulse_cnt++;
    }
  }
  __stop();

  //----------------
  // Subtract the number of pulses we backed off by
  // Return total number of pulses between just released limits
  return num_pulses - __pulse_cnt;
 }
 
// ------------------------------------
// Calibrate the motor in the reverse directiom
// Count number of pulses between limits
 int Arduino_Motor::calibrate_rev() {
  volatile int num_pulses = 0;
  // This leaves the motor at 'home' which is 0 deg (fully reversed)
  // ready for forward 0-span deg.
  // We want the normal travel to avoid the limit switches as they cause over-travel
  // so we count between just released limit switches

  //----------------
  // Run forward at moderate speed until we hit forward limit switch
  __forward(__speed);
  if (!__wait_fwd_limit()) {
    __stop();
    return -1;
  }
  __stop();
  delay(500);
  // Back off until forward switch just releases
  __reverse(__backoff_speed);
  if (!__wait_not_fwd_limit()) {
    __stop();
    return -1;
  }
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

  //Serial.print("Initial num rev pulses: ");
  //Serial.println(num_pulses);
  
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
  
  //----------------
  // Subtract the number of pulses we backed off by
  // Return total number of pulses between just released limits
  return  num_pulses - __pulse_cnt;
 }
 
// ------------------------------------
// Move to home position
bool Arduino_Motor::move_to_home() {
  if(__calibrated) {
    // Run reverse at moderate speed until we hit reverse limit switch
    __reverse(__speed);
    if (!__wait_rev_limit()) {
      __stop();
      return false;
    }
    __stop();
    delay(500);
    // Back off until reverse switch just releases
    __forward(__backoff_speed);
    if (!__wait_not_rev_limit()) {
      __stop();
      return false;
    }
    __stop();
    delay(500);
    __degrees = 0;
    __event_func(0);
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
    //Serial.println("Deg, Pulses, Direction, Fwd, Rev");
    //Serial.println(degrees_to_move);
    //Serial.println(pulses_to_move);
    //Serial.println(direction_to_move);
    //Serial.println(__test_not_fwd_limit());
    //Serial.println(__test_not_rev_limit());
    if (direction_to_move == PLUS) {
      __forward(__speed);
      while(__test_not_fwd_limit()) {
        if(__read_sensor()) {
            pulses_to_move--;
            //Serial.print("Before fwd: ");
            //Serial.println(pulses_to_move);
            __do_event(current_degrees, deg, num_pulses, pulses_to_move);
            //Serial.print("After fwd: ");
            //Serial.println(pulses_to_move);
            if (pulses_to_move <= 0)
              break;
        }
      }
    } else {
      __reverse(__speed);
      while(__test_not_rev_limit()) {
        if(__read_sensor()) {
            pulses_to_move--;
            //Serial.print("Before rev: ");
            //Serial.println(pulses_to_move);
            __do_event(current_degrees, deg, num_pulses, pulses_to_move);
            //Serial.print("After rev: ");
            //Serial.println(pulses_to_move);
            if (pulses_to_move <= 0)
              break;
        }
      }
    }
    __stop();
    //----------------
    // Set new position 
    __degrees = deg;

    // Although the calibration should be between points that are clear of
    // the limits we could have ended up with the limit activated. We must move away
    // we either won't move or will end up rotating twice in the same direction!
    // This should only occur if using a single limit switch for both directions.
    //Serial.println(direction_to_move);
    //Serial.println(__test_fwd_limit());
    if ((direction_to_move == PLUS) && (__test_fwd_limit())) {
      Serial.println("Nudge reverse");
      // Move reverse a little to clear the switch
      __reverse(__speed);
      __wait_not_fwd_limit();
      __stop();
      } else if ((direction_to_move == MINUS) && (__test_rev_limit())) {
        Serial.println("Nudge forward");
        // Move forward a little to clear the switch
        __forward(__speed);
        __wait_not_rev_limit();
        __stop();
    }
  } else {
    // Not calibrated
    return false;
  }
  return true;
}

// ------------------------------------
// Nudge
void Arduino_Motor::nudge_fwd() {
  int count = 10;
  if (__test_fwd_limit()) return;
  __forward(__speed);
  while(__test_not_fwd_limit()) {
     if (count-- <= 0) break;
     delay(10);  
  }
  __stop();
}

void Arduino_Motor::nudge_rev() {
  int count = 10;
  if (__test_rev_limit()) return;
  __reverse(__speed);
  while(__test_not_rev_limit()) {
     if (count-- <= 0) break;
     delay(10);  
  }
  __stop();
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
// In this and all other waits we need a failsafe exit. Its possible due to 
// incorrect wiring or a software error (especially during testing) that we
// are moving the motor the wrong way or waiting for the wrong limit switch
// to close. In this case we will try and destroy the switch or motor.
// To aleviate this we always check for the other limit switch and immediately
// stop the motor.
bool Arduino_Motor::__wait_fwd_limit() {
  int count = 20000; // 20 second timeout
  while (digitalRead(__limit_fwd)) {
    delay (1);
    if (__test_rev_limit()) {
      Serial.println("Detected reverse limit switch waiting for forward limit switch!");
      __stop();
      return false;
    }
    if (__abort) {
      return false;
    }
    --count;
    if (count <= 0) return false;
  }
  return true;
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
bool Arduino_Motor::__wait_rev_limit() {
  int count = 20000; // 20 second timeout
  while (digitalRead(__limit_rev)) {
    delay (1);
    if (__test_fwd_limit()) {
      Serial.println("Detected forward limit switch waiting for reverse limit switch!");
      __stop();
      return false;
    }
    if (__abort) {
      return false;
    }
    --count;
    if (count <= 0) return false;
  }
  return true;
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
bool Arduino_Motor::__wait_not_fwd_limit() {
  int count = 20000; // 20 second timeout
  while (!digitalRead(__limit_fwd)) {
    delay (1);
    if (__test_rev_limit()) {
      Serial.println("Detected reverse limit switch waiting for forward limit switch to release!");
      __stop();
      return false;
    }
    if (__abort) {
      return false;
    }
    --count;
    if (count <= 0) return false;
  }
  return true;
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
bool Arduino_Motor::__wait_not_rev_limit() {
  int count = 20000; // 20 second timeout
  while (!digitalRead(__limit_rev)) {
    delay (1);
    if (__test_fwd_limit()) {
      Serial.println("Detected forward limit switch waiting for reverse limit switch to release!");
      __stop();
      return false;
    }
    if (__abort) {
      return false;
    }
    --count;
    if (count <= 0) return false;
  }
  return true;
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

  //Serial.println("Current deg, deg, num pulses, pulses to move");
  //Serial.println(current_degrees);
  //Serial.println(deg);
  //Serial.println(num_pulses);
  //Serial.println(pulses_to_move);
  
  if (deg > current_degrees) {
    // Moving forward
    idegrees = current_degrees + ((int)((float)(num_pulses - pulses_to_move) / __pulses_per_degree));
  } else {
    idegrees = current_degrees - ((int)((float)(num_pulses - pulses_to_move) / __pulses_per_degree));
  }
  __event_func(idegrees);
}
