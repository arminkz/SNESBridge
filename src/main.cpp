#include <Arduino.h>

#define CLOCK_PIN  5
#define LATCH_PIN  6
#define DATA_PIN   7

const char* buttonNames[] = {
  "B", "Y", "Select", "Start",
  "Up", "Down", "Left", "Right",
  "A", "X", "L", "R"
};

uint16_t readSNES() {
  uint16_t state = 0;

  // Latch pulse — snapshot button states
  digitalWrite(LATCH_PIN, HIGH);
  delayMicroseconds(12);
  digitalWrite(LATCH_PIN, LOW);
  delayMicroseconds(6);

  // Read 16 bits (12 buttons + 4 unused)
  for (int i = 0; i < 16; i++) {
    digitalWrite(CLOCK_PIN, LOW);
    delayMicroseconds(6);

    // Data is active LOW: 0 = pressed
    if (digitalRead(DATA_PIN) == LOW) {
      state |= (1 << i);
    }

    digitalWrite(CLOCK_PIN, HIGH);
    delayMicroseconds(6);
  }

  return state;
}

void setup() {
  Serial.begin(9600);
  pinMode(LATCH_PIN, OUTPUT);
  pinMode(CLOCK_PIN, OUTPUT);
  pinMode(DATA_PIN, INPUT);

  digitalWrite(LATCH_PIN, LOW);
  digitalWrite(CLOCK_PIN, HIGH);
}

void loop() {
  uint16_t raw = readSNES();

  // When the controller is unplugged, DATA sits LOW for all 16 clocks,
  // so every bit in `raw` gets set. A connected controller can't produce
  // 0xFFFF (would require all 12 buttons + 4 unused lines pressed at once).
  bool connected = raw != 0xFFFF;

  static bool wasConnected = true;
  if (connected != wasConnected) {
    Serial.println(connected ? "Controller connected" : "Controller disconnected");
    wasConnected = connected;
  }

  if (!connected) {
    delay(50);
    return;
  }

  uint16_t buttons = raw & 0x0FFF;
  for (int i = 0; i < 12; i++) {
    if (buttons & (1 << i)) {
      Serial.print(buttonNames[i]);
      Serial.print(" ");
    }
  }

  if (buttons != 0) Serial.println();

  delay(50); // ~20Hz polling
}