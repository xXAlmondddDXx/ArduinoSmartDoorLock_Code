/*************************************************************************************
Author: Chen Yu Wei (T00228555)
Subject: Capstone Project
Title: Arduino Smart Door Lock System
Programme: Manufacturing and Mechanical Engineering
Institution: Munster Technological University

Original by Limor Fried/Ladyada for Adafruit Industries
Modified by Chen Yu Wei, For Final Year Capstone Project

Hardware: Arduino Uno R3, R307 Fingerprint Sensor, HC-05, RFID MFRC522, HC-SR04,
          Solenoid Lock, PiezoBuzzer, LEDs, 5V Relay.

Software: Arduino IDE (Board: Arduino Uno), Arduino Bluetooth Control (by Giristudio)
*************************************************************************************/

// --- Include library ---
#include <Adafruit_Fingerprint.h>
#include "BluetoothControl.h"
#include "RFIDDoorLock.h"
#include "UltrasonicSensor.h"

// --- Declare variable pins ---
#define ledRed      A0
#define ledGreen    A1
#define buzzerPin   A2
#define solenoidPin A3

//--- Variables use across functions ---
const byte maxFingerAttempt = 3; // Replace your max fingerprint attempt here
byte fingerAttempt = 0;

bool buzzerActive = false;
bool buzzerState = false;
unsigned long lastBuzzTime = 0;
const unsigned long buzzerInterval = 200;

// --- Software Serial ---
SoftwareSerial fingerprintSerial(2, 3);

Adafruit_Fingerprint finger = Adafruit_Fingerprint(&fingerprintSerial);
BluetoothControl btControl;
RFIDDoorLock rfidF;
UltrasonicSensor ultraSensor;

void setup() {
  Serial.begin(9600);   // Init serial monitor at 9600 baud rate
  finger.begin(57600);  // Init fingerpring sensor at 57600 baud rate

  if (finger.verifyPassword()) {
    Serial.println(F("Found fingerprint sensor!"));
  } else {
    Serial.println(F("Did not find fingerprint sensor :("));
    while (1) delay (1);
  }

  // --- Init sub functions ---
  btControl.begin();    // Initialise the Bluetooth functions
  rfidF.begin();        // Initialise the RFID functions
  ultraSensor.begin();  // Initialise the Ultrasonic functions
  
  pinMode(ledRed, OUTPUT);
  pinMode(ledGreen, OUTPUT);
  pinMode(buzzerPin, OUTPUT);
  pinMode(solenoidPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(trigPin, OUTPUT);

  digitalWrite(ledRed, HIGH);
  digitalWrite(ledGreen, LOW);
  digitalWrite(buzzerPin, LOW);
  digitalWrite(solenoidPin, LOW);

  finger.getTemplateCount();
  Serial.print(F("\nR307 Sensor contains ")); Serial.print(finger.templateCount); Serial.println(F(" templates"));
  Serial.println(F("Waiting for fingerprint access"));
  Serial.println(F("OR\nUser Bluetooth command"));
  Serial.println(F("OR\nRFID access"));
}

void loop() {
  // --- Call for Fingerprint function ---
  getFingerprintIDez();
  delay(25);  // Small delay to ensure stability
  
  // --- Call for sub functions ---
  btControl.checkCommand();
  rfidF.readRFID();
  ultraSensor.doorStatus();
  doorAlarm();
}

// --- Door Alarm System ---
void doorAlarm() {
  if (buzzerActive) {
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
  } else if (ultraSensor.doorOpenAlert) {
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
  } else {
    noTone(buzzerPin);
    digitalWrite(ledRed, HIGH);
    buzzerState = false;
  }
}

// --- Fingerprint Authentication ---
// Returns error message if failed, otherwise returns ID#
int getFingerprintIDez() {
  uint8_t p = finger.getImage();
  if (p != FINGERPRINT_OK){
  return -1;
}

  p = finger.image2Tz();
  if (p != FINGERPRINT_OK){
  return -1;
}

  // Mateching the fingerprint...
  p = finger.fingerFastSearch();
  if (p != FINGERPRINT_OK){
    Serial.println("\nAccess Denied!");
    fingerAttempt++;
    if (fingerAttempt >= maxFingerAttempt) {
      Serial.println("Too many invalid fingerprint attemps!");
      buzzerActive = true; // Trigger alarm
      fingerAttempt = 0; // Reset
    }
    return -1;
  }

  // Fingerprint Matched
  Serial.print(F("\nFound ID #")); Serial.print(finger.fingerID); 
  Serial.print(F(" with confidence of ")); Serial.println(finger.confidence);
  Serial.println(F("Access Granted!"));
  unlockDoor();
  fingerAttempt = 0; // Reset after successful attempt
  buzzerActive = false; // Reset the alarm system
  return finger.fingerID;
}

// --- Access Granted ---
void unlockDoor() {
  Serial.println(F("Unlocking the door..."));
  digitalWrite(ledRed, LOW);
  digitalWrite(ledGreen, HIGH);
  digitalWrite(solenoidPin, HIGH);
  delay(3000);
  digitalWrite(ledRed, HIGH);
  digitalWrite(ledGreen, LOW);
  digitalWrite(solenoidPin, LOW);
  Serial.println("Door is locked");

  // Reset the timer
  ultraSensor.doorOpenTime = millis();
}
