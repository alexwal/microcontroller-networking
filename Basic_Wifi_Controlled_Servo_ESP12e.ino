#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <Servo.h>

//WiFi
const char wifissid[] = "MDPAHome";
const char wifipsk[] = "migasumi2009";

//LED Definitions
const int SERVO_PIN = 16; // labeled D0

Servo servo1;  // servo control object

WiFiServer server(80);

void setup() {
  // put your setup code here, to run once:
  initHardWare();
  connectWiFi();
  server.begin();
  setupMDNS();
}

int prev_ang = -1;

void loop() {
  // Check if connected
  WiFiClient client = server.available();
  if (!client) {
    return;
  }

  //Read the first line of the request
  String req = client.readStringUntil('\r');
  Serial.println(req);
  client.flush();

  // Parse the request
  int ang = -1;
  String digits_start_after = "/servo/";
  String ang_string = getDigitStringAfter(req, digits_start_after);
//  String ang_string = "";
//  int digit_index = req.indexOf(digit_starts_after) + digit_starts_after.length();
//  for (char gotChar = req.charAt(digit_index); isDigit(gotChar); gotChar = req.charAt(++digit_index)) { // get input digit
//    ang_string += req.charAt(digit_index);
//  }

  if (ang_string != "") {
    ang = ang_string.toInt(); // ang remains -1 if ang_string did not start with integers
    ang = constrain(ang, 0, 180); // servo can only move to positions at 0 to 180 degrees
  }

  //Set GPIOs according to the request.
  if (ang >= 0 && prev_ang != ang) {
     servo1.write(ang);
     prev_ang = ang;
  }
   client.flush();

   //Prepare html response

   String s = "HTTP/1.1 200 OK\r\n";
   s += "Content Type: text/html\r\n\r\n";
   s += "<!DOCTYPE HTML>\r\n<html>\r\n";

   //Print a message
   if (ang >= 0) {
    s += "SERVO is at " + String(ang);
    s += " degrees.";
   }
   else {
    s += "Invalid Request. <br> Try /servo/0";
   }
   s += "</html>\n";

   //send response to the client
   client.print(s);
   delay(1000); // pause 1s to get servo time to move
   Serial.println("Client disconnected");

}

void connectWiFi() {
  byte ledStatus = LOW;
  Serial.println();
  Serial.println("Connecting to: " + String(wifissid));
  WiFi.mode(WIFI_STA);

  WiFi.begin(wifissid,wifipsk);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.println("Connecting to wifi...");
    delay(1000);
  }
  Serial.println("WiFi Connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());  
}

void setupMDNS()
{
  //Call mdns to setup mdns to pint to domain.local
  if (!MDNS.begin("begins-mdns"))
  {
    Serial.println("Error setting up MDNS responder!");
    while (1) {
      delay(1000);
    }
  }
  Serial.println("mDNS responder started");
}

void initHardWare()
{
  Serial.begin(115200);
  pinMode(SERVO_PIN, OUTPUT);
  servo1.attach(SERVO_PIN);
  servo1.write(0);
}

String getDigitStringAfter(String input, String pattern) {
  // Returns the first continuous string of digits in input beginning directly
  // after first occurence of pattern; otherwise, return the empty string.
  String found_string = "";
  int digit_index = input.indexOf(pattern) + pattern.length();
  for (char gotChar = input.charAt(digit_index); isDigit(gotChar); gotChar = input.charAt(++digit_index)) { // get input digit
    found_string += input.charAt(digit_index);
  }
  return found_string;
}

int isDigit(char c) {
  if ((c>='0') && (c<='9')) {
    return 1;
  } else {
    return 0;
  }
}

