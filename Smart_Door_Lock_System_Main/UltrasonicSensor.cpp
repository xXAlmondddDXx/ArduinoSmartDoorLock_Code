//Ultrasonic Source file (w/ extension .cpp)
//The source file contains the actual code for the Ultrasonic sensor program.
//Replace your sensing distance under [float distance = (duration*sp of sound)/2].

#include "UltrasonicSensor.h"

const unsigned long UltrasonicSensor::doorOpenAllowance = 10000;
unsigned long UltrasonicSensor::doorOpenTime = 0;
bool UltrasonicSensor::doorOpenAlert = false;
bool UltrasonicSensor::doorAlertMsgPrinted = false;

void UltrasonicSensor::begin() {
  pinMode(trigPin, OUTPUT);    //Set the trigger pin as output to transmit sound wave
  pinMode(echoPin, INPUT);     //Set the echo pin as input to receive sound wave
  Serial.println(F("Door Status Detection: activated!"));
}

float UltrasonicSensor::getDistance() {
  // The trigger point sends out 8 cycle sonic burst from the transmitter
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  //Read the echoPin to acquire the duration of the ultrasonic pulse to return
  //Convert into distance via speed of sound @ 0.0343 cm/µs
  duration = pulseIn(echoPin, HIGH);
  distance = (duration * 0.0343) / 2;

  return distance;
}

// --- Door Status Detection ---
void UltrasonicSensor::doorStatus() {
  getDistance(); // Call for distance measure function

  if (distance > 10) {
    if ((millis() - doorOpenTime > doorOpenAllowance) && !buzzerActive && !shutDoorAlert) {
      doorOpenAlert = true;
      if (!doorAlertMsgPrinted) {
        Serial.println(F("\nPlease shut the door!"));
        doorAlertMsgPrinted = true;
      }
    }
  } else {
    doorOpenTime = millis();
    doorOpenAlert = false;
    doorAlertMsgPrinted = false;
  }
}
