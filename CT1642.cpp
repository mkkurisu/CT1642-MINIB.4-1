/*****************************************************************************
**  Device:     CT1642 / SM1642 LED Display Driver & Key Scanner            **
**  Board:      S-CT1642-MINIB.4-1 (DVB-T / Set-Top Box Front Panel)        **
**                                                                          **
**  Original Author:  George Efstathiou (arkroan)                           **
**  Refactored by:    mkkurisu (https://github.com/mkkurisu)                **
**  License:          GNU General Public License v3.0                       **
******************************************************************************/

#include "CT1642.h"

static CT1642* _activeInstance = nullptr;

// Standard 0-9 Digit Lookup Table
static const uint8_t DIGIT_TABLE[10] = {
  SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F,         // 0
  SEG_B | SEG_C,                                         // 1
  SEG_A | SEG_B | SEG_D | SEG_E | SEG_G,                 // 2
  SEG_A | SEG_B | SEG_C | SEG_D | SEG_G,                 // 3
  SEG_B | SEG_C | SEG_F | SEG_G,                         // 4
  SEG_A | SEG_C | SEG_D | SEG_F | SEG_G,                 // 5
  SEG_A | SEG_C | SEG_D | SEG_E | SEG_F | SEG_G,         // 6
  SEG_A | SEG_B | SEG_C,                                 // 7
  SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F | SEG_G, // 8
  SEG_A | SEG_B | SEG_C | SEG_D | SEG_F | SEG_G          // 9
};

CT1642::CT1642(uint8_t clkPin, uint8_t dataPin) {
  _clkPin = clkPin;
  _dataPin = dataPin;
  _keyPin = -1;
  _colonState = false;
  _redLedState = false;
  _greenLedState = true;
  _displayBlank = false;
  _isScanningKey = false;
  _currentDigit = 0;
  _lastRefresh = 0;
  _refreshIntervalUs = 2000;
  for (int i = 0; i < 4; i++) _buffer[i] = 0x00;
}

CT1642::CT1642(uint8_t clkPin, uint8_t dataPin, uint8_t keyPin) : CT1642(clkPin, dataPin) {
  _keyPin = keyPin;
}

void CT1642::begin(bool useTimerInterrupt) {
  pinMode(_clkPin, OUTPUT);
  pinMode(_dataPin, OUTPUT);
  digitalWrite(_clkPin, LOW);
  digitalWrite(_dataPin, LOW);
  if (_keyPin >= 0) {
    pinMode(_keyPin, INPUT);
  }
  clear();

  if (useTimerInterrupt) {
    _activeInstance = this;
    #if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega168__) || defined(__AVR_ATmega2560__)
      cli();
      TCCR2A = (1 << WGM21);                        // CTC Mode
      TCCR2B = (1 << CS22) | (1 << CS21);           // Prescaler 256 (16MHz / 256 = 62.5kHz)
      OCR2A  = 124;                                 // ~2ms interval (500Hz step / 125Hz full refresh)
      TIMSK2 |= (1 << OCIE2A);                      // Enable Timer2 Compare Match A Interrupt
      sei();
    #endif
  }
}

#if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega168__) || defined(__AVR_ATmega2560__)
ISR(TIMER2_COMPA_vect) {
  if (_activeInstance != nullptr) {
    _activeInstance->_isrRefreshStep();
  }
}
#endif

void CT1642::clear() {
  for (int i = 0; i < 4; i++) _buffer[i] = 0x00;
  _colonState = false;
  send18Bit(0x0F, 0x00);
}

void CT1642::setPovDelay(int delayMs) {
  if (delayMs > 0) {
    _refreshIntervalUs = delayMs * 1000;
  }
}

void CT1642::setColon(bool state) {
  _colonState = state;
}

void CT1642::setRedLed(bool state) {
  _redLedState = state;
}

void CT1642::setGreenLed(bool state) {
  _greenLedState = state;
}

void CT1642::setStandbyMode(bool standby) {
  _displayBlank = standby;
  if (standby) {
    _greenLedState = false;
    _redLedState = true;
  } else {
    _greenLedState = true;
    _redLedState = false;
  }
}

