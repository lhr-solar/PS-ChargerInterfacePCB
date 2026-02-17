#ifndef DISPLAYSPI_H
#define DISPLAYSPI_H

#include "stm32xx_hal.h"
#include "pinDef.h"
#include <stdbool.h>
#include <stdint.h>

//LCD Display Controller Commands
//lowk will delete some commands that will never be used


//turns display on and off
#define CMD_DISPLAY_OFF             0xAE
#define CMD_DISPLAY_ON              0xAF

// decides which RAM row map to utilize as the top of the display
#define CMD_START_LINE              0x40

// sets RAM page address (0-3 for 32-row display)
#define CMD_PAGE_ADDR               0xB0

// sets where next data byte will be wriitten
#define CMD_COL_ADDR_HI             0x10  // OR with upper nibble
#define CMD_COL_ADDR_LO             0x00  // OR with lower nibble

// controlls LCD segment driver and common output direction
#define CMD_ADC_NORMAL              0xA0  // SEG0 -> SEG131
//mirrors dislpay horizontally by reversing column order
#define CMD_ADC_REVERSE             0xA1  // SEG131 -> SEG0

//regular dipslay
#define CMD_DISPLAY_NORMAL          0xA6
//invert light and dark pixels
#define CMD_DISPLAY_REVERSE         0xA7

// forces all pixels on and off
#define CMD_ALL_POINTS_OFF          0xA4  // normal display
#define CMD_ALL_POINTS_ON           0xA5  // all pixels ON (test)

// (11) LCD bias set
#define CMD_BIAS_1_9                0xA2
#define CMD_BIAS_1_7                0xA3

//software reset
#define CMD_RESET                   0xE2

//verticlal ADC flips to rotate displays 
#define CMD_COM_NORMAL              0xC0  // COM0 -> COM63
#define CMD_COM_REVERSE             0xC8  // COM63 -> COM0

// power control sets
#define CMD_POWER_CTRL              0x28  //staged power ups
#define POWER_BOOSTER               0x04
#define POWER_REGULATOR             0x02
#define POWER_FOLLOWER              0x01
#define CMD_POWER_ALL_ON            0x2F  // booster + regulator + follower

// sets internal resistor divider ratio that determines operating voltage of LCD
#define CMD_RESISTOR_RATIO          0x20

// sets the V0 voltage within range to adjust contras
#define CMD_SET_VOLUME              0x81  // followed by value 0-63

//ultra low power mode (lowk never used)
#define CMD_SLEEP_ON                0xAC
#define CMD_SLEEP_OFF               0xAD

//tuning voltage step-up multiplier (not used)
#define CMD_BOOSTER_RATIO           0xF8  // followed by 0x00-0x03


//display dimension from datasheet: 128x32 pixels, 4 pages of 8 rows each
#define DISPLAY_WIDTH   128
#define DISPLAY_HEIGHT  32
#define DISPLAY_PAGES   (DISPLAY_HEIGHT / 8)  
#define DISPLAY_BUF_SIZE (DISPLAY_WIDTH * DISPLAY_PAGES)  

/* Font dimensions (5x7 in a 6x8 cell) */
#define FONT_WIDTH  5
#define FONT_HEIGHT 7
#define FONT_CELL_W 6  
#define FONT_CELL_H 8  

#define DISPLAY_MAX_COLS (DISPLAY_WIDTH / FONT_CELL_W)   
#define DISPLAY_MAX_ROWS (DISPLAY_HEIGHT / FONT_CELL_H) 

//init and control functions
void Display_Init(void);
void Display_Clear(void);
void Display_Update(void);
void Display_SetContrast(uint8_t val);
void Display_On(void);
void Display_Off(void);

//action functions

void Display_SetPixel(uint8_t x, uint8_t y, bool on);
void Display_DrawChar(uint8_t x, uint8_t y, char c);
void Display_DrawString(uint8_t x, uint8_t y, const char *str);
void Display_TestPattern(void);

#endif
