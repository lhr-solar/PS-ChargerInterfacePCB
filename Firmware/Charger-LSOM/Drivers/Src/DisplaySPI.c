#include "DisplaySPI.h"
#include "spi.h"
#include <string.h>
#include "stm32g4xx_hal.h"
#include "gpio.h"
#include "StatusLED.h"

enum { GLYPH_W = 5, GLYPH_H = 7, CELL_W = 6, CELL_H = 8 };


static const uint8_t font5x7[][GLYPH_W] = {
    {0x00,0x00,0x00,0x00,0x00}, /* 32 ' ' */
    {0x00,0x00,0x5F,0x00,0x00}, /* 33 '!' */
    {0x00,0x07,0x00,0x07,0x00}, /* 34 '"' */
    {0x14,0x7F,0x14,0x7F,0x14}, /* 35 '#' */
    {0x24,0x2A,0x7F,0x2A,0x12}, /* 36 '$' */
    {0x23,0x13,0x08,0x64,0x62}, /* 37 '%' */
    {0x36,0x49,0x55,0x22,0x50}, /* 38 '&' */
    {0x00,0x05,0x03,0x00,0x00}, /* 39 ''' */
    {0x00,0x1C,0x22,0x41,0x00}, /* 40 '(' */
    {0x00,0x41,0x22,0x1C,0x00}, /* 41 ')' */
    {0x08,0x2A,0x1C,0x2A,0x08}, /* 42 '*' */
    {0x08,0x08,0x3E,0x08,0x08}, /* 43 '+' */
    {0x00,0x50,0x30,0x00,0x00}, /* 44 ',' */
    {0x08,0x08,0x08,0x08,0x08}, /* 45 '-' */
    {0x00,0x60,0x60,0x00,0x00}, /* 46 '.' */
    {0x20,0x10,0x08,0x04,0x02}, /* 47 '/' */
    {0x3E,0x51,0x49,0x45,0x3E}, /* 48 '0' */
    {0x00,0x42,0x7F,0x40,0x00}, /* 49 '1' */
    {0x42,0x61,0x51,0x49,0x46}, /* 50 '2' */
    {0x21,0x41,0x45,0x4B,0x31}, /* 51 '3' */
    {0x18,0x14,0x12,0x7F,0x10}, /* 52 '4' */
    {0x27,0x45,0x45,0x45,0x39}, /* 53 '5' */
    {0x3C,0x4A,0x49,0x49,0x30}, /* 54 '6' */
    {0x01,0x71,0x09,0x05,0x03}, /* 55 '7' */
    {0x36,0x49,0x49,0x49,0x36}, /* 56 '8' */
    {0x06,0x49,0x49,0x29,0x1E}, /* 57 '9' */
    {0x00,0x36,0x36,0x00,0x00}, /* 58 ':' */
    {0x00,0x56,0x36,0x00,0x00}, /* 59 ';' */
    {0x00,0x08,0x14,0x22,0x41}, /* 60 '<' */
    {0x14,0x14,0x14,0x14,0x14}, /* 61 '=' */
    {0x41,0x22,0x14,0x08,0x00}, /* 62 '>' */
    {0x02,0x01,0x51,0x09,0x06}, /* 63 '?' */
    {0x32,0x49,0x79,0x41,0x3E}, /* 64 '@' */
    {0x7E,0x11,0x11,0x11,0x7E}, /* 65 'A' */
    {0x7F,0x49,0x49,0x49,0x36}, /* 66 'B' */
    {0x3E,0x41,0x41,0x41,0x22}, /* 67 'C' */
    {0x7F,0x41,0x41,0x22,0x1C}, /* 68 'D' */
    {0x7F,0x49,0x49,0x49,0x41}, /* 69 'E' */
    {0x7F,0x09,0x09,0x01,0x01}, /* 70 'F' */
    {0x3E,0x41,0x41,0x51,0x32}, /* 71 'G' */
    {0x7F,0x08,0x08,0x08,0x7F}, /* 72 'H' */
    {0x00,0x41,0x7F,0x41,0x00}, /* 73 'I' */
    {0x20,0x40,0x41,0x3F,0x01}, /* 74 'J' */
    {0x7F,0x08,0x14,0x22,0x41}, /* 75 'K' */
    {0x7F,0x40,0x40,0x40,0x40}, /* 76 'L' */
    {0x7F,0x02,0x04,0x02,0x7F}, /* 77 'M' */
    {0x7F,0x04,0x08,0x10,0x7F}, /* 78 'N' */
    {0x3E,0x41,0x41,0x41,0x3E}, /* 79 'O' */
    {0x7F,0x09,0x09,0x09,0x06}, /* 80 'P' */
    {0x3E,0x41,0x51,0x21,0x5E}, /* 81 'Q' */
    {0x7F,0x09,0x19,0x29,0x46}, /* 82 'R' */
    {0x46,0x49,0x49,0x49,0x31}, /* 83 'S' */
    {0x01,0x01,0x7F,0x01,0x01}, /* 84 'T' */
    {0x3F,0x40,0x40,0x40,0x3F}, /* 85 'U' */
    {0x1F,0x20,0x40,0x20,0x1F}, /* 86 'V' */
    {0x7F,0x20,0x18,0x20,0x7F}, /* 87 'W' */
    {0x63,0x14,0x08,0x14,0x63}, /* 88 'X' */
    {0x03,0x04,0x78,0x04,0x03}, /* 89 'Y' */
    {0x61,0x51,0x49,0x45,0x43}, /* 90 'Z' */
    {0x00,0x00,0x7F,0x41,0x41}, /* 91 '[' */
    {0x02,0x04,0x08,0x10,0x20}, /* 92 '\' */
    {0x41,0x41,0x7F,0x00,0x00}, /* 93 ']' */
    {0x04,0x02,0x01,0x02,0x04}, /* 94 '^' */
    {0x40,0x40,0x40,0x40,0x40}, /* 95 '_' */
    {0x00,0x01,0x02,0x04,0x00}, /* 96 '`' */
    {0x20,0x54,0x54,0x54,0x78}, /* 97 'a' */
    {0x7F,0x48,0x44,0x44,0x38}, /* 98 'b' */
    {0x38,0x44,0x44,0x44,0x20}, /* 99 'c' */
    {0x38,0x44,0x44,0x48,0x7F}, /* 100 'd' */
    {0x38,0x54,0x54,0x54,0x18}, /* 101 'e' */
    {0x08,0x7E,0x09,0x01,0x02}, /* 102 'f' */
    {0x08,0x14,0x54,0x54,0x3C}, /* 103 'g' */
    {0x7F,0x08,0x04,0x04,0x78}, /* 104 'h' */
    {0x00,0x44,0x7D,0x40,0x00}, /* 105 'i' */
    {0x20,0x40,0x44,0x3D,0x00}, /* 106 'j' */
    {0x00,0x7F,0x10,0x28,0x44}, /* 107 'k' */
    {0x00,0x41,0x7F,0x40,0x00}, /* 108 'l' */
    {0x7C,0x04,0x18,0x04,0x78}, /* 109 'm' */
    {0x7C,0x08,0x04,0x04,0x78}, /* 110 'n' */
    {0x38,0x44,0x44,0x44,0x38}, /* 111 'o' */
    {0x7C,0x14,0x14,0x14,0x08}, /* 112 'p' */
    {0x08,0x14,0x14,0x18,0x7C}, /* 113 'q' */
    {0x7C,0x08,0x04,0x04,0x08}, /* 114 'r' */
    {0x48,0x54,0x54,0x54,0x20}, /* 115 's' */
    {0x04,0x3F,0x44,0x40,0x20}, /* 116 't' */
    {0x3C,0x40,0x40,0x20,0x7C}, /* 117 'u' */
    {0x1C,0x20,0x40,0x20,0x1C}, /* 118 'v' */
    {0x3C,0x40,0x30,0x40,0x3C}, /* 119 'w' */
    {0x44,0x28,0x10,0x28,0x44}, /* 120 'x' */
    {0x0C,0x50,0x50,0x50,0x3C}, /* 121 'y' */
    {0x44,0x64,0x54,0x4C,0x44}, /* 122 'z' */
    {0x00,0x08,0x36,0x41,0x00}, /* 123 '{' */
    {0x00,0x00,0x7F,0x00,0x00}, /* 124 '|' */
    {0x00,0x41,0x36,0x08,0x00}, /* 125 '}' */
    {0x08,0x08,0x2A,0x1C,0x08}, /* 126 '~' */
    {0x08,0x1C,0x2A,0x08,0x08}, /* 127 DEL */
};

