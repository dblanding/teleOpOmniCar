/*
 * GY-273 magnetometer has been added to report magnetic heading to driver sta. 
 * Unfortunately, the effect of the permanent magnets seems to be overwhelming
 * the sensor and prevents detection of the Earth's magnetic field.
 * 
 * TeleOp code for an "Omni wheel" type car with 4 motors.
 * A driver station sends motor speed data to the car via bluetooth.
 * Data arrives on the serial port as 4 comma separated substrings: 
 * "str1,str2,str3,str4", with substrings representing the speed of
 * each motor (m1,m2,m3,m4)in a range from -255 to +255,
 * where -255 is full speed reverse and +255 is full speed forward.
 * After receiving motor speed data, car replies with its magnetic heading.
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
#define address 0x1E //0011110b, I2C 7bit address of HMC5883
#define pi 3.14159

Adafruit_MotorShield AFMS = Adafruit_MotorShield(); 
Adafruit_DCMotor *mtr1 = AFMS.getMotor(1); // mtr1 is a pointer
Adafruit_DCMotor *mtr2 = AFMS.getMotor(2);
Adafruit_DCMotor *mtr3 = AFMS.getMotor(3);
Adafruit_DCMotor *mtr4 = AFMS.getMotor(4);

int spd1; // motor 1 speed
int spd2; // motor 2 speed
int spd3; // motor 3 speed
int spd4; // motor 4 speed

void setup() {
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);
  
  Wire.begin();
  //Put the HMC5883 IC into the correct operating mode
  Wire.beginTransmission(address); //open communication with HMC5883
  Wire.write(0x02); //select mode register
  Wire.write(0x00); //continuous measurement mode
  Wire.endTransmission();

  Serial.begin(9600);  // Serial communictes with IDE

  Serial2.begin(9600);   // Serial2 communicates with driver station
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
  int x,y,z; //triple axis magnetometer data
  digitalWrite(ledPin, LOW);
  // Read serial input:
  if (Serial2.available() > 0) {
    delay(50); // give time for entire string to arrive
    String inString = Serial2.readString();
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
    
    // Get heading (degrees) and send to driver station
    // Tell the HMC5883L where to begin reading data
    Wire.beginTransmission(address);
    Wire.write(0x03); //select register 3, X MSB register
    Wire.endTransmission();
    
    //Read data from each axis, 2 registers per axis
    Wire.requestFrom(address, 6);
    if(6<=Wire.available()){
      x = Wire.read()<<8; //X msb
      x |= Wire.read(); //X lsb
      z = Wire.read()<<8; //Z msb
      z |= Wire.read(); //Z lsb
      y = Wire.read()<<8; //Y msb
      y |= Wire.read(); //Y lsb
    }
    
    float h = atan((float)y/(float)x); // heading
    if (x<0) {
      h += pi;
    }
    if (h<0) {
      h += 2*pi;
    }
    h = h * 180/pi;
    Serial2.println((int)h);
    Serial2.flush();
    Serial.println((int)h);
    Serial.flush();
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