void CT1642::setRaw(uint8_t digitIndex, uint8_t rawByte) {
  if (digitIndex < 4) {
    _buffer[digitIndex] = rawByte & 0xFE;
  }
}

void CT1642::setChar(uint8_t digitIndex, char c) {
  if (digitIndex < 4) {
    _buffer[digitIndex] = encodeChar(c);
  }
}

void CT1642::showChars(char c1, char c2, char c3, char c4) {
  _displayBlank = false;
  _buffer[0] = encodeChar(c1);
  _buffer[1] = encodeChar(c2);
  _buffer[2] = encodeChar(c3);
  _buffer[3] = encodeChar(c4);
}

void CT1642::showSingle(int number, int digitNo) {
  _displayBlank = false;
  for (int i = 0; i < 4; i++) _buffer[i] = 0x00;
  if (digitNo >= 1 && digitNo <= 4) {
    int num = (number >= 0 && number <= 9) ? number : 0;
    _buffer[digitNo - 1] = DIGIT_TABLE[num];
  }
}

void CT1642::showText(const char* text) {
  _displayBlank = false;
  for (int i = 0; i < 4; i++) {
    if (text[i] != '\0') {
      _buffer[i] = encodeChar(text[i]);
    } else {
      for (int j = i; j < 4; j++) _buffer[j] = 0x00;
      break;
    }
  }
}

void CT1642::showNumber(int number, bool leadingZeros) {
  _displayBlank = false;
  if (number < -999) number = -999;
  if (number > 9999) number = 9999;

  bool isNegative = (number < 0);
  if (isNegative) number = -number;

  for (int i = 3; i >= 0; i--) {
    if (number > 0 || i == 3 || leadingZeros) {
      _buffer[i] = DIGIT_TABLE[number % 10];
      number /= 10;
    } else {
      if (isNegative) {
        _buffer[i] = SEG_G;
        isNegative = false;
      } else {
        _buffer[i] = 0x00;
      }
    }
  }
}

void CT1642::showTime(uint8_t hours, uint8_t minutes, bool colon) {
  _displayBlank = false;
  _buffer[0] = DIGIT_TABLE[(hours / 10) % 10];
  _buffer[1] = DIGIT_TABLE[hours % 10];
  _buffer[2] = DIGIT_TABLE[(minutes / 10) % 10];
  _buffer[3] = DIGIT_TABLE[minutes % 10];
  _colonState = colon;
}

int CT1642::getKeyCode() {
  if (_keyPin < 0) return 0;

  _isScanningKey = true; // Pause ISR display refresh during key scan
  int keyCodeSum = 0;

  // Turn off all digits (0x0F) and scan segment lines Q9..Q2 one by one
  for (int i = 0; i < 8; i++) {
    send18Bit(0x0F, (1 << i));
    delayMicroseconds(250); // Fast settling delay (no blocking ms delay!)
    if (digitalRead(_keyPin) == HIGH) {
      keyCodeSum += (i + 1);
    }
  }

  _isScanningKey = false; // Resume ISR display refresh
  return keyCodeSum; // Returns 0 if no button is pressed, or 1..8
}

void CT1642::_isrRefreshStep() {
  if (_isScanningKey) return; // Do not interfere while getKeyCode() is running

  uint8_t segData = _displayBlank ? 0x00 : _buffer[_currentDigit];

  if (!_displayBlank && _currentDigit == 1 && _colonState) {
    segData |= SEG_AUX;
  }

  if (_currentDigit == 3 && _redLedState) {
    segData |= SEG_AUX;
  }

  uint8_t digitMask = ~(1 << _currentDigit) & 0x0F;

  if (_currentDigit == 0 && !_greenLedState) {
    digitMask = 0x0F;
  }

  send18Bit(digitMask, segData);
  _currentDigit = (_currentDigit + 1) % 4;
}

void CT1642::update() {
  if (_activeInstance != nullptr) return;
  if (micros() - _lastRefresh >= _refreshIntervalUs) {
    _lastRefresh = micros();
    _isrRefreshStep();
  }
}

