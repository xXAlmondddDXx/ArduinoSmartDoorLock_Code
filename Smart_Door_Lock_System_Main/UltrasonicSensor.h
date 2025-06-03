//The Ultrasonic Header file (w/ extension .h)
//The header file list the Ulrasonic function declarations here in a class.

#ifndef ULTRASONIC_SENSOR_H
#define ULTRASONIC_SENSOR_H

#include <Arduino.h>

#define trigPin 6
#define echoPin 7

// --- Declare global variable ---
extern bool buzzerActive;
extern bool shutDoorAlert;

class UltrasonicSensor {
  public:
  void begin();
  float getDistance();
  void doorStatus();
  
  static const unsigned long doorOpenAllowance;
  static unsigned long doorOpenTime;
  static bool doorOpenAlert;
  static bool doorAlertMsgPrinted;

  private:
  float duration;
  float distance;
};

#endif
