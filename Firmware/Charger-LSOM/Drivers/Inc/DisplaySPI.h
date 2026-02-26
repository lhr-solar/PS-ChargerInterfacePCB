#pragma once

#include "stm32xx_hal.h"
#include "pinDef.h"
#include <stdbool.h>
#include <stdint.h>

// LCD Display Controller Commands
// lowk will delete some commands that will never be used
// Data sheet for commands: https://newhavendisplay.com/content/specs/NHD-C12832A1Z-FSB-FBW-3V3.pdf



typedef enum {
    DISPLAY_OK = 0,
    DISPLAY_ERROR_INVALID_PARAM,
    DISPLAY_ERROR_HW_FAILURE,
    DISPLAY_ERROR_OUT_OF_BOUNDS,
} Display_status_t;

//Start up values
#define DISPLAY_RESET_LOW_DELAY_MS      20      // hold RES low for 20ms to guarantee full controller reset
#define DISPLAY_RESET_HIGH_DELAY_MS     5       // bring RES high 5ms before sending commands (setup time)
#define DISPLAY_STARTUP_DELAY_MS        50      // wait 50ms after reset before first command (controller stabilization)
#define DISPLAY_START_CONTRAST_VAL      0x10    // initial contrast: 16/63, safe mid-low value to prevent burn-in at startup

// ST7565R has 132 column drivers but the NHD-C12832 display panel only uses 128.
// The panel is centered, so pixel 0 maps to column driver 4.
#define DISPLAY_COL_OFFSET              4

// CMD_SET_VOLUME accepts a 6-bit value (0-63). Values above 63 are clamped to 63.
#define DISPLAY_CONTRAST_MAX            63


// turns display on and off
#define CMD_DISPLAY_OFF 0xAE
#define CMD_DISPLAY_ON 0xAF

// decides which RAM row map to utilize as the top of the display
#define CMD_START_LINE 0x40

// sets RAM page address (0-3 for 32-row display)
#define CMD_PAGE_ADDR 0xB0

// sets where next data byte will be wriitten
#define CMD_COL_ADDR_HI 0x10 // OR with upper nibble
#define CMD_COL_ADDR_LO 0x00 // OR with lower nibble

// controlls LCD segment driver and common output direction
#define CMD_ADC_NORMAL 0xA0 // SEG0 -> SEG131
// mirrors dislpay horizontally by reversing column order
#define CMD_ADC_REVERSE 0xA1 // SEG131 -> SEG0

// regular dipslay
#define CMD_DISPLAY_NORMAL 0xA6
// invert light and dark pixels
#define CMD_DISPLAY_REVERSE 0xA7

// forces all pixels on and off
#define CMD_ALL_POINTS_OFF 0xA4 // normal display
#define CMD_ALL_POINTS_ON 0xA5  // all pixels ON (test)

// (11) LCD bias set
#define CMD_BIAS_1_9 0xA2
#define CMD_BIAS_1_7 0xA3

// software reset
#define CMD_RESET 0xE2

// verticlal ADC flips to rotate displays
#define CMD_COM_NORMAL 0xC0  // COM0 -> COM63
#define CMD_COM_REVERSE 0xC8 // COM63 -> COM0

// power control sets
#define CMD_POWER_CTRL 0x28 // staged power ups
#define POWER_BOOSTER 0x04
#define POWER_REGULATOR 0x02
#define POWER_FOLLOWER 0x01
#define CMD_POWER_ALL_ON 0x2F // booster + regulator + follower

// sets internal resistor divider ratio that determines operating voltage of LCD
#define CMD_RESISTOR_RATIO 0x20

// sets the V0 voltage within range to adjust contras
#define CMD_SET_VOLUME 0x81 // followed by value 0-63

// ultra low power mode (lowk never used)
#define CMD_SLEEP_ON 0xAC
#define CMD_SLEEP_OFF 0xAD

// tuning voltage step-up multiplier (not used)
#define CMD_BOOSTER_RATIO 0xF8 // followed by 0x00-0x03

// display dimension from datasheet: 128x32 pixels, 4 pages of 8 rows each
#define DISPLAY_WIDTH 128
#define DISPLAY_HEIGHT 32
#define DISPLAY_PAGES (DISPLAY_HEIGHT / 8)
#define DISPLAY_BUF_SIZE (DISPLAY_WIDTH * DISPLAY_PAGES)

/* Font dimensions (5x7 in a 6x8 cell) */
#define FONT_WIDTH 5
#define FONT_HEIGHT 7
#define FONT_CELL_W 6
#define FONT_CELL_H 8

#define DISPLAY_MAX_COLS (DISPLAY_WIDTH / FONT_CELL_W)
#define DISPLAY_MAX_ROWS (DISPLAY_HEIGHT / FONT_CELL_H)


/**
 * @brief   Initalizes the display by configuring GPIO pins, resetting the controller, and sending the required initialization command sequence. 
 */
void Display_Init(void);



/**
 * @brief   Clears the display by setting all pixels in the framebuffer to off and updating the display.
 */
void Display_Clear(void);

/**
 * @brief   Updates the display by sending the contents of the framebuffer to the display.
 */
void Display_Update(void);

/**
 * @brief   Sets contrast level of the Display
 * @param   val Contrast value (0-63), default is around 32. Higher values increase contrast.
 * @return  Adjusts the display contrast by setting the internal resistor value to change internal Voltage value
 */

void Display_SetContrast(uint8_t val);

/**
 * @brief   Turns the display on by sending the appropriate command to the controller
 */
void Display_On(void);

/**
 * @brief   Turns the display off by sending the appropriate command to the controller
 */
void Display_Off(void);


/**
 * @brief   Sets pixel at (x,y) to on or off
 * @param   x X-coordinate of the pixel (0 to DISPLAY_WIDTH-1)
 * @param   y Y-coordinate of the pixel (0 to DISPLAY_HEIGHT-1)
 * @param   on true to turn the pixel on, false to turn it off
 * @return  Modifies the framebuffer to set or clear the specified pixel.
 */
void Display_SetPixel(uint8_t x, uint8_t y, bool on);

/**
 * @brief   Draws a single character at (x,y) using a 5x7 font within a 6x8 cell. Only supports ASCII characters 32-126.
 * @param   x X-coordinate of the top-left corner of the character cell (0 to DISPLAY_WIDTH-1)
 * @param   y Y-coordinate of the top-left corner of the character cell (0 to DISPLAY_HEIGHT-1)
 * @param   c ASCII character to draw (32-126). Characters outside this range will be rendered as '?'.
 * @return  Renders the specified character on the display at the given coordinates. The character is drawn using a 5x7 pixel font, and the function handles clipping if the character exceeds display boundaries.
 */
void Display_DrawChar(uint8_t x, uint8_t y, char c);

/**
 * @brief   Outputs a null-terminated string starting at (x,y). Supports newline characters to move to the next line. Text will wrap to the next line if it exceeds display width, and will be truncated if it exceeds display height.
 * @param   x X-coordinate of the top-left corner of the first character cell (0 to DISPLAY_WIDTH-1)
 * @param   y Y-coordinate of the top-left corner of the first character cell (0 to DISPLAY_HEIGHT-1)
 * @param   str Null-terminated string to draw
 */
Display_status_t Display_DrawString(uint8_t x, uint8_t y, const char *str);

/**
 * @brief   Draws a test pattern on the display (checkerboard) for testing and debugging purposes.
 * @return  Renders a checkerboard pattern on the display by setting pixels on and off
 */
void Display_TestPattern(void);