//frame buffers for display data; each byte corresponds to a vertical column of 8 pixels, so 4 pages of 128 bytes for 128x32 display
static uint8_t framebuffer[DISPLAY_BUF_SIZE];


//helper functions for CS, A0, and RES pin control
static void Display_CS_Low(void)  { HAL_GPIO_WritePin(Display_NSS_GPIO_Port, Display_NSS_Pin, GPIO_PIN_RESET); }
static void Display_CS_High(void) { HAL_GPIO_WritePin(Display_NSS_GPIO_Port, Display_NSS_Pin, GPIO_PIN_SET); }
static void Display_A0_Low(void)  { HAL_GPIO_WritePin(Display_MISO_GPIO_Port, Display_MISO_Pin, GPIO_PIN_RESET); } // A0=0: command
static void Display_A0_High(void) { HAL_GPIO_WritePin(Display_MISO_GPIO_Port, Display_MISO_Pin, GPIO_PIN_SET); }   // A0=1: data
static void Display_RES_Low(void) { HAL_GPIO_WritePin(Display_RES_GPIO_Port, Display_RES_Pin, GPIO_PIN_RESET); }
static void Display_RES_High(void){ HAL_GPIO_WritePin(Display_RES_GPIO_Port, Display_RES_Pin, GPIO_PIN_SET); }

