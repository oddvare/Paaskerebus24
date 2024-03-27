#include <Arduino.h>
#include <DNSServer.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include "ESPAsyncWebServer.h"
#include <FastLED.h>
#include <ESP32Servo.h>
#include <HardwareSerial.h>
#include <TinyGPS++.h>

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
#define GPS_RX 16  // GPS module RX pin
#define GPS_TX 17  // GPS module TX pin

HardwareSerial gpsSerial(1);
TinyGPSPlus gps;


// put function declarations here:
int myFunction(int, int);

void setup() {
  // put your setup code here, to run once:
  int result = myFunction(2, 3);
}

void loop() {
  // put your main code here, to run repeatedly:
}

// put function definitions here:
int myFunction(int x, int y) {
  return x + y;
}