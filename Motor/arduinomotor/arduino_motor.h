/*
  arduino_motor.h - Library for managing a DC motor with encoder
*/

#ifndef arduino_motor_h
#define arduino_motor_h

#include "Arduino.h"

class Arduino_Motor
{
  public:
    Arduino_Motor(int dir, int pwm, int sensor, int limit_fwd, int limit_rev);

	// Public method prototypes
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
