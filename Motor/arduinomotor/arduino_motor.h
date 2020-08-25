/*
  arduino_motor.h - Library for managing a DC motor with encoder
*/

#ifndef arduino_motor_h
#define arduino_motor_h

#include "Arduino.h"

class Arduino_Motor
{
  public:
    Arduino_Motor(int dir, int pwm, int sensor, int limit_fwd, int limit_rev, int span);
    Arduino_Motor(int dir, int pwm, int sensor, int limit_fwd_rev, int span);

	// Public method prototypes
  void set_speed(int new_speed);
  void set_backoff_speed(int new_speed);
	bool calibrate();
  bool move_to_home();
  bool move_to_position(int deg);
 
  private:
  // Pin allocations
  int __direction;
  int __pwm;
  int __sensor;
  int __limit_fwd;
  int __limit_rev;
  int __limit_fwd_rev;
  int __span;
  
  // Speed
  int __speed;
  int __backoff_speed;

  // Instance vars
  volatile int __type;
  volatile bool __calibrated;
  volatile int __pulse_cnt;
  volatile int __num_pulses;
  volatile int __degrees;
  volatile float __pulses_per_degree;
  
	// Private method prototypes
	void __forward(int fwd_speed);
  void __reverse(int rev_speed);
  void __stop();
  bool __test_fwd_limit();
  bool __test_not_fwd_limit();
  void __wait_fwd_limit();
  void __wait_not_fwd_limit();
  
  bool __test_rev_limit();
  bool __test_not_rev_limit();
  void __wait_rev_limit();
  void __wait_not_rev_limit();

  bool __read_sensor();
  
};

#endif