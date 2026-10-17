/*****************************************************************************
**  Example:    ShowNumbers.ino                                             **
**  Updated by: mkkurisu                                                    **
**  Info:       Demonstrates displaying positive/negative integers and      **
**              optional leading zeros on the 4-digit display.              **
******************************************************************************/

#include <CT1642.h>

#define CLOCK_PIN 6
#define DATA_PIN  7

CT1642 disp(CLOCK_PIN, DATA_PIN);

void setup() {
  // Starts Timer2 interrupt automatically -> safe to use delay() in loop!
  disp.begin(true);
}

void loop() {
  // 1. Show maximum positive integer (9999)
  disp.showNumber(9999);
  delay(2000);

  // 2. Show negative integer (-45)
  disp.showNumber(-45);
  delay(2000);

  // 3. Show number with leading zeros enabled (displays "0007")
  disp.showNumber(7, true);
  delay(2000);

  // 4. Fast counter demonstration
  for (int i = 0; i <= 100; i++) {
    disp.showNumber(i);
    delay(30);
  }
  delay(1000);
}