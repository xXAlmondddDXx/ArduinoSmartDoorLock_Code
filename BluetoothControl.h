// This source file contains the actual code for the Bluetooth module program.
// HC-05 Bluetooth Module
// Modify the Bluetooth command according to your settings.

#ifndef BLUETOOTH_CONTROL_H
#define BLUETOOTH_CONTROL_H

#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>

SoftwareSerial btSerial (2, 3); // RX, TX

#define buzzerPin A2

extern void unlockDoor();
extern void lcdDisplay();
extern void logEvent(String event);
extern int fingerAttempt;
extern int rfidAttempt;
extern bool buzzerActive;
extern bool doorOpenAlert;
extern bool doorAlertMsgPrinted;
extern unsigned long doorOpenTime;

static bool tempAccess = false;
static bool shutDoorAlert = false;
static bool lcdTransition = false;
static bool intrusionAlert = false;
static unsigned long displayTime = 0;
static unsigned long intrusionAlertTime = 0;

void initBTCtrl() {
  Serial.begin(115200);
  btSerial.begin(9600);
  Serial.println(F("Bluetooth ready"));
}

// --- Replace your command inside "" ---
void btCommands(LiquidCrystal_I2C &lcd) {
  if (btSerial.available()) { // Check for available byte to read
    String cmd = btSerial.readStringUntil('\n');
    cmd.trim(); // Remove any whitespace or newline chars

    if (cmd.equalsIgnoreCase("open sesame")) {
      Serial.println(F("\nManual Unlock"));
      Serial.println(F("Door Status Detection: deactivated!"));
      btSerial.println(F("Manual unlock requested and done"));
      btSerial.println(F("Door Status Detection: deactivated!"));
      
      lcd.clear();
      lcd.setCursor(0,0); lcd.print(F("Manual Unlocking"));
      logEvent("Manually unlock via app | Door Status Detection: deactivated");
      shutDoorAlert = true;
      unlockDoor();

    } else if (cmd.equalsIgnoreCase("close sesame")) {
      Serial.println(F("\nDoor Status Detection: reactivated!"));
      btSerial.println(F("Door Status Detection: reactivated! (Door open time set to 60s threshold)"));

      logEvent("Door Status Detection: reactivated");
      shutDoorAlert = false;
      
    } else if (cmd.equalsIgnoreCase("temporary access")) {
      Serial.println(F("\nVisitor portal opened"));
      btSerial.println(F("Visitor portal opened"));

      lcd.clear();
      lcd.setCursor(0,0); lcd.print(F("Visitor Portal:"));
      lcd.setCursor(0,1); lcd.print(F("Opened"));
      logEvent("Visitor portal open via app");
      lcdTransition = true;
      displayTime = millis();
      tempAccess = true;

    } else if (cmd.equalsIgnoreCase("close temporary access")) {
      Serial.println(F("\nVisitor portal closed"));
      btSerial.println(F("Visitor portal closed"));

      lcd.clear();
      lcd.setCursor(0,0); lcd.print(F("Visitor Portal:"));
      lcd.setCursor(0,1); lcd.print(F("Closed"));
      logEvent("Visitor portal close via Bluetooth");
      lcdTransition = true;
      displayTime = millis();
      tempAccess = false;

    } else if (cmd.equalsIgnoreCase("reset")) {
      Serial.println(F("\nSystem Reset"));
      btSerial.println(F("System Reset!"));

      lcd.clear();
      lcd.setCursor(0,0); lcd.print(F("Reseting"));
      lcd.setCursor(0,1); lcd.print(F("System..."));
      logEvent("System being reset");
      lcdTransition = true;
      displayTime = millis();

      fingerAttempt = 0;
      rfidAttempt = 0;
      tempAccess = false;
      buzzerActive = false;
      doorOpenAlert = false;
      shutDoorAlert = false;
      doorOpenTime = millis();
      doorAlertMsgPrinted = false;

      tone(buzzerPin, 1000); // Alert sound for Reset

    } else if (cmd.equalsIgnoreCase("intrusion")) {
      Serial.println(F("\nSystem Intrusion Alert!"));
      Serial.println(F("Please check your mobile application!"));

      lcd.clear();
      lcd.setCursor(0,0); lcd.print(F("Intrusion Alert!"));
      lcd.setCursor(0,1); lcd.print(F("Please check!"));
      logEvent("System intrusion detected!");
      intrusionAlert = true;
      intrusionAlertTime = millis();
      buzzerActive = true;

    } else {
      Serial.print(F("Invalid command: ")); Serial.println(cmd);
    }
  }
}

bool reqTempAccess() {
  return tempAccess;
}

#endif