static void Display_SendCommand(uint8_t cmd)
{
    Display_A0_Low();
    Display_CS_Low();
    

    HAL_SPI_Transmit(&hspi3, &cmd, 1, HAL_MAX_DELAY);

    for(volatile int i=0; i<100; i++) __NOP();

    Display_CS_High();

}

static void Display_SendData(uint8_t *data, uint16_t len)
{
    Display_A0_High();
    Display_CS_Low();

    HAL_SPI_Transmit(&hspi3, data, len, HAL_MAX_DELAY);

    for(volatile int i=0; i<100; i++) __NOP();
    
    Display_CS_High();

    
}

static void Display_Reset(void)
{
    Display_RES_High();
    vTaskDelay(pdMS_TO_TICKS(5));
    Display_RES_Low();
   vTaskDelay(pdMS_TO_TICKS(20));
    Display_RES_High();
    vTaskDelay(pdMS_TO_TICKS(50));
}

//init functions 
void Display_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};


    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();

    //MISO (A0), RES, and NSS pins start in known states
    HAL_GPIO_WritePin(Display_NSS_GPIO_Port, Display_NSS_Pin, GPIO_PIN_SET);      //CS = high (deselected)
    HAL_GPIO_WritePin(Display_RES_GPIO_Port, Display_RES_Pin, GPIO_PIN_SET);    // RES = high (not in reset)
    HAL_GPIO_WritePin(Display_MISO_GPIO_Port, Display_MISO_Pin, GPIO_PIN_RESET); // A0 = low (command mode by default)

    //A0 pin init

    GPIO_InitStruct.Pin = Display_MISO_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(Display_MISO_GPIO_Port, &GPIO_InitStruct);


    //RES pin init
    GPIO_InitStruct.Pin = Display_RES_Pin;
    HAL_GPIO_Init(Display_RES_GPIO_Port, &GPIO_InitStruct);

    //NSS pin init
    GPIO_InitStruct.Pin = Display_NSS_Pin;
    HAL_GPIO_Init(Display_NSS_GPIO_Port, &GPIO_InitStruct);
    
}


void Display_Init(void)
{

    Display_GPIO_Init();
    Display_Reset();

    // ST7565R internal initialization sequence from datasheet
    Display_SendCommand(CMD_ADC_NORMAL);
    Display_SendCommand(CMD_COM_REVERSE);
    Display_SendCommand(CMD_BIAS_1_7);
    Display_SendCommand(CMD_POWER_ALL_ON);
    Display_SendCommand(CMD_RESISTOR_RATIO | 1);
    Display_SendCommand(CMD_SET_VOLUME);
    Display_SendCommand(0x10);  // contrast value: 32
    Display_SendCommand(CMD_START_LINE | 0);
    Display_SendCommand(CMD_DISPLAY_NORMAL);
    Display_SendCommand(CMD_DISPLAY_ON);

    Display_Clear();
}

