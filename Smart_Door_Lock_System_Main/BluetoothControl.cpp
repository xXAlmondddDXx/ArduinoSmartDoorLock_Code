//Bluetooth Source file (w/ extension .cpp)
//The source file contains the actual code for the Bluetooth module program.
//Replace your door access command under [if readString == ""].

#include "BluetoothControl.h"

bool BluetoothControl::tempAccess = false;
bool shutDoorAlert = false;

void BluetoothControl::begin() {
  Serial.begin(9600);
  Serial.println(F("Bluetooth ready!"));
}

void BluetoothControl::checkCommand() {
  while(Serial.available()){    //Check for available byte to read
    delay (10);                 //Delay to stabilise the reading
    char c = Serial.read();     //Conduct a serial read
    if(c == '#') {
      break;                    //Stop reading when '#' is encountered
    }
    readString += c;            //Appends c, (readString = readString + c)
  }

  if(readString.length() > 0) {
    readString.toLowerCase();
    Serial.println("\nReceived: " + readString); // Feedback to serial monitor

    if (readString == "open sesame") {
      Serial.println(F("Manually unlock door"));
      Serial.println(F("Door open alert shutdown"));
      shutDoorAlert = true;
      unlockDoor();
      
    } else if (readString == "close sesame") {
      Serial.println(F("Door open alert restart!"));
      shutDoorAlert = false;

    } else if (readString == "temporary access") {
      Serial.println(F("Visitor portal open"));
      tempAccess = true;
      
    } else if (readString == "close temporary access") {
      Serial.println(F("Visitor portal close"));
      tempAccess = false;
      
    } else {
      Serial.println("Invalid command: " + readString);  //Invalid command
    }
    readString = "";  // Clear readString for next command reading
  }
}

bool BluetoothControl::reqTempAccess() {
  return tempAccess;
}
