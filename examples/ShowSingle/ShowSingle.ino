/*****************************************************************************
**  Example:    ShowSingle.ino                                              **
**  Updated by: mkkurisu                                                    **
**  Info:       Demonstrates lighting up a single digit at a time           **
**              from left (Digit 1) to right (Digit 4).                     **
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
  // showSingle(number, digitPosition) -> digitPosition: 1 (Left) to 4 (Right)
  disp.showSingle(1, 1);
  delay(1000);

  disp.showSingle(2, 2);
  delay(1000);

  disp.showSingle(3, 3);
  delay(1000);

  disp.showSingle(4, 4);
  delay(1000);
}