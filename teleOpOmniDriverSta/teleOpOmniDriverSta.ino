/*
 3 channel (joystick) control for omni-wheel car
 Allow jumper selectable 'FWD direction' for car.
 
 Calculate motor speeds & send drive values to car
 as four comma separated values: v1,v2,v3,v4
 ***********************************************
 Joystick (3 DOF):
 
 X-axis (R/L sideways)
 Y-axis (Fwd/Rev)
 Theta-Z (spin)
 
         +Y
     Q2   |   Q1
          |
 -X ---- J/S ---- +X
          |
     Q3   |   Q4
         -Y

 Note designation of four 'quadrants'
 *************************************************
 Car moves in same direction as Joystick:
 FWD dir of car corresponds to +Y dir of joystick
 
            ^
           FWD
            |
          _____
         |_____|
   Q2    /  |  \   Q1
       /    M2   \
  _  /             \ _
 | |                | |
 | |--M3   CAR  M4--| |
 |_|                |_|
     \             /
       \    M1   /
   Q3    \__|__/    Q4
         |_____|
                      
 For example:
 
 Moving joystick in +Y direction sends car FWD.
 M4 runs CW and M3 runs CCW at same speed.

 Moving joystick to the right sends car sideways.
 M1 runs CW and M2 runs CCW at same speed.
  
 Twisting joystick CW (theta-Z) spins car CW.
 All four motors run CCW at same speed.
 ************************************************
 It is easy to change car's FWD direction (by 45 deg):
 1. convert rect joystick coords to polar
 2. add 45 deg to angle
 3. convert back to rect coords
          _____
         |_____|
         /  |  \
       /    M3   \
  _  /             \ _
 | |                | |
 | |--M1   CAR  M2--| |---> 'natural' FWD dir
 |_|                |_|
     \             /
       \    M4   / \
         \__|__/     \
         |_____|     _\| 
                  'new' FWD dir
                 (rotated 45 deg)
*/

/*
 * Move blue-tooth to a softwareSerial port.
 * This will leave pins 1 and 2 for debugging by IDE
*/
#include <math.h>
#define fwdDirPin 11
#define VMIN 5
#define pi 3.14159
#include <SoftwareSerial.h>
SoftwareSerial ssBT(9, 8); // BlueToothSerial(Tx, Rx)

double X = 0;   // Joystick X axis value
double Y = 0;   // Joystick Y axis value
double Z = 0;   // Joystick theta-Z value
double R, T;    // polar coords (R, Theta)
double X0, Y0, Z0;  // Joystick "center" (rest) values
String inString;  // incoming acknowledgement from car

void setup() {
  Serial.begin(9600); // For diagnostic communication w/ IDE
  pinMode(fwdDirPin, INPUT); // set HIGH to change fwd dir
  // get joystick 'rest' positions
  X0 = analogRead(A0);
  Y0 = analogRead(A1);
  Z0 = analogRead(A2);
  Serial.println("Hello from driver Station");
  // start serial port and wait for it to open
  ssBT.begin(9600);
  ssBT.setTimeout(50);
  // send initial string to car
  ssBT.println("0,0,0,0");
}

void loop() {
  // if we get an acknowledgement from car:
  if (ssBT.available() > 0) {
    inString = ssBT.readString();
    Serial.print("Received: ");
    Serial.println(inString);
    X = analogRead(A0) - X0;
    Y = analogRead(A1) - Y0;
    Z = analogRead(A2) - Z0;
    // map +/- 512 to +/- 255:
    X = X/2;
    Y = Y/2;
    Z = Z/4; // reduce gain for spin
    // use jumper to set fwdDirPin HIGH or LOW
    if (digitalRead(fwdDirPin) == HIGH) {
      r2p(); // convert rect coords to polar
      T = T + pi/4; // rotate 'FWD' dir by pi/4 (45 deg)
      p2r(); // convert polar coords back to rect
    }
    int v1,v2,v3,v4;  //; motor drive values
    v4 = Y+Z;
    v3 = -Y+Z;
    v2 = X+Z;
    v1 = -X+Z;
    // keep motors quiet at very low drive values
    if (abs(v1) < VMIN) {v1=0;}
    if (abs(v2) < VMIN) {v2=0;}
    if (abs(v3) < VMIN) {v3=0;}
    if (abs(v4) < VMIN) {v4=0;}
    String strV1 = String(v1);
    String strV2 = String(v2);
    String strV3 = String(v3);
    String strV4 = String(v4);
    // send motor speed values to car:
    ssBT.println(strV1+","+strV2+","+strV3+","+strV4);
    ssBT.flush();
    Serial.print("Data sent: ");
    Serial.println(strV1+","+strV2+","+strV3+","+strV4);
  }
}

void r2p() {
  R = sqrt(X*X + Y*Y);
  T = atan(Y/X);
  // arctan is tricky... add pi for quadrants 2 & 3 
  if (X<0){
    T += pi;
  }
}

void p2r() {
  X = R * cos(T);
  Y = R * sin(T);
}
