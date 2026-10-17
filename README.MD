# CT1642 Arduino Library (Interrupt-Driven & Non-Blocking)

An optimized, non-blocking Arduino library for **CT1642 / SM1642** 18-bit shift-register LED display driver ICs, commonly found in compact SCART DVB-T/Satellite Set-Top Box front panels (specifically reverse-engineered and tested on board **`S-CT1642-MINIB.4-1`**).

> **Maintained & Refactored by:** [@mkkurisu](https://github.com/mkkurisu)  
> **Original Library Concept by:** George Efstathiou ([@arkroan](https://github.com/arkroan/CT1642))

---

## ✨ Key Improvements in v2.0 (by mkkurisu)
* **Timer2 Interrupt Driven (`delay()` Safe):** Display multiplexing is handled automatically in the background via Hardware Timer2 ISR (on ATmega328P/168/2560). You can freely use `delay()` or slow blocking sensors in your `loop()` without any display flickering!
* **True Datasheet Latching:** Implements the exact two-step rising-edge `DATA` latch and output enable waveform specified in the Shenzhen Coretech CT1642 datasheet (completely eliminates ghosting).
* **Full Character Set:** Supports `0-9`, `A-Z` approximations (including `X`, `M`, `H`, `C`, etc.), negative numbers, and the degree symbol (`*` -> `°`).
* **Bi-Color (Red/Green) LED, Colon (`:`), & Standby Mode Support:** Full control over the center colon and front-panel status LEDs.

---

## 🔌 Hardware Pinout (`S-CT1642-MINIB.4-1`)

### Segment Mapping (8-Bit Data Byte)
| Bit | Mask | Display Connection |
| :--- | :--- | :--- |
| **Bit 0 (MSB)** | `0x80` | Segment **A** (Top) |
| **Bit 1** | `0x40` | Segment **B** (Top Right) |
| **Bit 2** | `0x20` | Segment **C** (Bottom Right) |
| **Bit 3** | `0x10` | Segment **D** (Bottom) |
| **Bit 4** | `0x08` | Segment **E** (Bottom Left) |
| **Bit 5** | `0x04` | Segment **F** (Top Left) |
| **Bit 6** | `0x02` | Segment **G** (Middle) |
| **Bit 7 (LSB)** | `0x01` | **Colon (`:`)** (on Digit 2) & **Red LED** (on Digit 4) |

### Digit Mapping (4-Bit Common Cathode Address - Active LOW)
* **Bit 0:** Digit 4 (Rightmost) + **Red LED Cathode**
* **Bit 1:** Digit 3
* **Bit 2:** Digit 2 + **Colon (`:`) Cathode**
* **Bit 3:** Digit 1 (Leftmost) + **Green LED Cathode** *(Hardwired via 1kΩ resistor R4)*

> **Note on the Green LED:** On the `S-CT1642-MINIB.4-1` PCB, the Green LED is hardwired directly to the cathode of Digit 1. It lights up automatically whenever Digit 1 is multiplexed. Calling `disp.setGreenLed(false)` or `disp.setStandbyMode(true)` disables Digit 1 multiplexing to turn off the Green LED.

---

## 🚀 Quick Start & Example

```cpp
#include <CT1642.h>

#define CLOCK_PIN 6
#define DATA_PIN  7

CT1642 disp(CLOCK_PIN, DATA_PIN);

void setup() {
  // Pass 'true' (default) to enable Timer2 background refresh
  disp.begin(true);
}

void loop() {
  disp.showTime(15, 59, true); // Displays "15:59"
  delay(3000);                 // 100% safe to use delay()!

  disp.setColon(false);
  disp.showText("24*C");       // Displays "24°C" ('*' maps to degree symbol)
  delay(3000);

  disp.showText("bOX ");       // Displays "bOX "
  delay(3000);

  disp.showNumber(-42);        // Displays " -42"
  delay(3000);

  disp.setStandbyMode(true);   // Screen & Green LED OFF, Red LED ON
  delay(3000);
  disp.setStandbyMode(false);
}
```

---

## 📖 API Reference

| Method | Description |
| :--- | :--- |
| `disp.begin(bool useTimerInterrupt = true)` | Initializes pins. When `true`, uses Timer2 ISR so `disp.update()` is not needed. |
| `disp.update()` | Only call inside `loop()` if `disp.begin(false)` was used. |
| `disp.clear()` | Clears all digits and turns off the colon. |
| `disp.showNumber(int num, bool leadingZeros = false)` | Displays integers from `-999` to `9999`. |
| `disp.showTime(uint8_t hours, uint8_t minutes, bool colon = true)` | Displays 4-digit time (`HH:MM`). |
| `disp.showText(const char* text)` | Displays a 4-character string (use `*` for `°`). |
| `disp.setChar(uint8_t digitIndex, char c)` | Sets a single character at `digitIndex` (`0` = Leftmost, `3` = Rightmost). |
| `disp.setRaw(uint8_t digitIndex, uint8_t rawByte)` | Writes raw segment bits to a specific digit. |
| `disp.setColon(bool state)` | Turns the center colon (`:`) ON or OFF. |
| `disp.setRedLed(bool state)` | Turns the Red status LED ON or OFF. |
| `disp.setGreenLed(bool state)` | Enables/disables Digit 1 cathode to control the hardwired Green LED. |
| `disp.setStandbyMode(bool standby)` | Blanks the screen, turns OFF Green LED, and turns ON Red LED. |

---

## 🛠️ Diagnostic Tool (How to Adapt for Different Front Panels)

Different DVB-T/Satellite box manufacturers wire the CT1642 outputs (`Q2-Q9` and `C1-C4`) in different orders. If your display shows scrambled characters, upload the standalone diagnostic sketch below and open the **Serial Monitor (9600 baud)** to discover your board's exact bit mapping, then update `SEG_A..SEG_G` in `CT1642.h`:

```cpp
#define CLK_PIN  6
#define DATA_PIN 7

void setup() {
  pinMode(CLK_PIN, OUTPUT);
  pinMode(DATA_PIN, OUTPUT);
  Serial.begin(9600);
}

void sendCT1642(byte digitMask, byte segData) {
  for (int i = 0; i < 4; i++) {
    digitalWrite(CLK_PIN, LOW);
    digitalWrite(DATA_PIN, (digitMask & (0x08 >> i)) ? HIGH : LOW);
    digitalWrite(CLK_PIN, HIGH);
  }
  for (int i = 0; i < 6; i++) {
    digitalWrite(CLK_PIN, LOW);
    digitalWrite(DATA_PIN, LOW);
    digitalWrite(CLK_PIN, HIGH);
  }
  for (int i = 0; i < 8; i++) {
    digitalWrite(CLK_PIN, LOW);
    digitalWrite(DATA_PIN, (segData & (0x80 >> i)) ? HIGH : LOW);
    digitalWrite(CLK_PIN, HIGH);
  }
  digitalWrite(CLK_PIN, HIGH);
  digitalWrite(DATA_PIN, LOW);
  digitalWrite(DATA_PIN, HIGH);
  digitalWrite(CLK_PIN, LOW);
  digitalWrite(DATA_PIN, LOW);
  digitalWrite(DATA_PIN, HIGH);
}

void loop() {
  // 1. Test Segments (0 to 7) across all digits
  for (int bit = 0; bit < 8; bit++) {
    Serial.print("Testing Segment Bit: ");
    Serial.println(bit);
    unsigned long start = millis();
    while (millis() - start < 2500) {
      sendCT1642(0x00, (1 << (7 - bit)));
      delay(5);
    }
  }
  // 2. Test Digits (0 to 3)
  for (int d = 0; d < 4; d++) {
    Serial.print("Testing Digit Bit: ");
    Serial.println(d);
    byte digitMask = ~(1 << (3 - d)) & 0x0F;
    unsigned long start = millis();
    while (millis() - start < 2500) {
      sendCT1642(digitMask, 0xFF);
      delay(5);
    }
  }
}
```

---

## 📜 License & Credits
* **Original Library (v0.0.1):** Copyright (C) 2015 George Efstathiou (`arkroan`)
* **Refactored & Interrupt-Driven Release (v2.0.0):** Copyright (C) 2026 `mkkurisu`
* Licensed under the **GNU General Public License v3.0 (GPL-3.0)**.
