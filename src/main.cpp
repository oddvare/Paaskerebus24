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
#define firsttime 0

HardwareSerial gpsSerial(1);
TinyGPSPlus gps;

//======our target
//const float pos_lat =59.3528644;
//const float pos_long =5.2192641;
//=====test target  
const float pos_lat = 59.400607;
const float pos_long = 5.2833096;

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
<!DOCTYPE HTML><html><head>
  <title>Fritz portal</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  </head><body>
  <h3>Fritz box</h3>
  <br><br>
  <h4>Distance to target:</h4>
  <table> 
  <tr>
  <th>Distance (km)</th>  
  <th>Course to target</th> 
  </tr>  
  <tr>  
  <td>distanceKm</td> 
  <td>courseTo</td>
  </tr>
  </table>
  <br><br>
  
  <pre>
  <h4>Hint:</h4>
  <ul>  
  <li>Cu</li>
  <li>NY</li>
  <li>Kolhusvegen 116</li>
  </ul>


  </pre>  
  <button onclick="window.location.href='/hint'">Check location</button>
  <button onclick="window.location.href='/hint'">Get hint</button>
  </body></html>
  )rawliteral";

void setupServer(){
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send_P(200, "text/html", index_html);
      Serial.println("Client Connected");
  });

  server.on("/get", HTTP_GET, [] (AsyncWebServerRequest *request) {
      String inputMessage;
      String inputParam;

      if (request->hasParam("name")) {
        inputMessage = request->getParam("name")->value();
        inputParam = "name";
        user_name = inputMessage;
        Serial.println(inputMessage);
        name_received = true;
      }

      // if (request->hasParam("proficiency")) {
      //   inputMessage = request->getParam("proficiency")->value();
      //   inputParam = "proficiency";
      //   proficiency = inputMessage;
      //   Serial.println(inputMessage);
      //   proficiency_received = true;
      // }
      request->send(200, "text/html", "The answer entered by you have been evaluated, if correct greeen light and box is open <br><a href=\"/\">Try Again</a>");
  });
}

void setup() {
  
  //your other setup stuff...
  //serial for debugging
  Serial.begin(115200);
  Serial.println();
  Serial.println("Setting up AP Mode");
  WiFi.mode(WIFI_AP);
  WiFi.softAP("Mystery-box");
  Serial.print("AP IP address: ");Serial.println(WiFi.softAPIP());
  Serial.println("Setting up Async WebServer");
  setupServer();
  Serial.println("Starting DNS Server");
  dnsServer.start(53, "*", WiFi.softAPIP());
  server.addHandler(new CaptiveRequestHandler()).setFilter(ON_AP_FILTER);//only when requested from AP
  //more handlers...
  //Setup for servo
  myservo.setPeriodHertz(50);
  myservo.attach(servoPin);

  server.begin();
  Serial.println("All Done!");
  Serial.println("Rødt lys");
  FastLED.addLeds<NEOPIXEL, dataPin>(leds, numLeds);
  //for (p=0; p <= 4; p += 1) {
  //leds[p]= CRGB::Red;
  //tester åpen posisjon
  fill_solid(leds,5,CRGB::GreenYellow);
  myservo.write(179);
  FastLED.show();
  //tester lukket
  delay(2000);
  myservo.write(180);
  fill_solid(leds,5,CRGB::CornflowerBlue);
  FastLED.show();
  delay(2000);
  //lar den stå i åpen posisjon
  myservo.write(10);
  //}
}

void loop() {
  //set a timer to show the user a new hint every 10 minutes
  //set a timer to show the user a new hint every 5 minutes after the first 10 minutes
  bool isFirstTime = true; // Add this line
    dnsServer.processNextRequest();
    
    if (isFirstTime) { // Add this if condition
      firsttime = timeClient.now();
      isFirstTime = false;
    }
    
    // Rest of the code...

 
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

if (distanceKm < 0.02) {
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