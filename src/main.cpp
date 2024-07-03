#include <Arduino.h>
#include <esp_now.h>
#include <WiFi.h>

const int trigPins[] = {5, 14, 26, 27, 22};
const int echoPins[] = {18, 13, 25, 32, 23};
const int numSensors = sizeof(trigPins) / sizeof(trigPins[0]);
const int EMERG_PIN = 33; // Define an output pin for the emergency signal

#define SOUND_SPEED 0.034

long durations[numSensors];
float distances[numSensors];

uint8_t peerAddress[] = {0x08, 0xD1, 0xF9, 0xDC, 0xB6, 0x20};

void setup() {
  Serial.begin(115200);
  
  WiFi.mode(WIFI_STA);
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, peerAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    return;
  }
  
  for (int i = 0; i < numSensors; i++) {
    pinMode(trigPins[i], OUTPUT);
    pinMode(echoPins[i], INPUT);
  }

  pinMode(EMERG_PIN, OUTPUT); // Configure the emergency pin as an output
  digitalWrite(EMERG_PIN, LOW); // Ensure the emergency pin is initially low

  Serial.println("Setup complete. Sensors ready.");
}

void sendAlert() {
  const char *message = "Distance alert!";
  esp_err_t result = esp_now_send(peerAddress, (uint8_t *)message, strlen(message));
  
  if (result == ESP_OK) {
    Serial.println("Alert sent successfully");
  } else {
    Serial.printf("Error sending the data: %d\n", result);
  }
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
      sendAlert(); // Send the ESP-NOW message
      alertSent = true;
      break;
    }
  }

  if (!alertSent) {
    digitalWrite(EMERG_PIN, LOW); // Ensure the emergency pin is low if no emergency is triggered
  }
  
  Serial.print("Distances (cm): ");
  for (int i = 0; i < numSensors; i++) {
    Serial.print(distances[i]);
    Serial.print(" ");
  }
  Serial.println();

  delay(100);
}
