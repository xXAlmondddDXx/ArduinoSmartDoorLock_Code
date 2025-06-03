//RFID Header file (/w the extension .h)
//The header file list the RFID function declarations here in a class.
//Replace the RFID UID for the validTag inside the "".

#ifndef RFIDDoorLock_H
#define RFIDDoorLock_H

#include <Arduino.h>
#include <MFRC522.h>
#include <SPI.h>

#define RST_PIN 9
#define SS_PIN  10

// --- Declare global functions & variables ---
extern bool buzzerActive;
extern void unlockDoor();

class RFIDDoorLock {
  public:
  void begin();
  void readRFID();
  void checkTempAccess();
  
  private:
  static byte scanCount;
  const String validTag = " 73 47 1D 95";  // The UID is case senstivie and space sensitive
  const String tempTag  = " 23 80 85 90";
};

#endif
