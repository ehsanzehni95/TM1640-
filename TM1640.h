/*
 * TM1640.h - TM1640 LED Driver Library
 * 
 * A comprehensive, reusable library for controlling TM1640 LED driver IC
 * 
 * Features:
 * - Low-level protocol implementation (bit-banging)
 * - High-level display functions
 * - Support for common anode/cathode displays
 * - Buffer/inverter (ULN2003) support
 * - Configurable brightness levels
 * - Display buffer management
 * 
 * Hardware: TM1640 IC (16 GRID x 8 SEG)
 * 
 * Created on: Dec 22, 2025
 * Author: e.zehni
 */

#ifndef LIBRARY_TM1640_H_
#define LIBRARY_TM1640_H_

#include <stdbool.h>
#include <stdint.h>
#include "stm32f4xx_hal.h"

/* ============================================
 * Configuration - Edit TM1640_Config.h
 * ============================================ */
#include "TM1640_Config.h"

/* ============================================
 * Default GPIO Configuration
 * ============================================ */
#ifndef TM1640_CLK_Pin
#define TM1640_CLK_Pin        GPIO_PIN_1
#define TM1640_CLK_GPIO_Port  GPIOA
#endif

#ifndef TM1640_DIO_Pin
#define TM1640_DIO_Pin        GPIO_PIN_3
#define TM1640_DIO_GPIO_Port  GPIOA
#endif

/* ============================================
 * TM1640 Hardware Constants
 * ============================================ */
#define TM1640_TOTAL_GRIDS        16   // GRID1-16
#define TM1640_TOTAL_SEGMENTS     8    // SEG1-8

/* TM1640 Command Definitions */
#define TM1640_CMD_DATA           0x40  // Data command
#define TM1640_CMD_DISPLAY        0x80  // Display control
#define TM1640_CMD_ADDRESS        0xC0  // Address command

/* Data command options */
#define TM1640_DATA_WRITE         0x00  // Write mode
#define TM1640_DATA_AUTO_INC      0x00  // Auto increment
#define TM1640_DATA_FIXED         0x04  // Fixed address

/* Display control */
#define TM1640_DISPLAY_OFF        0x00
#define TM1640_DISPLAY_ON         0x08

/* ============================================
 * Segment Pattern Constants
 * ============================================ */
#define TM1640_SEG_A     0x01
#define TM1640_SEG_B     0x02
#define TM1640_SEG_C     0x04
#define TM1640_SEG_D     0x08
#define TM1640_SEG_E     0x10
#define TM1640_SEG_F     0x20
#define TM1640_SEG_G     0x40
#define TM1640_SEG_DP    0x80

/* ============================================
 * Brightness Levels
 * ============================================ */
typedef enum {
    TM1640_BRIGHTNESS_1_16 = 0,
    TM1640_BRIGHTNESS_2_16 = 1,
    TM1640_BRIGHTNESS_4_16 = 2,
    TM1640_BRIGHTNESS_10_16 = 3,
    TM1640_BRIGHTNESS_11_16 = 4,
    TM1640_BRIGHTNESS_12_16 = 5,
    TM1640_BRIGHTNESS_13_16 = 6,
    TM1640_BRIGHTNESS_14_16 = 7
} TM1640_Brightness_t;

/* ============================================
 * Initialization
 * ============================================ */

/** Initialize TM1640 driver */
void TM1640_Init(void);

/** De-initialize TM1640 driver */
void TM1640_DeInit(void);

/* ============================================
 * Display Control
 * ============================================ */

/** Set brightness (0-7) */
void TM1640_SetBrightness(TM1640_Brightness_t brightness);

/** Turn display ON/OFF */
void TM1640_DisplayOn(bool on);

/** Clear all segments */
void TM1640_Clear(void);

/* ============================================
 * Raw Data Access
 * ============================================ */

/** Write data to single grid address (0-15) */
void TM1640_WriteGrid(uint8_t address, uint8_t data);

/** Write array to multiple grids */
void TM1640_WriteGridArray(uint8_t start_address, const uint8_t *data, uint8_t length);

/** Get current buffer value for a grid */
uint8_t TM1640_GetBuffer(uint8_t address);

/** Set buffer value without writing to TM1640 */
void TM1640_SetBuffer(uint8_t address, uint8_t data);

/** Set/clear specific bit in buffer */
void TM1640_SetBufferBit(uint8_t address, uint8_t bit_mask, bool set);

/** Flush entire buffer to TM1640 */
void TM1640_Flush(void);

/* ============================================
 * Pattern Functions
 * ============================================ */

/** Get segment pattern for digit (0-9) */
uint8_t TM1640_GetDigitPattern(uint8_t digit);

/** Get segment pattern for hex (0-F) */
uint8_t TM1640_GetHexPattern(uint8_t hex);

/** Get segment pattern for character */
uint8_t TM1640_GetCharPattern(char ch);

/* ============================================
 * Utility
 * ============================================ */

/** Test: turn on all segments */
void TM1640_Test(void);

#endif /* LIBRARY_TM1640_H_ */
