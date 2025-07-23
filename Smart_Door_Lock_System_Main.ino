/***************************************************************************
Author: Chen Yu Wei (T00228555)
Subject: Final Year Capstone Project
Title: Smart Door Lock System
Programme: Manufacturing and Mechanical Engineering
Institution: Munster Technological University

Hardware: Arduino Uno R4, HC-05, HC-SR04, RTC DS3231, Micro SD Card Adapter,
          RFID MFRC522, Solenoid Lock, 5V Relay, LCD1602I2C, PiezoBuzzer,
          LEDs.

Software: Arduino IDE, Self Designed App via MIT App Inventor
***************************************************************************/

// --- Includes library ---
#include <Adafruit_Fingerprint.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>

#include "BluetoothControl.h"
#include "DoorStatus.h"
#include "EventLog.h"
#include "RFIDAccess.h"

// --- Declare Addresses ---
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&Serial1);
LiquidCrystal_I2C lcd(0x27, 16, 2); // LCD address

// --- Declare pins ---
#define ledRed      A0
#define ledGreen    A1
#define buzzerPin   A2
#define solenoidPin A3

// --- Declare variables ---
const int maxFingerAttempt = 3; // Replace your max fingerprint attempt here
int fingerAttempt = 0;

bool buzzerActive = false;
bool buzzerState = false;
unsigned long lastBuzzTime = 0;
const unsigned long buzzerInterval = 200;
bool isLoggedEvent = false;

void setup() {
  // --- Initialise Arduino Serial Communication Baud Rate ---
  Serial.begin(115200);
  while(!Serial);
  delay(1000);
  Serial.println(F("\nSystem Starting..."));
  Serial.println(F("=============================="));

  pinMode(ledRed, OUTPUT);
  pinMode(ledGreen, OUTPUT);
  pinMode(buzzerPin, OUTPUT);
  pinMode(solenoidPin, OUTPUT);

  digitalWrite(ledRed, HIGH);
  digitalWrite(ledGreen, LOW);
  digitalWrite(buzzerPin, LOW);
  digitalWrite(solenoidPin, LOW);

  // --- Init LCD---
  lcd.init();
  lcd.backlight();

  // --- Init Fingerprint Sensor ---
  finger.begin(57600);
  if (finger.verifyPassword()) {
    Serial.println(F("Fingerprint sensor Ready"));
  } else {
    Serial.println(F("Fingerprint sensor not detected"));
    lcd.setCursor(0,0); lcd.print(F("System Error!"));
    while (1);
  }
  finger.getTemplateCount();
  Serial.print(F("Sensor contains: #")); Serial.print(finger.templateCount); Serial.println(F(" templates"));

  // --- Init Sub-Functinos ---
  initBTCtrl();
  initDoorSensor();
  initLogSystem();
  initRFID();

  Serial.println(F("=============================="));
  lcd.setCursor(0,0); lcd.print(F("Smart Door Lock"));
  lcd.setCursor(0,1); lcd.print(F("Ready"));
  delay(2000);
  lcdDisplay();
}

void loop() {

  // --- Call for fingerprint function ---
  getFingerprintIDez();
  delay(25);  // Short delay before next scan

  // --- Call for sub-functions ---
  btCommands(lcd);
  checkDoorStatus(lcd);
  checkRFID(lcd);
  doorAlarm();

  // --- LCD Transition ---
  if (lcdTransition) {
    if (millis() - displayTime >= 2000) {
      lcdDisplay();
      lcdTransition = false;
    }
  }

  // --- Intrusion Alert ---
  if (intrusionAlert) {
    if (millis() - intrusionAlertTime >= 5000) {
      buzzerActive = false;
      lcdDisplay();
      intrusionAlert = false;
    }
  }
}

// --- Fingerprint functions ---
uint8_t getFingerprintIDez() {
  uint8_t p = finger.getImage(); // Unsigned Integer 8 bits
  if (p != FINGERPRINT_OK) return p;

  p = finger.image2Tz();
  if (p != FINGERPRINT_OK) return p;

  // --- Trying to match fingerprint ---
  p = finger.fingerFastSearch();
  if (p != FINGERPRINT_OK){
    Serial.println(F("\nFingerprint Access Denied!"));
    lcd.clear();
    lcd.setCursor(0,0); lcd.print(F("Fingerprint:"));
    lcd.setCursor(0,1); lcd.print(F("Access Denied!"));
    logEvent("Fingerprint Access: Denied.");

    fingerAttempt++;
    if (fingerAttempt > maxFingerAttempt) {
      Serial.println(F("Too many invalid fingerprint attemps!"));
      btSerial.println(F("Alarm Triggered: Too many invalid fingerprint attempts!"));

      lcd.clear();
      lcd.setCursor(0,0); lcd.print(F("Access Denied!"));
      lcd.setCursor(0,1); lcd.print(F("Over Attempts!"));
      buzzerActive = true;
      fingerAttempt = 0; // Reset
    }
    return p;
  } else {
    Serial.print(F("\nFound ID #")); Serial.println(finger.fingerID); 
    Serial.println(F("Fingerprint Access Granted!"));
    lcd.clear();
    lcd.setCursor(0,0); lcd.print(F("Fingerprint:"));
    lcd.setCursor(0,1); lcd.print(F("Access Granted!"));
    logEvent("Fingerprint Access Granted: ID #" + String(finger.fingerID));
    fingerAttempt = 0; // Reset before door unlock to prevent 'delay' blocking
    buzzerActive = false;
    unlockDoor();
    return finger.fingerID;
  }
}

void unlockDoor() {
  digitalWrite(ledRed, LOW);
  digitalWrite(ledGreen, HIGH);
  digitalWrite(solenoidPin, HIGH);
  delay(3000);
  digitalWrite(ledRed, HIGH);
  digitalWrite(ledGreen, LOW);
  digitalWrite(solenoidPin, LOW);

  // --- Reset ---
  doorOpenTime = millis();
  doorOpenAlert = false;
  doorAlertMsgPrinted = false;
  lcdDisplay();
}

void doorAlarm() {
  if (buzzerActive || doorOpenAlert) {
    if (millis() - lastBuzzTime >= buzzerInterval) {
      lastBuzzTime = millis();
      buzzerState = !buzzerState;

      if (buzzerState) {
        tone(buzzerPin, 500);
        digitalWrite(ledRed, HIGH);
      } else {
        noTone(buzzerPin);
        digitalWrite(ledRed, LOW);
      }
    }
    if (!isLoggedEvent) {
      logEvent("Alarm triggered!");
      isLoggedEvent = true;
    }
    tempAccess = false;

  } else {
    noTone(buzzerPin);
    digitalWrite(ledRed, HIGH);
    buzzerState = false;
    isLoggedEvent = false;
  }
}

void lcdDisplay() {
  if (!buzzerActive && !doorOpenAlert) {
    lcd.clear();
    lcd.setCursor(0,0); lcd.print(F("   Welcome :D"));
    lcd.setCursor(0,1); lcd.print(F("Access Verifying"));
  }
}