void CT1642::send18Bit(uint8_t digitMask, uint8_t segData) {
  for (int i = 0; i < 4; i++) {
    digitalWrite(_clkPin, LOW);
    digitalWrite(_dataPin, (digitMask & (0x08 >> i)) ? HIGH : LOW);
    digitalWrite(_clkPin, HIGH);
  }
  for (int i = 0; i < 6; i++) {
    digitalWrite(_clkPin, LOW);
    digitalWrite(_dataPin, LOW);
    digitalWrite(_clkPin, HIGH);
  }
  for (int i = 0; i < 8; i++) {
    digitalWrite(_clkPin, LOW);
    digitalWrite(_dataPin, (segData & (0x80 >> i)) ? HIGH : LOW);
    digitalWrite(_clkPin, HIGH);
  }
  digitalWrite(_clkPin, HIGH);
  digitalWrite(_dataPin, LOW);
  digitalWrite(_dataPin, HIGH);

  digitalWrite(_clkPin, LOW);
  digitalWrite(_dataPin, LOW);
  digitalWrite(_dataPin, HIGH);
}

uint8_t CT1642::encodeChar(char c) {
  if (c >= '0' && c <= '9') return DIGIT_TABLE[c - '0'];
  switch (c) {
    case 'A': case 'a': return SEG_A | SEG_B | SEG_C | SEG_E | SEG_F | SEG_G;
    case 'B': case 'b': return SEG_C | SEG_D | SEG_E | SEG_F | SEG_G;
    case 'C':           return SEG_A | SEG_D | SEG_E | SEG_F;
    case 'c':           return SEG_D | SEG_E | SEG_G;
    case 'D': case 'd': return SEG_B | SEG_C | SEG_D | SEG_E | SEG_G;
    case 'E': case 'e': return SEG_A | SEG_D | SEG_E | SEG_F | SEG_G;
    case 'F': case 'f': return SEG_A | SEG_E | SEG_F | SEG_G;
    case 'G': case 'g': return SEG_A | SEG_B | SEG_C | SEG_D | SEG_F | SEG_G;
    case 'H':           return SEG_B | SEG_C | SEG_E | SEG_F | SEG_G;
    case 'h':           return SEG_C | SEG_E | SEG_F | SEG_G;
    case 'I':           return SEG_E | SEG_F;
    case 'i':           return SEG_C;
    case 'J': case 'j': return SEG_B | SEG_C | SEG_D | SEG_E;
    case 'K': case 'k': return SEG_B | SEG_C | SEG_E | SEG_F | SEG_G;
    case 'L': case 'l': return SEG_D | SEG_E | SEG_F;
    case 'M': case 'm': return SEG_A | SEG_C | SEG_E;
    case 'N': case 'n': return SEG_C | SEG_E | SEG_G;
    case 'O':           return SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F;
    case 'o':           return SEG_C | SEG_D | SEG_E | SEG_G;
    case 'P': case 'p': return SEG_A | SEG_B | SEG_E | SEG_F | SEG_G;
    case 'Q': case 'q': return SEG_A | SEG_B | SEG_C | SEG_F | SEG_G;
    case 'R': case 'r': return SEG_E | SEG_G;
    case 'S': case 's': return SEG_A | SEG_C | SEG_D | SEG_F | SEG_G;
    case 'T': case 't': return SEG_D | SEG_E | SEG_F | SEG_G;
    case 'U':           return SEG_B | SEG_C | SEG_D | SEG_E | SEG_F;
    case 'u':           return SEG_C | SEG_D | SEG_E;
    case 'V': case 'v': return SEG_C | SEG_D | SEG_E;
    case 'W': case 'w': return SEG_B | SEG_D | SEG_F;
    case 'X': case 'x': return SEG_B | SEG_C | SEG_E | SEG_F | SEG_G;
    case 'Y': case 'y': return SEG_B | SEG_C | SEG_D | SEG_F | SEG_G;
    case 'Z': case 'z': return SEG_A | SEG_B | SEG_D | SEG_E | SEG_G;
    case '-':           return SEG_G;
    case '_':           return SEG_D;
    case '=':           return SEG_D | SEG_G;
    case '*':           return SEG_A | SEG_B | SEG_F | SEG_G; // Degree (°) symbol
    case ' ': default:  return 0x00;
  }
}