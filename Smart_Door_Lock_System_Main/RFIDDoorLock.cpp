//RFID Source file (w/ extension .cpp)
//The source file contains the actual code for the RFID module program.

#include "BluetoothControl.h"
#include "RFIDDoorLock.h"

MFRC522 mfrc522(SS_PIN, RST_PIN);

byte RFIDDoorLock::scanCount = 0;

void RFIDDoorLock::begin() {
  Serial.begin(9600);
  SPI.begin();
  mfrc522.PCD_Init();

  if (mfrc522.PCD_PerformSelfTest()) {
    Serial.println(F("RFID ready!"));
  } else {
    Serial.println(F("RFID failed or not responding!"));
  }
}

void RFIDDoorLock::readRFID() {
  // Check if a new card is present OR read the Tag serial no.
  if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial()) {
    delay(25);
    return;
  }

  // The loop runs a number of times equal to the mfrc522.uid.size
  // Appends 0 to single-digit Hexa no. to ensure all bytes represent in two-digit Hexa no.
  // Construct the UID by converting each byte into Hexadecimal
  String rfidTag = "";
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    rfidTag += String(mfrc522.uid.uidByte[i] < 0x10 ? "0" : " ");
    rfidTag += String(mfrc522.uid.uidByte[i], HEX);
    rfidTag.toUpperCase();
  }
  
  Serial.println("\nRFID Tag Detected:" + rfidTag);
  
  // Tag UID comparison with the predefined UID
  if (rfidTag == validTag) {
    Serial.println(F("RFID Access Granted!"));
    unlockDoor();
    buzzerActive = false;
    scanCount = 0; // Reset after successful attempt
    
  } else if (BluetoothControl::reqTempAccess()) {
    checkTempAccess();
    
  } else {
    Serial.println(F("RFID Access Denied!"));
    scanCount++;
    if (scanCount == 3) {
      Serial.println(F("Too many invalid RFID attempts!"));
      buzzerActive = true;
      scanCount = 0; // Reset
    }
  }
  
  mfrc522.PICC_HaltA();  //Halt the RFID module operation and prepare for the next scan
}

void RFIDDoorLock::checkTempAccess() {
  String rfidTag = "";
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    rfidTag += String(mfrc522.uid.uidByte[i] < 0x10 ? "0" : " ");
    rfidTag += String(mfrc522.uid.uidByte[i], HEX);
    rfidTag.toUpperCase();
  }
  
  Serial.println("\nRFID Tag Detected:" + rfidTag);
  
  if (rfidTag == tempTag) {
    Serial.println(F("Visitor Access Granted!"));
    unlockDoor();
    scanCount = 0; // Reset
  } else {
    Serial.println(F("Access Denied!"));
    scanCount++;
    if (scanCount == 3) {
      Serial.println(F("Too many invalid RFID attempts!"));
      buzzerActive = true;
      scanCount = 0; // Reset
    }
  }
  mfrc522.PICC_HaltA();  //Halt the RFID module and prepare for the next scan
}
