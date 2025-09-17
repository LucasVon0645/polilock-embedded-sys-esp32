#include <Arduino.h>
#include "io_pins.hpp"
#include "sensors.hpp"
#include "rfid.hpp"
#include "servo_ctrl.hpp"
#include "cloud_blynk.hpp"
#include "telemetry.hpp"
#include "app_fsm.hpp"

// === Setup ===
// This function runs once after reset/power-up
void setup() {
  Serial.begin(115200);
  delay(300);  // give time for serial monitor to connect

  // Initialize subsystems (no logic, just hardware/services setup)
  io::begin();          // configure GPIOs, SPI, PWM
  sensors::begin();     // setup PIR / Hall (debounce or thresholds)
  rfid::begin();        // initialize MFRC522 (SPI)
  cloud::begin();       // connect Wi-Fi + Blynk (virtual pins)
  servo::begin();       // set initial servo position
  fsm::begin();         // initialize application state machine
  telemetry::begin();   // schedule periodic telemetry tasks
}

// === Main Loop ===
// This function runs repeatedly after setup()
// It acts as a cooperative scheduler
void loop() {
  cloud::run();         // keep Blynk/Wi-Fi alive
  sensors::poll();      // read PIR / Hall (polling)
  rfid::poll();         // check if new RFID card is present
  fsm::tick();          // run application logic (state machine)
  servo::update();      // refresh servo position if needed
  telemetry::tick();    // send periodic telemetry
}
