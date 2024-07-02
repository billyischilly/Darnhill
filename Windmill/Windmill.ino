#include <Adafruit_NeoPixel.h>
#include <BLEMIDI_Transport.h>
#include <hardware/BLEMIDI_ESP32_NimBLE.h>

// Define the pin for the anemometer and the LED
#define ANEMOMETER_PIN 3
#define LED_PIN 8
#define NUM_LEDS 1

// Create an instance of the Adafruit NeoPixel library
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

// Bluetooth MIDI device name
const char deviceName[] = "Windmill";
bool isConnected = false;
const int MIDI_CC = 21;
const int CHANNEL = 1;

// Create an instance of the BLEMIDI
BLEMIDI_CREATE_INSTANCE(deviceName, MIDI);

void setup() {
  // Initialize serial communication for debugging
  Serial.begin(115200);

  // Initialize the LED strip
  strip.begin();
  strip.show();  // Initialize all pixels to 'off'
  strip.setBrightness(255);

  // Set the anemometer pin as input
  pinMode(ANEMOMETER_PIN, INPUT);
  analogReadResolution(10);

  // Set up BLE MIDI connection handlers
  BLEMIDI.setHandleConnected([]() {
    isConnected = true;
    Serial.println("Bluetooth Connected");
    Serial.println("___________________");
  });

  BLEMIDI.setHandleDisconnected([]() {
    isConnected = false;
    Serial.println("Bluetooth Disconnected");
    Serial.println("______________________");
  });

  // Start the MIDI service
  MIDI.begin();
}

void loop() {
  // Blink blue LED while waiting for a Bluetooth connection
  while (!isConnected) {
    strip.setPixelColor(0, strip.Color(0, 0, 255));  // Blue for not connected
    strip.show();
    delay(500);
    strip.setPixelColor(0, strip.Color(0, 0, 0));  // Off
    strip.show();
    delay(500);
  }

  // Read incoming MIDI messages (if any)
  MIDI.read();

  // Define thresholds and limits for the wind speed
  const int LOWER_THRESHOLD = 25;
  const int UPPER_LIMIT = 500;

  // Read and constrain the wind speed value
  int windSpeed = analogRead(ANEMOMETER_PIN);
  windSpeed = constrain(windSpeed, LOWER_THRESHOLD, UPPER_LIMIT);

  // Map wind speed to RGB values
  int blue = map(windSpeed, LOWER_THRESHOLD, UPPER_LIMIT, 0, 255);
  int green = map(windSpeed, 300, UPPER_LIMIT, 0, 255);
  green = max(green, 0); // Ensure green is not negative
  int red = map(windSpeed, 400, UPPER_LIMIT, 0, 255);
  red = max(red, 0); // Ensure red is not negative

  // Update the LED color based on wind speed
  strip.setPixelColor(0, strip.Color(red, green, blue));
  strip.show();

  // Map wind speed to MIDI CC values and send if changed
  static int previousReading = 0;
  int midiValue = map(windSpeed, LOWER_THRESHOLD, UPPER_LIMIT, 0, 127);
  if (midiValue != previousReading) {
    MIDI.sendControlChange(MIDI_CC, midiValue, CHANNEL);
    Serial.println(midiValue);
    previousReading = midiValue;
    delay(10);
  }
}
