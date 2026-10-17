/*****************************************************************************
**  Device:     CT1642 / SM1642 LED Display Driver & Key Scanner            **
**  Board:      S-CT1642-MINIB.4-1 (DVB-T / Set-Top Box Front Panel)        **
**                                                                          **
**  Original Library (v0.0.1 - 2015):                                       **
**      George Efstathiou (https://github.com/arkroan/CT1642)               **
**                                                                          **
**  Refactored, Optimized & Hardware-Mapped (v2.0.0 - 2026) by:             **
**      mkkurisu (https://github.com/mkkurisu/CT1642)                       **
**                                                                          **
**  License:    GNU General Public License v3.0                             **
******************************************************************************/

#ifndef CT1642_H
#define CT1642_H

#include "Arduino.h"

// Segment Bit Mapping for S-CT1642-MINIB.4-1 (Bit 0 = MSB = 0x80)
#define SEG_A   0x80  // Bit 0: Top
#define SEG_B   0x40  // Bit 1: Top Right
#define SEG_C   0x20  // Bit 2: Bottom Right
#define SEG_D   0x10  // Bit 3: Bottom
#define SEG_E   0x08  // Bit 4: Bottom Left
#define SEG_F   0x04  // Bit 5: Top Left
#define SEG_G   0x02  // Bit 6: Middle
#define SEG_AUX 0x01  // Bit 7: Colon (:) on Digit 2 & Red LED on Digit 4

class CT1642 {
  public:
    CT1642(uint8_t clkPin, uint8_t dataPin);
    CT1642(uint8_t clkPin, uint8_t dataPin, uint8_t keyPin);

    // Initialize driver. Set useTimerInterrupt = true for automatic background refresh
    void begin(bool useTimerInterrupt = true);

    // Manual refresh (only needed in loop() if useTimerInterrupt is false)
    void update();

    // Primary Display Functions
    void clear();
    void showNumber(int number, bool leadingZeros = false);
    void showTime(uint8_t hours, uint8_t minutes, bool colon = true);
    void showText(const char* text);
    void setChar(uint8_t digitIndex, char c); // digitIndex: 0 (Left) to 3 (Right)
    void setRaw(uint8_t digitIndex, uint8_t rawByte);

    // Legacy Compatibility Functions (for older sketches)
    void showChars(char c1, char c2, char c3, char c4);
    void showSingle(int number, int digitNo); // digitNo: 1 (Left) to 4 (Right)
    void setPovDelay(int delayMs);

    // Indicator Controls
    void setColon(bool state);
    void setRedLed(bool state);
    void setGreenLed(bool state);      // Hardware linked to Digit 1 cathode
    void setStandbyMode(bool standby); // Turns off digits & Green LED, turns on Red LED

    // Key Scan Function (Safe to call with Timer2 active)
    int getKeyCode();

    // Internal ISR handler
    void _isrRefreshStep();

  private:
    uint8_t _clkPin;
    uint8_t _dataPin;
    int8_t  _keyPin;
    volatile uint8_t _buffer[4];
    volatile bool _colonState;
    volatile bool _redLedState;
    volatile bool _greenLedState;
    volatile bool _displayBlank;
    volatile bool _isScanningKey;
    volatile uint8_t _currentDigit;
    unsigned long _lastRefresh;
    unsigned int _refreshIntervalUs;

    void send18Bit(uint8_t digitMask, uint8_t segData);
    uint8_t encodeChar(char c);
};

#endif