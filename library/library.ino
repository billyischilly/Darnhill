#include <TFT_eSPI.h>
#include <SPI.h>
#include <DHT.h>
#include <BLEMIDI_Transport.h>
#include <hardware/BLEMIDI_ESP32_NimBLE.h>

// Pin definitions
#define DHTPIN 27
#define DHTTYPE DHT11

const char deviceName[] = "Weather System";
BLEMIDI_CREATE_INSTANCE(deviceName, MIDI);
bool isConnected = false;

TFT_eSPI tft = TFT_eSPI();
TFT_eSprite sprite = TFT_eSprite(&tft);

DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(115200);

  delay(100);
  Serial.println("Library");

  dht.begin();
  Serial.println("DHT sensor initialized");

  tft.init();
  tft.setRotation(1);

  sprite.createSprite(tft.width(), tft.height());
  sprite.setTextSize(2.8);

  MIDI.begin();
  Serial.println("Bluetooth MIDI initialized");

  BLEMIDI.setHandleConnected([]() {
    isConnected = true;
    Serial.println("Bluetooth Connected");
    tft.fillScreen(TFT_BLACK);
  });

  BLEMIDI.setHandleDisconnected([]() {
    isConnected = false;
    Serial.println("Bluetooth Disconnected");
    tft.fillScreen(TFT_BLACK);
  });
}

void loop() {
  MIDI.read();

  while (!isConnected) {
    sprite.fillSprite(TFT_BLACK); // Use fillSprite instead of fillScreen
    sprite.setCursor(0, 0);
    sprite.setTextColor(TFT_RED);
    sprite.println("WELCOME TO");
    sprite.println("DARNHILL FESTIVAL");
    sprite.println("-----------------");
    sprite.setTextColor(TFT_PINK);
    sprite.println();
    sprite.println("Connect me to");
    sprite.setTextColor(TFT_BLUE);
    sprite.println("Bluetooth :)");
    sprite.pushSprite(0, 0);
    delay(100);

    sprite.fillSprite(TFT_BLACK); // Clear sprite again
    sprite.setCursor(0, 0);
    sprite.setTextColor(TFT_RED);
    sprite.println("WELCOME TO");
    sprite.println("DARNHILL FESTIVAL");
    sprite.println("-----------------");
    sprite.println();
    sprite.setTextColor(TFT_PINK);
    sprite.println("Connect me to");
    sprite.setTextColor(TFT_CYAN);
    sprite.println("Bluetooth :)");
    sprite.pushSprite(0, 0);
    delay(100);
  }

  // Check if Bluetooth is connected
  if (isConnected) {
    // Read temperature and humidity
    float temperature = dht.readTemperature(true);
    float humidity = dht.readHumidity();

    // Display on TFT screen
    sprite.fillSprite(TFT_BLACK); // Clear sprite before drawing
    sprite.setTextSize(3);
    sprite.setCursor(0, 0);
    sprite.setTextColor(TFT_PINK);
    sprite.println("Temperature: ");
    sprite.setTextColor(TFT_YELLOW);
    sprite.print((int)temperature);
    sprite.println("F");
    sprite.println("");
    sprite.setTextColor(TFT_BLUE);
    sprite.println("Humidity: ");
    sprite.setTextColor(TFT_YELLOW);
    sprite.print((int)humidity);
    sprite.println(" %");
    sprite.pushSprite(0, 0);

    // Map values to MIDI range (adjust ranges as per your requirement)
    int midiValueTemp = map(temperature, 0, 35, 0, 127);  // Example range mapping
    int midiValueHum = map(humidity, 0, 100, 0, 127);     // Example range mapping

    // Send MIDI control change messages
    MIDI.sendControlChange(23, midiValueTemp, 1);  // Example MIDI CC number
    MIDI.sendControlChange(24, midiValueHum, 1);   // Example MIDI CC number

    // Print values to Serial Monitor
    Serial.print("Temperature: ");
    Serial.print(temperature);
    Serial.println("°C");
    Serial.print("Humidity: ");
    Serial.print(humidity);
    Serial.println(" %");
  }

  delay(20);  // Adjust delay as needed for your application
}
