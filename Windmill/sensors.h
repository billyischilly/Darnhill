#include "DHT_Async.h"
#include <Smoothed.h>

class Sensor {
protected:
  unsigned long lastUpdate;
  unsigned long updateInterval;
public:

  Sensor(unsigned long interval)
    : lastUpdate(0), updateInterval(interval) {}
  virtual void update() = 0;
  virtual void begin() = 0;
  virtual void printToSerial() = 0;

  bool shouldUpdate() {
    unsigned long currentMillis = millis();
    if (currentMillis - lastUpdate >= updateInterval) {
      lastUpdate = currentMillis;
      return true;
    }
    return false;
  }
};

class WindSpeedSensor : public Sensor {
private:
  Smoothed<int> windSpeed;
  int pin;
  int wind;
  int bpm = 0;
  int quarterNoteValue = 0;
  int clockInterval = 0;
  long lastNote = 0;


public:
  WindSpeedSensor(int pin, unsigned long interval)
    : Sensor(interval), windSpeed(), pin(pin), wind(0) {
  }

  void update() override {
    if (shouldUpdate()) {
      windSpeed.add(analogRead(pin));
      wind = windSpeed.get();
    }
  }

  bool midiClock() {
    bpm = map(wind, 0, 1024, 0, 180);
    if (bpm > 0) {
      quarterNoteValue = 60000 / bpm;  // Calculate quarter note interval in milliseconds
      unsigned long currentMillis = millis();
      if (currentMillis - lastNote >= clockInterval) {
        lastNote = currentMillis;
        clockInterval =  quarterNoteValue/24;
        return true;
      }
    }
    return false;
  }

  void begin() override {
    windSpeed.begin(SMOOTHED_AVERAGE, 5);
  }

  void printToSerial() override {
    int max = 1500;
    int min = 0;
    Serial.print("Wind:");
    Serial.print(wind);
    Serial.print(",");
    Serial.print("Max:");
    Serial.print(max);
    Serial.print(",");
    Serial.print("Min:");
    Serial.println(min);
  }
};
