/*
 * TM1640.c - TM1640 LED Driver Library
 * 
 * A comprehensive, reusable library for controlling TM1640 LED driver IC
 * 
 * Created on: Dec 22, 2025
 * Author: e.zehni
 */

#include "TM1640_Config.h"
#include "TM1640.h"

/* ============================================
 * Segment Patterns
 * ============================================ */

/* Digits 0-9 */
static const uint8_t digit_patterns[10] = {
    0x3F, // 0: a,b,c,d,e,f
    0x06, // 1: b,c
    0x5B, // 2: a,b,d,e,g
    0x4F, // 3: a,b,c,d,g
    0x66, // 4: b,c,f,g
    0x6D, // 5: a,c,d,f,g
    0x7D, // 6: a,c,d,e,f,g
    0x07, // 7: a,b,c
    0x7F, // 8: a,b,c,d,e,f,g
    0x6F  // 9: a,b,c,d,f,g
};

/* Hex A-F */
static const uint8_t hex_patterns[6] = {
    0x77, // A
    0x7C, // b
    0x39, // C
    0x5E, // d
    0x79, // E
    0x71  // F
};

/* Characters A-Z */
static const uint8_t char_patterns[26] = {
    0x77, 0x7C, 0x39, 0x5E, 0x79, 0x71, 0x3D, 0x76,
    0x06, 0x1E, 0x76, 0x38, 0x37, 0x54, 0x3F, 0x73,
    0x67, 0x50, 0x6D, 0x78, 0x3E, 0x3E, 0x3E, 0x76,
    0x6E, 0x5B
};

/* ============================================
 * Display Buffer
 * ============================================ */

static uint8_t display_buffer[TM1640_TOTAL_GRIDS];
static TM1640_Brightness_t current_brightness = TM1640_BRIGHTNESS_14_16;
static bool display_on = true;

/* ============================================
 * GPIO Macros
 * ============================================ */

#ifdef TM1640_USE_BUFFER
/* Inverted for buffer/inverter (ULN2003) */
#define TM1640_CLK_LOW()   HAL_GPIO_WritePin(TM1640_CLK_GPIO_Port, TM1640_CLK_Pin, GPIO_PIN_SET)
#define TM1640_CLK_HIGH()  HAL_GPIO_WritePin(TM1640_CLK_GPIO_Port, TM1640_CLK_Pin, GPIO_PIN_RESET)
#define TM1640_DIO_LOW()   HAL_GPIO_WritePin(TM1640_DIO_GPIO_Port, TM1640_DIO_Pin, GPIO_PIN_SET)
#define TM1640_DIO_HIGH()  HAL_GPIO_WritePin(TM1640_DIO_GPIO_Port, TM1640_DIO_Pin, GPIO_PIN_RESET)
#else
/* Direct connection */
#define TM1640_CLK_LOW()   HAL_GPIO_WritePin(TM1640_CLK_GPIO_Port, TM1640_CLK_Pin, GPIO_PIN_RESET)
#define TM1640_CLK_HIGH()  HAL_GPIO_WritePin(TM1640_CLK_GPIO_Port, TM1640_CLK_Pin, GPIO_PIN_SET)
#define TM1640_DIO_LOW()   HAL_GPIO_WritePin(TM1640_DIO_GPIO_Port, TM1640_DIO_Pin, GPIO_PIN_RESET)
#define TM1640_DIO_HIGH()  HAL_GPIO_WritePin(TM1640_DIO_GPIO_Port, TM1640_DIO_Pin, GPIO_PIN_SET)
#endif

/* ============================================
 * Low-Level Protocol
 * ============================================ */

static void TM1640_Delay(void)
{
    volatile uint32_t delay = 50;
    while(delay--);
}

static void TM1640_Start(void)
{
    TM1640_DIO_HIGH();
    TM1640_CLK_HIGH();
    TM1640_Delay();
    TM1640_DIO_LOW();
    TM1640_Delay();
    TM1640_CLK_LOW();
    TM1640_Delay();
}

