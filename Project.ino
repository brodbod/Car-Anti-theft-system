#include <WiFiNINA.h>
#include <Wire.h>  // include the standard Wire library
#include <BH1750.h> // include the BH1750 library
#include "secrets.h" 

BH1750 GY30; // instantiate a sensor event object

char ssid[] = SECRET_SSID;        
char pass[] = SECRET_PASS;
WiFiClient client;
char HOST_NAME[] = "maker.ifttt.com";
String PATH_NAME = "/trigger/alarm_tripped/with/key/c60HHHXkeOXUxmjCv95JOi"; // change your EVENT-NAME and YOUR-KEY
int buttonState = 0;
int lastButtonState = HIGH; // Variable to store the previous button state
bool alarmActive = false; // Control the alarm
const float lightThreshold = 2.0; // Light level threshold

void setup() {
  // initialize WiFi connection
  WiFi.begin(ssid, pass);

  Serial.begin(9600);
  while (!Serial);

  // connect to web server on port 80:
  if (client.connect(HOST_NAME, 80)) {
    // if connected:
    Serial.println("Connected to server");
  } else { // if not connected:
    Serial.println("connection failed");
  }

  pinMode(13, OUTPUT);
  pinMode(12, OUTPUT); 
  pinMode(10, INPUT_PULLUP); // Use internal pull-up resistor
  digitalWrite(13, HIGH); 
  Wire.begin();
  GY30.begin();
}

void loop() {
  // Read the button state
  buttonState = digitalRead(10);

  // Check if the button state has changed
  if (buttonState != lastButtonState) {
    // If the button state has changed to LOW (button pressed), toggle the alarmActive flag
    if (buttonState == LOW) {
      if (alarmActive) {
        // If the alarm is active, only toggle it off if it's dark
        float lux = GY30.readLightLevel();
        if (lux < lightThreshold) {
          alarmActive = false;
          Serial.println("Alarm deactivated due to low light level");
        }
      } else {
        // If the alarm is not active, toggle it on
        alarmActive = true;
      }
    }
    // Save the current state as the last state for next comparison
    lastButtonState = buttonState;
  }

  if (alarmActive) {
    float lux = GY30.readLightLevel();
    Serial.println();
    Serial.println((String)lux + " lx");
    String queryString = "?value1=" + String(lux);
    if (lux >= lightThreshold) {
      Serial.println("Lx is more than " + String(lightThreshold));
      Serial.println("SOUND THE ALARM!");
      digitalWrite(12, HIGH); 
      if (client.connect(HOST_NAME, 80)) {
        client.println("GET " + PATH_NAME + queryString + " HTTP/1.1");
        client.println("Host: " + String(HOST_NAME));
        client.println("Connection: close");
        client.println(); // end HTTP header
        client.stop(); // Close the connection
      } else {
        Serial.println("Failed to connect to server");
      }
    }
  } else {
    digitalWrite(12, LOW); 
  }

  // Pause for 1 second before repeating the sensor poll
  delay(1000);
}
