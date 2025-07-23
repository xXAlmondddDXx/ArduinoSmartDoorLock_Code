// This source file contains the actual code for the Door Status Detection program.
// HC-SR04 Ultrasonic Sensor
// Adjust the distance and maximum allowance time according to your settings.

#ifndef DOOR_STATUS_H
#define DOOR_STATUS_H

#include <LiquidCrystal_I2C.h>

#define TRIG_PIN  6
#define ECHO_PIN  7

extern SoftwareSerial btSerial;
extern bool buzzerActive;
extern bool shutDoorAlert;
extern void logEvent(String event);

unsigned long doorOpenTime = 0;
static const unsigned int doorOpenInterval = 10000; // 30 secs, adjust your max allowed duration here
long duration;
float distance;

bool doorOpenAlert = false;
bool doorAlertMsgPrinted = false;
static bool displayRestore = true;

void initDoorSensor() {
  pinMode(TRIG_PIN, OUTPUT);  // Trigger pin transmit soundwave
  pinMode(ECHO_PIN, INPUT);   // Echo pin receive soundwave
  Serial.println(F("Door Status Detection: activated!"));
}

float readDoorDistance() {
  // --- Trigger Point sends out 8 cycle sonic burst ---
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  // --- Echo Point receives bounce back pulses and convert into distance ---
  duration = pulseIn(ECHO_PIN, HIGH);
  distance = duration * 0.034 / 2; // Sp of sound in cm by microseconds (340000/1e6)
  return distance;
}

void checkDoorStatus(LiquidCrystal_I2C &lcd) {
  readDoorDistance(); // Call for distance measure function
  if (distance > 10) {
    if ((millis() - doorOpenTime > doorOpenInterval) && !buzzerActive && !shutDoorAlert) {
      doorOpenAlert = true;
      if (!doorAlertMsgPrinted) {
        Serial.println(F("\nDoor left open!"));
        btSerial.println(F("Alarm Triggered: Door left open!"));

        lcd.clear();
        lcd.setCursor(0,0); lcd.print(F("Please close"));
        lcd.setCursor(0,1); lcd.print(F("the door!"));
        logEvent("Door left open too long");
        displayRestore = false;
        doorAlertMsgPrinted = true;
      }
    }
  } else {
    doorOpenTime = millis();
    doorOpenAlert = false;
    doorAlertMsgPrinted = false;

    if (!displayRestore) {
      lcdDisplay();
      displayRestore = true;
    }
  }
}

#endif
