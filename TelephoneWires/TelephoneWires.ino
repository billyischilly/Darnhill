#include <Wire.h>
#include <Adafruit_MPR121.h>
#include <BLEMIDI_Transport.h>
#include <hardware/BLEMIDI_ESP32_NimBLE.h>

#define SDA_PIN 8
#define SCL_PIN 9

// MIDI settings
const int midiNoteBase = 36;  // Base note number for touch inputs

// Create MPR121 object
Adafruit_MPR121 cap = Adafruit_MPR121();

// BLE MIDI settings
const char deviceName[] = "Telephone Wires";
BLEMIDI_CREATE_INSTANCE(deviceName, MIDI);
bool isConnected = false;

bool touched[12] = { false };  // Store touch status for 12 electrodes

void setup() {
  Serial.begin(115200);
  Wire.begin(SDA_PIN, SCL_PIN);
  Wire.setClock(10000);
  delay(100);

  // pinMode(SDA_PIN, INPUT);
  // pinMode(SCL_PIN, INPUT);
  // Set I2C pins and initialize Wire library

  Serial.println("HELLO! I HAVE BEGUN!");

  // Initialize MPR121
  if (!cap.begin(0x5A)) {
    Serial.println("MPR121 not found, check wiring!");
    while (1)
      ;
  }
  Serial.println("MPR121 found!");

  // Configure MPR121 settings
  cap.writeRegister(MPR121_ECR, 0x00);  // Stop MPR121

  // Set touch and release thresholds for each electrode

  cap.setThreshholds(12, 6);  // Adjust these values to your needs


  // Set MPR121 baseline tracking off
  cap.writeRegister(MPR121_MHDR, 0x01);
  cap.writeRegister(MPR121_NHDR, 0x01);
  cap.writeRegister(MPR121_NCLR, 0x0E);
  cap.writeRegister(MPR121_FDLR, 0x00);
  cap.writeRegister(MPR121_MHDF, 0x01);
  cap.writeRegister(MPR121_NHDF, 0x05);
  cap.writeRegister(MPR121_NCLF, 0x01);
  cap.writeRegister(MPR121_FDLF, 0x00);
  cap.writeRegister(MPR121_NHDT, 0x00);
  cap.writeRegister(MPR121_NCLT, 0x00);
  cap.writeRegister(MPR121_FDLT, 0x00);

  // Enable electrodes 0 to 11 and set to run mode
  cap.writeRegister(MPR121_ECR, 0x8F);

  // Initialize BLE MIDI
  MIDI.begin();
  Serial.println("Bluetooth MIDI initialized");

  BLEMIDI.setHandleConnected([]() {
    isConnected = true;
    Serial.println("Bluetooth Connected");
  });

  BLEMIDI.setHandleDisconnected([]() {
    isConnected = false;
    Serial.println("Bluetooth Disconnected");
  });
}

void loop() {
  MIDI.read();

  if (isConnected) {
    // Read touch status
    uint16_t currtouched = cap.touched();

    // Iterate through all touch electrodes
    for (uint8_t i = 0; i < 12; i++) {
      // If the touch status has changed
      if ((currtouched & (1 << i)) && !touched[i]) {
        // Electrode i was just touched
        Serial.print("Touch detected on electrode: ");
        Serial.println(i);
        MIDI.sendNoteOn(midiNoteBase + i, 127, 1);
        touched[i] = true;
      }
      if (!(currtouched & (1 << i)) && touched[i]) {
        // Electrode i was just released
        Serial.print("Touch released on electrode: ");
        Serial.println(i);
        MIDI.sendNoteOff(midiNoteBase + i, 0, 1);
        touched[i] = false;
      }

      // If electrode i is being touched, send aftertouch data
      if (currtouched & (1 << i)) {
        uint16_t filteredData = cap.filteredData(i);  // Read the filtered data
        Serial.println(filteredData);
        uint8_t aftertouchValue = constrain(map(filteredData, 250, 50, 0, 127), 0, 127);  // Map and constrain the value
        MIDI.sendPolyPressure(midiNoteBase + i, aftertouchValue, 1);                      // Send Polyphonic Key Pressure
      }
    }
  }

  delay(10);  // Small delay to avoid overwhelming the sensor
}
