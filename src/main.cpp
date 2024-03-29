#include <Arduino.h>
#include <DNSServer.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include "ESPAsyncWebServer.h"
#include <FastLED.h>
#include <ESP32Servo.h>
//#include <HardwareSerial.h>
//#include <SoftwareSerial.h>
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

//=======Code to be used to open box if everything fails========
String user_name;
String Open= "Boksen er åpen";
String proficiency;
bool name_received = false;
bool proficiency_received = false;

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
  Hallo, ich bin Fritz!
  <br><br>
  Ich habe deinen Osterschnüffler mitgenommen und versteckt.
  Bring mich zum richtigen Ort, um den Süßigkeiten zurückzubekommen.
  <br><br>
  Erster Hinweis:
  Cu
  <br><br>
  Zweiter Hinweis:
  NY
    <br><br>
  Dritter Hinweis:
  Kolhusvegen 116
  <h4>Distance to target:</h4>
  wenn der erste LED grün ist, hat der GPS einen Fix, sonst blinkt der erste LED langsam rot
  <table> 
  <tr>
  <th>Distance (km)</th>  
  <th>Course to target</th> 
  </tr>  
  <tr>  
  <td>
    <script>
      document.write(distanceKm.toFixed(2));
    </script>
  </td>
  <td>
    <script>
      document.write(courseTo.toFixed(2));    
    </script>
  </td>
  </tr>
  </table>
  <br><br>
  <h4>GPS Coordinates:</h4>
  <table> 
  <tr>
  <th>Latitude</th>  
  <th>Longitude</th> 
  </tr>  
  <tr>  
  <td>
    <script>
      document.write(gps.location.lat().toFixed(6));
    </script>
  </td>
  <td>
    <script>
      document.write(gps.location.lng().toFixed(6));    
    </script>
  </td>
  </tr>
  </table>
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
  <br><br>
<hr> <!-- Dividing line -->
  <br><br> <!-- Whitespace -->

  <form action="/get">
    <br>
    Name: <input type="text" name="name">
    <br>

    <input type="submit" value="Submit">
  </form>
  </body></html>
  )rawliteral";

class CaptiveRequestHandler : public AsyncWebHandler {
public:
  CaptiveRequestHandler() {}
  virtual ~CaptiveRequestHandler() {}

  bool canHandle(AsyncWebServerRequest *request){
    //request->addInterestingHeader("ANY");
    return true;
  }

  void handleRequest(AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", index_html);
  }
};

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
//=====setup for GPS
  gpsSerial.begin(9600, SERIAL_8N1, GPS_RX, GPS_TX);
  
 
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
  // Read GPS data
  while (gpsSerial.available() > 0) {
    gps.encode(gpsSerial.read());
  }
// Debug: if we haven't seen lots of data in 5 seconds, something's wrong.
if (millis() > 5000 && gps.charsProcessed() < 10) // uh oh
{
  Serial.println("ERROR: not getting any GPS data!");
  // dump the stream to Serial
  Serial.println("GPS stream dump:");
  while (true) // infinite loop
    if (gpsSerial.available() > 0) // any data coming in?
      Serial.println(gpsSerial.read());
}
  dnsServer.processNextRequest();
  if(name_received){
      Serial.print("Hello ");Serial.println(user_name);
      if(user_name=="Askeladden"){
        Serial.println("Grønt lys");
        fill_solid(leds,5,CRGB::Green);FastLED.show();
        myservo.write(10);

    }
      else{fill_solid(leds,5,CRGB::Black);FastLED.show();delay(500);
          fill_solid(leds,5,CRGB::Red);FastLED.show();delay(500);
          fill_solid(leds,5,CRGB::Black);FastLED.show();delay(500);
          fill_solid(leds,5,CRGB::Red);FastLED.show();
          myservo.write(170);
    }

      name_received = false;

      Serial.println("We'll wait for the next client now");
    }

// Calculate distance and course to target
//Use gps.location.isValid() to check if the GPS has a fix, and show one LED in Green whn it has a fix, otherwise show one LED in slow blink Red
if (gps.location.isValid()) {
  Serial.println("GPS has a fix!");
  leds[0] = CRGB::Green;
  FastLED.show();
} else {
  Serial.println("GPS does not have a fix!");
  leds[0] = CRGB::Red;
  FastLED.show();
}
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
Serial.print(gps.location.lat(), 6);
Serial.print(", ");
Serial.println(gps.location.lng(), 6);       
Serial.print("Distance (km) to target: ");
Serial.println(distanceKm);
Serial.print("Course to target: ");
Serial.println(courseTo);
Serial.print("Human directions: ");
Serial.println(gps.cardinal(courseTo));
//need to add a button to check if the user has reached the destination
//if the user has reached the destination, the lights will turn green
//and the servo will turn to 10 degrees
String html = "<h4>GPS Coordinates:</h4>";
html += "<table>";
html += "<tr>";
html += "<th>Latitude</th>";
html += "<th>Longitude</th>";
html += "</tr>";
html += "<tr>";
html += "<td>" + String(gps.location.lat(), 6) + "</td>";
html += "<td>" + String(gps.location.lng(), 6) + "</td>";
html += "</tr>";
html += "</table>";
request->send(200, "text/html", html);
if (distanceKm < 0.02) {
  Serial.println("You have reached your destination!");

  delay(1000);
   Serial.println("grønne lys");
   for (int i = 0; i < numLeds; i++) {
    leds[i] = CRGB::Green;
    myservo.write(10);
    FastLED.show();
    } 
}
  else {
  Serial.println("You are still on your way.");
  delay(1000); 
     Serial.println("røde lys");
   for (int i = 0; i < numLeds; i++) {
    leds[i] = CRGB::Red;
    myservo.write(170);
    FastLED.show();
}
}
}

// put function definitions here:
int myFunction(int x, int y) {
  return x + y;
}