static void TM1640_Stop(void)
{
    TM1640_CLK_LOW();
    TM1640_DIO_LOW();
    TM1640_Delay();
    TM1640_CLK_HIGH();
    TM1640_Delay();
    TM1640_DIO_HIGH();
    TM1640_Delay();
}

static void TM1640_WriteByte(uint8_t data)
{
    for(uint8_t i = 0; i < 8; i++)
    {
        TM1640_CLK_LOW();
        TM1640_Delay();
        
        if(data & 0x01)
            TM1640_DIO_HIGH();
        else
            TM1640_DIO_LOW();
        
        TM1640_Delay();
        TM1640_CLK_HIGH();
        TM1640_Delay();
        
        data >>= 1;
    }
}

static void TM1640_SendCommand(uint8_t cmd)
{
    TM1640_Start();
    TM1640_WriteByte(cmd);
    TM1640_Stop();
}

static void TM1640_UpdateDisplayControl(void)
{
    uint8_t cmd = TM1640_CMD_DISPLAY;
    
    if(display_on)
        cmd |= TM1640_DISPLAY_ON | (current_brightness & 0x07);
    else
        cmd |= TM1640_DISPLAY_OFF;
    
    TM1640_SendCommand(cmd);
}

/* ============================================
 * Initialization
 * ============================================ */

void TM1640_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    // Enable GPIO clock
    if(TM1640_CLK_GPIO_Port == GPIOA) __HAL_RCC_GPIOA_CLK_ENABLE();
    else if(TM1640_CLK_GPIO_Port == GPIOB) __HAL_RCC_GPIOB_CLK_ENABLE();
    else if(TM1640_CLK_GPIO_Port == GPIOC) __HAL_RCC_GPIOC_CLK_ENABLE();
    else if(TM1640_CLK_GPIO_Port == GPIOD) __HAL_RCC_GPIOD_CLK_ENABLE();
    else if(TM1640_CLK_GPIO_Port == GPIOE) __HAL_RCC_GPIOE_CLK_ENABLE();
    
    // Configure CLK pin
    GPIO_InitStruct.Pin = TM1640_CLK_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(TM1640_CLK_GPIO_Port, &GPIO_InitStruct);
    
    // Configure DIO pin
    GPIO_InitStruct.Pin = TM1640_DIO_Pin;
    HAL_GPIO_Init(TM1640_DIO_GPIO_Port, &GPIO_InitStruct);
    
    // Set initial states
    TM1640_CLK_HIGH();
    TM1640_DIO_HIGH();
    
    // Clear buffer
    for(uint8_t i = 0; i < TM1640_TOTAL_GRIDS; i++)
        display_buffer[i] = 0x00;
    
    // Initialize TM1640
    TM1640_SendCommand(TM1640_CMD_DATA | TM1640_DATA_WRITE | TM1640_DATA_AUTO_INC);
    TM1640_Clear();
    
    // Set brightness and turn on
    current_brightness = TM1640_BRIGHTNESS_14_16;
    display_on = true;
    TM1640_UpdateDisplayControl();
}

void TM1640_DeInit(void)
{
    display_on = false;
    TM1640_UpdateDisplayControl();
    
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    
    GPIO_InitStruct.Pin = TM1640_CLK_Pin;
    HAL_GPIO_Init(TM1640_CLK_GPIO_Port, &GPIO_InitStruct);
    
    GPIO_InitStruct.Pin = TM1640_DIO_Pin;
    HAL_GPIO_Init(TM1640_DIO_GPIO_Port, &GPIO_InitStruct);
}

/* ============================================
 * Display Control
 * ============================================ */

void TM1640_SetBrightness(TM1640_Brightness_t brightness)
{
    current_brightness = brightness;
    TM1640_UpdateDisplayControl();
}

void TM1640_DisplayOn(bool on)
{
    display_on = on;
    TM1640_UpdateDisplayControl();
}

