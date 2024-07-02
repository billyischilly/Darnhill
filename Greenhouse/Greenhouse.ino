#include <Adafruit_NeoPixel.h>
#include <BLEMIDI_Transport.h>
#include <hardware/BLEMIDI_ESP32_NimBLE.h>

#define SOLAR_PIN 3
#define LED_PIN 8
#define NUM_LEDS 1

const char deviceName[] = "Greenhouse";
BLEMIDI_CREATE_INSTANCE(deviceName, MIDI);
bool isConnected = false;
cinst int MIDI_CC = 22
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

void setup() {
  Serial.begin(115200);
  strip.begin();
  strip.show();  // Initialize all pixels to 'off'
  strip.setBrightness(255);

  pinMode(SOLAR_PIN, INPUT);
  analogReadResolution(10);

  BLEMIDI.setHandleConnected([]() {
    isConnected = true;
    Serial.println("Bluetooth Connected");
    strip.setPixelColor(0, strip.Color(0, 0, 255));  // Blue for connected
    strip.show();
  });

  BLEMIDI.setHandleDisconnected([]() {
    isConnected = false;
    Serial.println("Bluetooth Disconnected");
    strip.setPixelColor(0, strip.Color(0, 0, 0));  // Off when disconnected
    strip.show();
  });

  MIDI.begin();
}

void loop() {
  while (!isConnected) {
    strip.setPixelColor(0, strip.Color(0, 0, 255));  // Blue for searching
    strip.show();
    delay(500);
    strip.setPixelColor(0, strip.Color(0, 0, 0));  // Off for searching
    strip.show();
    delay(500);
  }

  MIDI.read();

  int lightLevel = analogRead(SOLAR_PIN);
  int brightness = map(lightLevel, 0, 1023, 255, 0);  // Inverted mapping

  strip.setPixelColor(0, strip.Color(0, brightness, brightness/4));  // Green LED indicating light level
  strip.show();

  int midiValue = map(lightLevel, 0, 1023, 0, 127);
  MIDI.sendControlChange(MIDI_CC, midiValue, 1);
  Serial.println(midiValue);

  delay(100);
}
