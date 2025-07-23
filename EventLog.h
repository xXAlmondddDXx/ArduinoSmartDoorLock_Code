// This source file contains the actual code for the SD card data logging program.
// RTC DS3231, Mirco SD Card Adapter

#ifndef EVENT_LOG_H
#define EVENT_LOG_H

#include <Arduino.h>
#include <RTClib.h>
#include <SD.h>
#include <Wire.h>

#define SD_CS 8 // Chip select pin for SD

RTC_DS3231 rtc;
File logFile;

void initLogSystem() {

  // --- RTC init ---
  if (!rtc.begin()) Serial.println(F("RTC not found!"));
  if (rtc.lostPower()) {
    Serial.println(F("RTC lost power, setting time..."));
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__))); // Set to compile time
  }

  // --- SD Card init ---
  if (!SD.begin(SD_CS)) {
    Serial.println(F("SD initialise failed"));
    return;
  } else {
    Serial.println(F("RTC and SD initialised"));
  }
}

String getTimestamp() {
  DateTime now = rtc.now();
  char buffer[20];
  sprintf(buffer, "%02d-%02d-%02d %02d:%02d:%02d", now.day(), now.month(), now.year()%100, now.hour(), now.minute(), now.second());
  return String(buffer);
}

// --- Calculate ISO Week Number (Gregorian Calendar) ---
// ISO Week 1 is the week containing Jan 4th
// ISO Weeks start on Monday
// Calcualte by shifting Jan 4 back to Mon of that week
uint8_t getISOWeekNumber(DateTime dt) {
  DateTime jan4(dt.year(), 1, 4);
  uint8_t jan4Weekday = jan4.dayOfTheWeek(); // 0 = Sun, 1 = Mon, 2 = Tue, ... 6 = Sat
  int jan4Offset = (jan4Weekday == 0) ? -6 : (1 - jan4Weekday); // Back to Mon
  DateTime week1Start = jan4 + TimeSpan(jan4Offset * 86400L); // 86400 = secs per day
  TimeSpan delta = dt - week1Start;
  int daysSinceWeek1 = delta.days();
  return ((daysSinceWeek1 / 7) +1);
}

String getLogFileName() {
  DateTime now = rtc.now();
  uint8_t week = getISOWeekNumber(now);
  int year = now.year();

  char fileName[15];
  sprintf(fileName, "%04dW%02d.TXT", year, week);
  return String(fileName);
}

void logEvent(String event) {
  String accessLog = getTimestamp() + " - " + event;
  Serial.println(accessLog);

  String fileName = getLogFileName();
  logFile = SD.open(fileName.c_str(), FILE_WRITE);
  if (logFile) {
    logFile.println(accessLog); // Print the access log
    logFile.close();            // Save and close
    Serial.println(F("Successfuly write to SD card"));
  } else {
    Serial.println(F("Failed to write to SD card"));
  }
}

#endif
