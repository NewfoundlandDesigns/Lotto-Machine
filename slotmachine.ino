 #include <Adafruit_NeoPixel.h>
#include <WiFi.h>
#include <SoftwareSerial.h>
#include "ESP32_NOW.h"
#include <esp_mac.h>  // For the MAC2STR and MACSTR macros

#define BUTTON_PIN 25
#define BUTTON_LED_PIN 33
#define LED_STRIP_PIN 18
#define NUM_LEDS 50

#define ESPNOW_WIFI_CHANNEL 6

// Setup MP3 Serial connection
SoftwareSerial mp3Serial(16, 17); // RX, TX

Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LEDS, LED_STRIP_PIN, NEO_GRB + NEO_KHZ800);

bool isPlaying = false;
uint32_t msg_count = 0;  // Message count for ESP-NOW

// Creating a broadcast peer object for ESP-NOW
class ESP_NOW_Broadcast_Peer : public ESP_NOW_Peer {
public:
  ESP_NOW_Broadcast_Peer(uint8_t channel, wifi_interface_t iface, const uint8_t *lmk) : ESP_NOW_Peer(ESP_NOW.BROADCAST_ADDR, channel, iface, lmk) {}
  ~ESP_NOW_Broadcast_Peer() { remove(); }
  
  bool begin() {
    if (!ESP_NOW.begin() || !add()) {
      log_e("Failed to initialize ESP-NOW or register the broadcast peer");
      return false;
    }
    return true;
  }

  bool send_message(const uint8_t *data, size_t len) {
    if (!send(data, len)) {
      log_e("Failed to broadcast message");
      return false;
    }
    return true;
  }
};

ESP_NOW_Broadcast_Peer broadcast_peer(ESPNOW_WIFI_CHANNEL, WIFI_IF_STA, NULL);

void playMP3(int trackNumber) {
  mp3Serial.write((uint8_t)0x7E);   // Start byte
  mp3Serial.write((uint8_t)0xFF);   // Version
  mp3Serial.write((uint8_t)0x06);   // Length
  mp3Serial.write((uint8_t)0x03);   // Command: Play
  mp3Serial.write((uint8_t)0x00);   // Feedback
  mp3Serial.write((uint8_t)0x00);   // Track high byte
  mp3Serial.write((uint8_t)trackNumber); // Track low byte (1-255)
  mp3Serial.write((uint8_t)0xEF);   // End byte
}

void flashStrip() {
  unsigned long startTime = millis();
  unsigned long duration = 8000;  // Flash for 8 seconds

  while (millis() - startTime < duration) {
    // Turn all LEDs green
    for (int j = 0; j < strip.numPixels(); j++) {
      strip.setPixelColor(j, strip.Color(0, 255, 0));  // Green
    }
    strip.show();
    delay(250);  // Keep lights on for 250ms

    // Turn all LEDs off
    for (int j = 0; j < strip.numPixels(); j++) {
      strip.setPixelColor(j, strip.Color(0, 0, 0));  // Off
    }
    strip.show();
    delay(250);  // Keep lights off for 250ms
  }
}

void setup() {
  Serial.begin(115200);

  // Setup button and LED
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(BUTTON_LED_PIN, OUTPUT);
  digitalWrite(BUTTON_LED_PIN, HIGH); // Button LED on by default

  // Setup LED strip
  strip.begin();
  strip.show();  // Initialize all pixels to 'off'

  // Setup MP3 player
  mp3Serial.begin(9600);
  delay(1000);  // Give some time for the MP3 module to initialize
  playMP3(1);   // Play MP3 at startup

  // Initialize Wi-Fi and ESP-NOW
  WiFi.mode(WIFI_STA);
  WiFi.setChannel(ESPNOW_WIFI_CHANNEL);
  while (!WiFi.STA.started()) {
    delay(100);
  }
  
  Serial.println("ESP-NOW Broadcast Initialized");
  Serial.println("Wi-Fi parameters:");
  Serial.println("  Mode: STA");
  Serial.println("  MAC Address: " + WiFi.macAddress());
  Serial.printf("  Channel: %d\n", ESPNOW_WIFI_CHANNEL);

  // Register the broadcast peer for ESP-NOW
  if (!broadcast_peer.begin()) {
    Serial.println("Failed to initialize ESP-NOW broadcast peer");
    Serial.println("Rebooting in 5 seconds...");
    delay(5000);
    ESP.restart();
  }

  Serial.println("Setup complete.");
}

void loop() {
  if (digitalRead(BUTTON_PIN) == LOW && !isPlaying) {
    isPlaying = true;

    // Send message immediately when button is pressed
    char data[32];
    snprintf(data, sizeof(data), "Hello, World! #%lu", msg_count++);
    Serial.printf("Broadcasting message: %s\n", data);
    if (!broadcast_peer.send_message((uint8_t *)data, sizeof(data))) {
      Serial.println("Failed to broadcast message");
    }

    // Turn off button LED
    digitalWrite(BUTTON_LED_PIN, LOW);
    
    // Play MP3 track
    playMP3(1);

    // Flash LED strip in green for 8 seconds
    flashStrip();

    // Turn button LED back on after MP3 and light flashing
    digitalWrite(BUTTON_LED_PIN, HIGH);

    isPlaying = false;
  }
}
