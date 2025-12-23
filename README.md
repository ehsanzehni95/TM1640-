# TM1640 LED Driver Library

A comprehensive, reusable library for controlling TM1640 LED driver IC on STM32 microcontrollers.

## Features

- **Low-level protocol** - Bit-banging communication
- **High-level functions** - Easy digit and character display
- **Buffer management** - Efficient display updates
- **Flexible configuration** - Common anode/cathode, ULN2003 support
- **8 brightness levels** - 1/16 to 14/16 duty cycle
- **Portable** - Easy to use in different projects

## Hardware

- TM1640 IC (16 GRID × 8 SEG = 128 LEDs)
- STM32 microcontroller with HAL library
- Two GPIO pins (CLK, DIO)
- Optional: ULN2003 buffer/inverter

## Files

```
Library/TM1640/
├── TM1640.h          # Public API
├── TM1640.c          # Implementation
├── TM1640_Config.h   # Project-specific configuration
└── README.md         # This file
```

## Quick Start

### 1. Configure `TM1640_Config.h`

```c
// Set to 1 if using ULN2003 or other inverter
#define TM1640_USE_BUFFER    1

// Set to 1 for Common Anode displays
#define TM1640_COMMON_ANODE  1
```

### 2. Define GPIO pins (before including TM1640.h)

```c
#define TM1640_CLK_Pin        GPIO_PIN_1
#define TM1640_CLK_GPIO_Port  GPIOA
#define TM1640_DIO_Pin        GPIO_PIN_3
#define TM1640_DIO_GPIO_Port  GPIOA

#include "TM1640.h"
```

### 3. Initialize and use

```c
// Initialize
TM1640_Init();
TM1640_SetBrightness(TM1640_BRIGHTNESS_14_16);

// Write to specific grid
TM1640_WriteGrid(0, 0x3F);  // Display "0" on GRID1

// Or use buffer for multiple updates
TM1640_SetBufferBit(0, 0x01, true);  // Set bit
TM1640_SetBufferBit(1, 0x02, true);  // Set bit
TM1640_Flush();                       // Write all at once
```

## API Reference

### Initialization

| Function | Description |
|----------|-------------|
| `TM1640_Init()` | Initialize driver |
| `TM1640_DeInit()` | De-initialize driver |

### Display Control

| Function | Description |
|----------|-------------|
| `TM1640_SetBrightness(brightness)` | Set brightness (0-7) |
| `TM1640_DisplayOn(on)` | Turn display ON/OFF |
| `TM1640_Clear()` | Clear all segments |

### Raw Data Access

| Function | Description |
|----------|-------------|
| `TM1640_WriteGrid(address, data)` | Write to single grid (0-15) |
| `TM1640_WriteGridArray(start, data, length)` | Write to multiple grids |
| `TM1640_GetBuffer(address)` | Get buffer value |
| `TM1640_SetBuffer(address, data)` | Set buffer value |
| `TM1640_SetBufferBit(address, mask, set)` | Set/clear buffer bit |
| `TM1640_Flush()` | Write buffer to TM1640 |

### Pattern Functions

| Function | Description |
|----------|-------------|
| `TM1640_GetDigitPattern(digit)` | Get pattern for 0-9 |
| `TM1640_GetHexPattern(hex)` | Get pattern for 0-F |
| `TM1640_GetCharPattern(ch)` | Get pattern for A-Z, 0-9 |

### Utility

| Function | Description |
|----------|-------------|
| `TM1640_Test()` | Turn on all segments |

## Segment Patterns

```
     a
    ---
 f |   | b
    -g-
 e |   | c
    ---
     d   .dp
```

Constants:
- `TM1640_SEG_A` (0x01) to `TM1640_SEG_G` (0x40)
- `TM1640_SEG_DP` (0x80)

## Brightness Levels

| Constant | Duty Cycle |
|----------|------------|
| `TM1640_BRIGHTNESS_1_16` | 1/16 (min) |
| `TM1640_BRIGHTNESS_2_16` | 2/16 |
| `TM1640_BRIGHTNESS_4_16` | 4/16 |
| `TM1640_BRIGHTNESS_10_16` | 10/16 |
| `TM1640_BRIGHTNESS_11_16` | 11/16 |
| `TM1640_BRIGHTNESS_12_16` | 12/16 |
| `TM1640_BRIGHTNESS_13_16` | 13/16 |
| `TM1640_BRIGHTNESS_14_16` | 14/16 (max) |

## Configuration Options

### `TM1640_USE_BUFFER`

Define this if CLK/DIO signals pass through ULN2003 or other inverter:

```c
#define TM1640_USE_BUFFER    1
```

This inverts the GPIO logic so MCU HIGH → TM1640 LOW.

### `TM1640_COMMON_ANODE`

Define for common anode displays:

```c
#define TM1640_COMMON_ANODE  1
```

**Note:** When using ULN2003 with common anode displays, the double inversion results in normal logic (bit=1 → segment ON).

## Example: Project-Specific Wrapper

For complex displays, create a wrapper in your project:

```c
// MyDisplay.c

#include "TM1640.h"

// Hardware mapping for your display
#define DIGIT1_BIT  0x01
#define DIGIT2_BIT  0x02

void MyDisplay_ShowNumber(uint16_t num)
{
    uint8_t d1 = num % 10;
    uint8_t d2 = (num / 10) % 10;
    
    // Update buffer
    for(int i = 0; i < 8; i++)
    {
        uint8_t pattern1 = TM1640_GetDigitPattern(d1);
        uint8_t pattern2 = TM1640_GetDigitPattern(d2);
        
        TM1640_SetBufferBit(i, DIGIT1_BIT, pattern1 & (1 << i));
        TM1640_SetBufferBit(i, DIGIT2_BIT, pattern2 & (1 << i));
    }
    
    TM1640_Flush();
}
```

## Troubleshooting

### Display not lighting up

1. Check GPIO configuration
2. Check `TM1640_USE_BUFFER` setting
3. Verify CLK/DIO connections
4. Run `TM1640_Test()` to test hardware

### Wrong segments

1. Check segment-to-GRID wiring
2. Verify `TM1640_COMMON_ANODE` setting

## License

MIT License - Free to use in any project.

## Author

Created by e.zehni - Dec 2025
