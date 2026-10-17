/*****************************************************************************
**  Example:    ScanKey.ino                                                 **
**  Updated by: mkkurisu                                                    **
**  Info:       Demonstrates reading tactile buttons connected to CT1642    **
**              while displaying the pressed button number on Digit 4.      **
******************************************************************************/

#include <CT1642.h>

#define CLOCK_PIN 6
#define DATA_PIN  7
#define KEY_PIN   3   // Must be an external interrupt pin (Pin 2 or 3 on Uno)

// Initialize CT1642 with Key Scan pin
CT1642 disp(CLOCK_PIN, DATA_PIN, KEY_PIN);

volatile bool btnTriggered = false;
volatile unsigned long lastInterruptTime = 0;
const unsigned long debounceDelayMs = 200;

int lastKeyPressed = 0;

void onKeyInterrupt() {
  unsigned long now = millis();
  if (now - lastInterruptTime >= debounceDelayMs) {
    btnTriggered = true;
    lastInterruptTime = now;
  }
}

void setup() {
  Serial.begin(115200);

  // Start background Timer2 display refresh
  disp.begin(true);
  disp.showSingle(0, 4);

  // Trigger interrupt when a button pulls KEY_PIN HIGH
  attachInterrupt(digitalPinToInterrupt(KEY_PIN), onKeyInterrupt, RISING);
}

void loop() {
  if (btnTriggered) {
    btnTriggered = false;

    // Safely scan keys outside of the interrupt routine
    int key = disp.getKeyCode();

    if (key > 0) {
      lastKeyPressed = key;
      disp.showSingle(lastKeyPressed, 4);

      Serial.print("Button press read: ");
      Serial.print(lastKeyPressed);
      Serial.print(", BIN: ");
      Serial.print(lastKeyPressed, BIN);
      Serial.print(", HEX: 0x");
      Serial.println(lastKeyPressed, HEX);
    }
  }
}