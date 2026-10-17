#include <CT1642.h>

#define CLOCK_PIN 6
#define DATA_PIN  7

CT1642 disp(CLOCK_PIN, DATA_PIN);

void setup() {
  // Starts Timer2 interrupt automatically -> safe to use delay() in loop!
  disp.begin(true); 
}

void loop() {
  // 1. Show Time (15:59) with blinking colon
  disp.setRedLed(false);
  for (int i = 0; i < 4; i++) {
    disp.showTime(15, 59, i % 2 == 0);
    delay(1000);
  }

  // 2. Show Temperature (24°C)
  disp.setColon(false);
  disp.showText("24*C");
  delay(3000);

  // 3. Show Text with X ("bOX ")
  disp.showText("bOX ");
  delay(3000);

  // 4. Show Integer Numbers
  disp.showNumber(-128);
  delay(3000);

  // 5. Standby Mode (Screen & Green LED OFF, Red LED ON)
  disp.setStandbyMode(true);
  delay(3000);
  disp.setStandbyMode(false);
}