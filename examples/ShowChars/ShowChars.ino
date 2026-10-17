/*****************************************************************************
**  Example:    ShowChars.ino                                               **
**  Updated by: mkkurisu                                                    **
**  Info:       Demonstrates displaying text, temperature degree symbol,    **
**              and individual characters on the 4-digit display.           **
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
  // Method 1: Easy string display using showText()
  disp.showText("Err1");
  delay(2500);

  // Display temperature using '*' for the degree (°) symbol
  disp.showText("24*C");
  delay(2500);

  // Display words including 'X'
  disp.showText("bOX ");
  delay(2500);

  // Method 2: Legacy character-by-character display using showChars()
  disp.showChars('C', 'o', 'o', 'L');
  delay(2500);
}