//command functions to send to display controller
void Display_Clear(void)
{
    memset(framebuffer, 0x00, DISPLAY_BUF_SIZE);
    Display_Update();
}

void Display_Update(void)
{
    const uint8_t col_offset = 4; // ST7565R has 132 columns; display starts at column 4
    for (uint8_t page = 0; page < DISPLAY_PAGES; page++) {
        Display_SendCommand(CMD_PAGE_ADDR | page);
        Display_SendCommand(CMD_COL_ADDR_HI | (col_offset >> 4));
        Display_SendCommand(CMD_COL_ADDR_LO | (col_offset & 0x0F));
        Display_SendData(&framebuffer[page * DISPLAY_WIDTH], DISPLAY_WIDTH);
    }
}

void Display_SetContrast(uint8_t val)
{
    if (val > 63) val = 63;
    Display_SendCommand(CMD_SET_VOLUME);
    Display_SendCommand(val);
}

void Display_On(void)
{
    Display_SendCommand(CMD_DISPLAY_ON);
}

void Display_Off(void)
{
    Display_SendCommand(CMD_DISPLAY_OFF);
}

//drawing functions for display

void Display_SetPixel(uint8_t x, uint8_t y, bool on)
{
    if (x >= DISPLAY_WIDTH || y >= DISPLAY_HEIGHT) {
        return;
    }

    uint16_t idx = (y / 8) * DISPLAY_WIDTH + x;
    uint8_t  bit = y % 8;

    if (on)
        framebuffer[idx] |= (1 << bit);
    else
        framebuffer[idx] &= ~(1 << bit);
}

void Display_DrawChar(uint8_t x, uint8_t y, char c)
{
    uint8_t uc = (uint8_t)c;
    if (uc < 32 || uc > 127) uc = (uint8_t)'?';
    if (x + GLYPH_W > DISPLAY_WIDTH || y >= DISPLAY_HEIGHT) return;

    const uint8_t *glyph = font5x7[uc - 32];
    uint8_t page = y / 8;
    uint16_t base = page * DISPLAY_WIDTH + x;

    for (uint8_t col = 0; col < GLYPH_W; col++) {
        framebuffer[base + col] = glyph[col];
    }
}

void Display_DrawString(uint8_t x, uint8_t y, const char *str)
{
    if (!str) return;
    if (x >= DISPLAY_WIDTH || y >= DISPLAY_HEIGHT) return;

    while (*str) {
        char c = *str++;

        if (c == '\n') {
            x = 0;
            y = (uint8_t)(y + CELL_H);
            if (y >= DISPLAY_HEIGHT) break;
            continue;
        }

        // dont draw off da screen
        if (x >= DISPLAY_WIDTH) break;

        Display_DrawChar(x, y, c);

        HAL_GPIO_TogglePin(LEDMaps[LED_CHARGE].port, LEDMaps[LED_CHARGE].pin);

        Display_Update();

        vTaskDelay(pdMS_TO_TICKS(200));

        if ((uint16_t)x + (uint16_t)CELL_W >= (uint16_t)DISPLAY_WIDTH) break;
        x = (uint8_t)(x + CELL_W);
    }
    
    Display_Update();
    HAL_GPIO_WritePin(LEDMaps[LED_CHARGE].port, LEDMaps[LED_CHARGE].pin, GPIO_PIN_RESET);

}


void Display_TestPattern(void)
{
    for (uint8_t y = 0; y < DISPLAY_HEIGHT; y++) {
        for (uint8_t x = 0; x < DISPLAY_WIDTH; x++) {
            Display_SetPixel(x, y, ((x + y) % 2) == 0);
        }
    }
    Display_Update();
}

void Display_DebugMark(void)
{
    for (uint8_t i = 0; i < 10; i++) {
        Display_SetPixel(i, i, true);
        Display_SetPixel(i, (uint8_t)(9 - i), true);
    }
    Display_Update();
}