void TM1640_Clear(void)
{
    for(uint8_t i = 0; i < TM1640_TOTAL_GRIDS; i++)
        display_buffer[i] = 0x00;
    
    TM1640_SendCommand(TM1640_CMD_DATA | TM1640_DATA_WRITE | TM1640_DATA_AUTO_INC);
    
    TM1640_Start();
    TM1640_WriteByte(TM1640_CMD_ADDRESS | 0x00);
    
    for(uint8_t i = 0; i < TM1640_TOTAL_GRIDS; i++)
        TM1640_WriteByte(0x00);
    
    TM1640_Stop();
}

/* ============================================
 * Raw Data Access
 * ============================================ */

void TM1640_WriteGrid(uint8_t address, uint8_t data)
{
    if(address >= TM1640_TOTAL_GRIDS) return;
    
    display_buffer[address] = data;
    
    TM1640_SendCommand(TM1640_CMD_DATA | TM1640_DATA_WRITE | TM1640_DATA_FIXED);
    
    TM1640_Start();
    TM1640_WriteByte(TM1640_CMD_ADDRESS | address);
    TM1640_WriteByte(data);
    TM1640_Stop();
}

void TM1640_WriteGridArray(uint8_t start_address, const uint8_t *data, uint8_t length)
{
    if(start_address >= TM1640_TOTAL_GRIDS) return;
    if(start_address + length > TM1640_TOTAL_GRIDS)
        length = TM1640_TOTAL_GRIDS - start_address;
    
    for(uint8_t i = 0; i < length; i++)
        display_buffer[start_address + i] = data[i];
    
    TM1640_SendCommand(TM1640_CMD_DATA | TM1640_DATA_WRITE | TM1640_DATA_AUTO_INC);
    
    TM1640_Start();
    TM1640_WriteByte(TM1640_CMD_ADDRESS | start_address);
    
    for(uint8_t i = 0; i < length; i++)
        TM1640_WriteByte(data[i]);
    
    TM1640_Stop();
}

uint8_t TM1640_GetBuffer(uint8_t address)
{
    if(address >= TM1640_TOTAL_GRIDS) return 0;
    return display_buffer[address];
}

void TM1640_SetBuffer(uint8_t address, uint8_t data)
{
    if(address >= TM1640_TOTAL_GRIDS) return;
    display_buffer[address] = data;
}

void TM1640_SetBufferBit(uint8_t address, uint8_t bit_mask, bool set)
{
    if(address >= TM1640_TOTAL_GRIDS) return;
    
    if(set)
        display_buffer[address] |= bit_mask;
    else
        display_buffer[address] &= ~bit_mask;
}

void TM1640_Flush(void)
{
    TM1640_SendCommand(TM1640_CMD_DATA | TM1640_DATA_WRITE | TM1640_DATA_AUTO_INC);
    
    TM1640_Start();
    TM1640_WriteByte(TM1640_CMD_ADDRESS | 0x00);
    
    for(uint8_t i = 0; i < TM1640_TOTAL_GRIDS; i++)
        TM1640_WriteByte(display_buffer[i]);
    
    TM1640_Stop();
}

/* ============================================
 * Pattern Functions
 * ============================================ */

uint8_t TM1640_GetDigitPattern(uint8_t digit)
{
    if(digit > 9) digit = 9;
    return digit_patterns[digit];
}

uint8_t TM1640_GetHexPattern(uint8_t hex)
{
    if(hex < 10)
        return digit_patterns[hex];
    else if(hex < 16)
        return hex_patterns[hex - 10];
    return 0x00;
}

uint8_t TM1640_GetCharPattern(char ch)
{
    if(ch >= '0' && ch <= '9')
        return digit_patterns[ch - '0'];
    else if(ch >= 'A' && ch <= 'Z')
        return char_patterns[ch - 'A'];
    else if(ch >= 'a' && ch <= 'z')
        return char_patterns[ch - 'a'];
    else if(ch == '-')
        return 0x40;
    else if(ch == '_')
        return 0x08;
    return 0x00;
}

/* ============================================
 * Utility
 * ============================================ */

void TM1640_Test(void)
{
    for(uint8_t i = 0; i < TM1640_TOTAL_GRIDS; i++)
        display_buffer[i] = 0xFF;
    
    TM1640_Flush();
}
