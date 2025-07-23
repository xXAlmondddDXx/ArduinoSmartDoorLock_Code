// This source file contains the actual code for the RFID Access program.
// RFID MFRC522
// Take note that the tag UID is case sensitive and space sensitive.
// Replace the RFID UID according to your settings.

#ifndef RFID_ACCESS_H
#define RFID_ACCESS_H

#include <SPI.h>
#include <MFRC522.h>
#include <LiquidCrystal_I2C.h>

#define RFID_SS  10 // Chip select pin for SD
#define RFID_RST 9

MFRC522 mfrc522(RFID_SS, RFID_RST);

extern SoftwareSerial btSerial;
extern void unlockDoor();
extern void doorAlarm();
extern bool reqTempAccess();
extern void logEvent(String event);

const char masterTag[] PROGMEM  = " 73 47 1D 95"; // Replace your card UID here
const char visitorTag[] PROGMEM = " 23 80 85 90";

static const int maxRfidAttempt = 3;
int rfidAttempt = 0;

void initRFID() {
  Serial.begin(115200);

  SPI.begin();
  mfrc522.PCD_Init();

  if (mfrc522.PCD_PerformSelfTest()) {
    Serial.println(F("RFID ready!"));
  } else {
    Serial.println(F("RFID failed!"));
  }
}

void checkRFID(LiquidCrystal_I2C &lcd) {

  // --- IF NOT new card present OR NOT card serial read THEN return ---
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
  Serial.println("\nRFID Tag Detected: " + rfidTag);

  // --- Tag UID comparison ---
  if (rfidTag == masterTag) {
    Serial.println(F("RFID Access Granted"));
    lcd.clear();
    lcd.setCursor(0,0); lcd.print(F("RFID:"));
    lcd.setCursor(0,1); lcd.print(F("Access Granted"));
    logEvent("RFID Access: Granted | Master");
    rfidAttempt = 0; // Reset before unlocking door to prevent 'delay' blocking
    buzzerActive = false;
    unlockDoor();

  } else if (reqTempAccess()) {
    if (rfidTag == visitorTag) {
      Serial.println(F("Visitor Access Granted"));
      lcd.clear();
      lcd.setCursor(0,0); lcd.print(F("Visitor Entry:"));
      lcd.setCursor(0,1); lcd.print(F("Access Granted"));
      logEvent("RFID Access: Granted | Visitor");
      rfidAttempt = 0; // Reset
      unlockDoor();
    }

  } else {
    Serial.println(F("RFID Access Denied!"));
    lcd.clear();
    lcd.setCursor(0,0); lcd.print(F("RFID:"));
    lcd.setCursor(0,1); lcd.print(F("Access Denied!"));
    logEvent("RFID Access: Denied");

    rfidAttempt++;
    if (rfidAttempt > maxRfidAttempt) {
      Serial.println(F("Too many invalid RFID attempts!"));
      btSerial.println(F("Alarm Triggered: Too many invalid RFID attempts!"));

      lcd.clear();
      lcd.setCursor(0,0); lcd.print(F("Too many invalid"));
      lcd.setCursor(0,1); lcd.print(F("attempts!"));
      logEvent("RFID Access: Denied");
      buzzerActive = true;
      rfidAttempt = 0; // Reset
    }
  }

  mfrc522.PICC_HaltA();       // Stops communication with the card
  mfrc522.PCD_StopCrypto1();  // Ends encrypted session
}

#endif
