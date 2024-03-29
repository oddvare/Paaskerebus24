#include <Arduino.h>
#include <DNSServer.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include "ESPAsyncWebServer.h"
#include <FastLED.h>
#include <ESP32Servo.h>
#include <HardwareSerial.h>
#include <TinyGPS++.h>
#include <Wire.h>

DNSServer dnsServer;
AsyncWebServer server(80);

//=========lys=====
int p=0;
#define numLeds 5
#define dataPin 12
CRGB leds[numLeds];
//----------
//=======Servo========
Servo myservo;
int servoPin = 14;
//=======GPS=========
#define GPS_RX 16  // GPS module RX pin
#define GPS_TX 17  // GPS module TX pin

HardwareSerial gpsSerial(1);
TinyGPSPlus gps;

//======our target
const float pos_lat =59.3528644;
const float pos_long =5.2192641;

// put function declarations here:
int myFunction(int, int);

//define a web page to be presented to the user when they connect to the server
//with hints on how to find the target. New hints to be presented every 10 minutes from first logon 
//and every 5 minutes after the first 10 minutes
//the hints will be presented in the form of a list
//the distance and heading to the target will be presented in the form of a table, based on gps data distanceKm and courseTo gps.cardinal(courseTo)
//the user will be able to click a button to check if they have reached the target
//if the user has reached the target, the lights will turn green and the servo will turn to 10 degrees
//picture of suspicius person carrying a box. The user will be able to click on the picture to get a hint(maybe)
const char index_html[] PROGMEM = R"rawliteral(

void setup() {
  // put your setup code here, to run once:
  int result = myFunction(2, 3);
}

void loop() {
  // put your main code here, to run repeatedly:
  double distanceKm =
  gps.distanceBetween(
    gps.location.lat(),
    gps.location.lng(),
    pos_lat,
    pos_long) / 1000.0;
double courseTo =
  gps.courseTo(
    gps.location.lat(),
    gps.location.lng(),
    pos_lat,
    pos_long);
Serial.print("Distance (km) to target: ");
Serial.println(distanceKm);
Serial.print("Course to target: ");
Serial.println(courseTo);
Serial.print("Human directions: ");
Serial.println(gps.cardinal(courseTo));
//need to add a button to check if the user has reached the destination
//if the user has reached the destination, the lights will turn green
//and the servo will turn to 10 degrees

if (distanceKm < 0.1) {
  Serial.println("You have reached your destination!");

  delay(1000);
   Serial.println("grønne lys");
   for (int i = 0; i < numLeds; i++) {
    leds[i] = CRGB::Green;
    myservo.write(10);

} else {
  Serial.println("You are still on your way.");
  delay(1000); 
}

// put function definitions here:
int myFunction(int x, int y) {
  return x + y;
}