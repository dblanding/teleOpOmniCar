/*
 * TeleOp code for an "Omni wheel" type car with 4 motors.
 * A driver station sends motor speed data to the car via bluetooth.
 * Data arrives on the serial port as 4 comma separated substrings: 
 * "str1,str2,str3,str4", with substrings representing the speed of
 * each motor in a range from -255 to +255,
 * where -255 is full speed reverse and +255 is full speed forward.
 * 
 * Code is based on the SerialCallReponseASCII tutorial online at:
 * https://www.arduino.cc/en/Tutorial/SerialCallResponseASCII
 * Bluetooth configuration was well explained by the article at the following link:
 * http://howtomechatronics.com/tutorials/arduino/how-to-configure-pair-two-hc-05-bluetooth-module-master-slave-commands/
*/

#include <Wire.h>
#include <Adafruit_MotorShield.h>
#include "utility/Adafruit_MS_PWMServoDriver.h"
#define ledPin 12

Adafruit_MotorShield AFMS = Adafruit_MotorShield(); 
Adafruit_DCMotor *mtr1 = AFMS.getMotor(1); // mtr1 is a pointer
Adafruit_DCMotor *mtr2 = AFMS.getMotor(2);
Adafruit_DCMotor *mtr3 = AFMS.getMotor(3);
Adafruit_DCMotor *mtr4 = AFMS.getMotor(4);

int spd1; // motor 1 speed
int spd2; // motor 2 speed
int spd3; // motor 3 speed
int spd4; // motor 4 speed

void setup()
{
  Serial.begin(9600); // For diagnostic communication w/ IDE
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);
  Serial2.begin(9600); // For BlueTooth communication w/ controller
  Serial2.setTimeout(50);
  while (!Serial2) {
    ; // wait for serial port to connect.
  }
  AFMS.begin();
  mtr1->setSpeed(0); // use '->' (not '.') because mtr1 is a pointer
  mtr2->setSpeed(0);
  mtr3->setSpeed(0);
  mtr4->setSpeed(0);
}
 
void loop() {
  digitalWrite(ledPin, LOW);
  // Serial.println("In loop");
  // Read serial input:
  if (Serial2.available() > 0) {
    delay(50); // give time for entire string to arrive
    //Serial.println("Serial2 available");
    String inString = Serial2.readString();
    Serial.print("Data Received: ");
    Serial.println(inString);
    String str1 = getSubString(inString, ',', 0);
    String str2 = getSubString(inString, ',', 1);
    String str3 = getSubString(inString, ',', 2);
    String str4 = getSubString(inString, ',', 3);
    int spd1 = str1.toInt();
    int spd2 = str2.toInt();
    int spd3 = str3.toInt();
    int spd4 = str4.toInt();
    if (spd1 < 0) {
      mtr1->run(BACKWARD);
    }
    else {
      mtr1->run(FORWARD);
    }
    if (spd2 < 0) {
      mtr2->run(BACKWARD);
    }
    else {
      mtr2->run(FORWARD);
    }
    if (spd3 < 0) {
      mtr3->run(BACKWARD);
    }
    else {
      mtr3->run(FORWARD);
    }
    if (spd4 < 0) {
      mtr4->run(BACKWARD);
    }
    else {
      mtr4->run(FORWARD);
    }
    if (spd1 > 255) {
      spd1 = 255;
    }
    if (spd1 < -255) {
      spd1 = -255;
    }
    if (spd2 > 255) {
      spd2 = 255;
    }
    if (spd2 < -255) {
      spd2 = -255;
    }
    if (spd3 > 255) {
      spd3 = 255;
    }
    if (spd3 < -255) {
      spd3 = -255;
    }
    if (spd4 > 255) {
      spd4 = 255;
    }
    if (spd4 < -255) {
      spd4 = -255;
    }
    mtr1->setSpeed(abs(spd1));
    mtr2->setSpeed(abs(spd2));
    mtr3->setSpeed(abs(spd3));
    mtr4->setSpeed(abs(spd4));
    // Send a character to ask for more data
    Serial2.println("A");
    Serial2.flush();
    Serial.print("Sent: ");
    Serial.println("A");
  }
}

// Split string into substrings, return substring by index
String getSubString(String data, char separator, int index) {
  int found = 0;
  int strIndex[] = { 0, -1 };
  int maxIndex = data.length() - 1;
  for (int i = 0; i <= maxIndex && found <= index; i++) {
    if (data.charAt(i) == separator || i == maxIndex) {
      found++;
      strIndex[0] = strIndex[1] + 1;
      strIndex[1] = (i == maxIndex) ? i+1 : i;
    }
  }
  return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}

// Debugging tool to show that code reached the point where this is called
void flashLED() {
  digitalWrite(ledPin, HIGH);
  delay(200);
  digitalWrite(ledPin, LOW);
  delay(200);
}
