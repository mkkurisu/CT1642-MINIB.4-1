/*****************************************************************************
**  Example:    ShowTime.ino                                                **
**  Updated by: mkkurisu                                                    **
**  Info:       Demonstrates displaying 24-hour time (HH:MM) with a         **
**              blinking center colon (:).                                  **
******************************************************************************/

#include <CT1642.h>

#define CLOCK_PIN 6
#define DATA_PIN  7

CT1642 disp(CLOCK_PIN, DATA_PIN);

int hours = 15;
int minutes = 59;
bool colon = true;

void setup() {
  // Starts Timer2 interrupt automatically -> safe to use delay() in loop!
  disp.begin(true); 
}

void loop() {
  // Display Time and toggle the center colon every second
  disp.showTime(hours, minutes, colon);
  colon = !colon;
  
  delay(1000);
}