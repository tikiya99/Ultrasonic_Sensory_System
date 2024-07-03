#include <Arduino.h>
#include <esp_now.h>
#include <WiFi.h>

const int trigPins[] = {5, 14, 26, 27, 22};
const int echoPins[] = {18, 13, 25, 32, 23};
const int numSensors = sizeof(trigPins) / sizeof(trigPins[0]);
const int EMERG_PIN = 33; //Emergency stop pin for the Drive control

#define SOUND_SPEED 0.034

long durations[numSensors];
float distances[numSensors];

void setup() {
  Serial.begin(115200);
  
  for (int i = 0; i < numSensors; i++) {
    pinMode(trigPins[i], OUTPUT);
    pinMode(echoPins[i], INPUT);
  }

  pinMode(EMERG_PIN, OUTPUT); // Configure the emergency pin as an output
  digitalWrite(EMERG_PIN, LOW); // Ensure the emergency pin is initially low

  Serial.println("Setup complete. Sensors ready.");
}

void loop() {
  bool alertSent = false;

  for (int i = 0; i < numSensors; i++) {
    digitalWrite(trigPins[i], LOW);
    delayMicroseconds(2);
    digitalWrite(trigPins[i], HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPins[i], LOW);

    durations[i] = pulseIn(echoPins[i], HIGH);
    distances[i] = durations[i] * SOUND_SPEED / 2;

    if (distances[i] <= 20) {
      digitalWrite(EMERG_PIN, HIGH); // Set the emergency pin high
      alertSent = true;
      break;
    }
  }

  if (!alertSent) {
    digitalWrite(EMERG_PIN, LOW); // Ensure the alert pin is low if no alert is triggered
  }
  
  Serial.print("Distances (cm): ");
  for (int i = 0; i < numSensors; i++) {
    Serial.print(distances[i]);
    Serial.print(" ");
  }
  Serial.println();

  delay(100);
}
