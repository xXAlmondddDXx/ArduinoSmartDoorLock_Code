//Bluetooth Header file (/w the extension .h)
//The header file list the Bluetooth function declarations here in a class.

#ifndef BLUETOOTH_CONTROL_H
#define BLUETOOTH_CONTROL_H

#include <Arduino.h>

// --- Declare global functions & variables ---
extern void unlockDoor();

class BluetoothControl {
  public:
  void begin();
  void checkCommand();
  static bool reqTempAccess();
  static bool tempAccess;

  private:
  String readString;
};

#endif
