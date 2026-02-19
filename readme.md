# 93LC46BT Microwire EEPROM Driver (Bit-Banged GPIO)

A lightweight, hardware-independent Microwire driver for the **Microchip 93LC46BT EEPROM**  
(64 × 16-bit organization mode).

This implementation uses **GPIO bit-banging** instead of a hardware SPI peripheral,  
making it suitable for low-level embedded systems and microcontrollers without native Microwire support.

---

## Device Configuration

This driver targets:

- Device: **93LC46BT**
- Organization: **64 words × 16 bits**
- Address width: **6 bits**
- Protocol: **Microwire (3-wire serial)**
- Data order: **MSB first**

> ⚠ If your EEPROM is configured in 8-bit organization mode, the driver must be modified accordingly.

---

##  Features

- ✔ Microwire 3-wire serial communication (CS / SK / DI / DO)
- ✔ MSB-first bit shifting
- ✔ 16-bit word read
- ✔ 16-bit word write
- ✔ Write Enable / Disable (EWEN / EWDS)
- ✔ Ready/Busy polling via DO
- ✔ Debug self-test routine

---

##  Hardware Interface

The driver uses four signal lines:

| EEPROM Pin | Description |
|------------|------------|
| CS         | Chip Select |
| SK         | Serial Clock |
| DI         | Data Input (MCU → EEPROM) |
| DO         | Data Output (EEPROM → MCU) |

## Software Setup

1) Please modify GPIO of (CS / SK /DI) based on your MCU type

/* Serial Clock */

static inline void EE_SK_LOW(void)  { DL_GPIO_clearPins(GPIO_PORTA_PORT, IO_SCK); }
static inline void EE_SK_HIGH(void) { DL_GPIO_setPins  (GPIO_PORTA_PORT, IO_SCK); }

/* Serial Data Input (MCU -> EEPROM) */

static inline void EE_DI_LOW(void)  { DL_GPIO_clearPins(GPIO_PORTB_PORT, IO_SDI); }
static inline void EE_DI_HIGH(void) { DL_GPIO_setPins  (GPIO_PORTB_PORT, IO_SDI); }

/* Chip Select */

static inline void EE_CS_LOW(void)  { DL_GPIO_clearPins(GPIO_CS_PORT, EEPROM_CS); }
static inline void EE_CS_HIGH(void) { DL_GPIO_setPins  (GPIO_CS_PORT, EEPROM_CS); } 

2) Add EEP_WriteReadTest() function to main for eeprom testing
   
---

## Timing Consideration
The current implementation uses a simple software delay: 
static inline void ee_delay(void)

⚠ This is intended for testing only.

For production use:
- Replace with a hardware timer delay
- use cycle-accurate delay (DWT, SysTick, etc.)
- Ensure Microwire timing requirements from datasheet are satisfied




