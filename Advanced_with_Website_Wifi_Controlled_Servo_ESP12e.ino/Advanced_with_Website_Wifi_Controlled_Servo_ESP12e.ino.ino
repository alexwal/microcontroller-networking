// The ESP-12e can be used to create an interactive website on your home Wifi network
// which can control components, such as in this toy example.

// Use:
// 1. Connect a servo to ESP-12e; input to servo is pin D0.
// 2. Upload this code on the Arduino IDE and open the Serial Monitor.
// 3. On a device connected to same wifi network as the ESP-12e,
//    visit the IP address printed by Serial Monitor on any web browser.
//    Full URL is of the form: IP_ADDRESS/
// 4. Change the value of ANGLE in the form (integer from 0-180) and watch the servo move.

#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <Servo.h>

// Inspired by Jeffrey App's Youtube video: https://www.youtube.com/watch?v=MN9-_hOpf_c

// WiFi
const char wifissid[] = "WifiNetworkName";
const char wifipsk[] = "WifiPassword";
String IP_address;

// Servo Definitions
const int SERVO_PIN = 16; // labeled D0

// Servo control object
Servo servo1;

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

  // Read the first line of the request
  String req = client.readStringUntil('\r');
  Serial.println(req);
  client.flush();

  // Parse the request
  int ang = -1;
  String digits_start_after = "/servo?angle=";
  String ang_string = getDigitStringAfter(req, digits_start_after);

  if (ang_string != "") {
    ang = ang_string.toInt(); // ang remains -1 if ang_string did not start with integers
    ang = constrain(ang, 0, 180); // servo can only move to positions at 0 to 180 degrees
  }

  // Set GPIOs according to the request.
  if (ang >= 0 && prev_ang != ang) {
    servo1.write(ang);
    prev_ang = ang;
  }
  client.flush();

  // Prepare html response
  String html_response = "HTTP/1.1 200 OK\r\n";
  html_response += "Content Type: text/html\r\n\r\n";
  html_response += "<!DOCTYPE HTML>\r\n<html>\r\n";
  html_response += "<h1> Wireless Servo Control </h1>";
  html_response += "<script> function process() {var url=\"/servo?angle=\"";
  html_response += " + document.getElementById(\"new_url\").value; location.href=url; return false; }";
  html_response += "</script>";
  html_response += "<form onSubmit=\"return process();\">";
  html_response += "Move to angle: <input type=\"text\" name=\"angle_textfield\" id=\"new_url\"> <input type=\"submit\" value=\"Go\">";
  html_response += "</form>";
  html_response += "</html>\n <br>";

  // Show a message
  if (ang >= 0) {
    html_response += "Servo is at " + String(ang);
    html_response += " degrees.";
  } else {
    html_response += "Enter an integer degree value from 0 to 180.<br>For example, try 90 degrees.";
  }

  // Send response to the client
  client.print(html_response);
  delay(1000); // Pause 1s to get servo time to move
  Serial.println("Client disconnected");

}

void connectWiFi() {
  Serial.println();
  Serial.println("Connecting to: " + String(wifissid));
  WiFi.mode(WIFI_STA);
  WiFi.begin(wifissid, wifipsk);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.println("Connecting to wifi...");
    delay(1000);
  }
  Serial.println("WiFi Connected");
  Serial.println("IP address: ");
  IP_address = WiFi.localIP().toString();
  Serial.println(IP_address);
}

void setupMDNS() {
  // Call mdns to setup mdns to pint to domain.local
  if (!MDNS.begin("begins-mdns"))
  {
    Serial.println("Error setting up MDNS responder!");
    while (1) {
      delay(1000);
    }
  }
  Serial.println("mDNS responder started");
}

void initHardWare() {
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
  if ((c >= '0') && (c <= '9')) {
    return 1;
  } else {
    return 0;
  }
}

