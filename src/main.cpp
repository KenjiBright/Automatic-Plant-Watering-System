#include <Arduino.h>
#include <WiFi.h>
#include <ESP32Servo.h>

#define BLYNK_TEMPLATE_ID "TMPL6xWoL16n6"
#define BLYNK_TEMPLATE_NAME "Tưới cây V2"
#define BLYNK_AUTH_TOKEN "ItTwFbcyxCPne-6TFQqp-TBGbX9RqGso"

#include <BlynkSimpleEsp32.h>
#include <TimeLib.h>

BlynkTimer timer;

char ssid[] = "S22 của Minh";   
char pass[] = "12345678";

int moisture, sensor_analog;
const int sensor_pin = 36;  // Moisture sensor pin
const int relay = 17;       // Relay control pin
bool relay_state = false;   

bool autoMode = true;  // Variable to track auto/manual mode

// Function to control the relay
void controlRelay(bool state, bool fromAuto = false) {
  if (fromAuto && !autoMode) return;  // Skip auto control if in manual mode
  
  relay_state = state;
  digitalWrite(relay, state);  // Changed: Removed the ! operator to fix relay logic
  Blynk.virtualWrite(V0, state);
  if (!fromAuto) {
    Blynk.virtualWrite(V1, state);
  }
}

void checkMoistureTrigger() {
  if (moisture < 5) {
    digitalWrite(relay, HIGH);  // Turn on pump
    Blynk.virtualWrite(V0, 1);  // Update status LED
  }
  else if (moisture > 20) {
    digitalWrite(relay, LOW);   // Turn off pump
    Blynk.virtualWrite(V0, 0);  // Update status LED
  }
}

void sendSensor()
{
  // Read moisture sensor
  sensor_analog = analogRead(sensor_pin);
  moisture = (100 - ((sensor_analog / 4095.0) * 100));
  
  // Debug output
  Serial.print("Moisture: ");
  Serial.print(moisture);
  Serial.println("%");
  
  // Send to Blynk
  Blynk.virtualWrite(V2, moisture);
  
  // Check moisture trigger separately from button control
  checkMoistureTrigger();
}

// Handle manual control from Blynk app
BLYNK_WRITE(V1)
{
    // Direct button control of the relay
    if (param.asInt()) {
        digitalWrite(relay, HIGH);  // Turn on pump
        Blynk.virtualWrite(V0, 1);  // Update status LED
    } else {
        digitalWrite(relay, LOW);   // Turn off pump
        Blynk.virtualWrite(V0, 0);  // Update status LED
    }
}

void setup()
{
  // Initialize hardware
  Serial.begin(115200);
  
  // Set pin modes
  pinMode(relay, OUTPUT);
  pinMode(sensor_pin, INPUT);  // Moisture sensor is an input
  
  // Initialize relay to OFF state (pump off)
  digitalWrite(relay, LOW);   // LOW turns pump off
  
  // Connect to Blynk
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  
  // Setup timer for sensor readings
  timer.setInterval(1000L, sendSensor);
  
  // Update Blynk with initial state
  Blynk.virtualWrite(V0, 0);  // Status LED
  Blynk.virtualWrite(V1, 0);  // Button state
}

void loop()
{
    Blynk.run();
    timer.run();
}