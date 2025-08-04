/*
 * i2cLcd.c
 *
 *  Created on: Aug 3, 2025
 *      Author: BrachiGH
 *  Description : LCD with I2C Interface.
 *  Source-Code : https://github.com/brachiGH/i2c_lcd_stm32_driver
 *  Description : I2C Serial Interface 1602 Hitachi’s HD44780 based character LCD Module STM32 Library
 */

#ifndef I2CLCD_H_
#define I2CLCD_H_

#include <stdbool.h>
#include <stdint.h>

#if __has_include("stm32f4xx_hal.h")
  #include "stm32f4xx_hal.h"
#elif __has_include("stm32f1xx_hal.h")
  #include "stm32f1xx_hal.h"
#elif __has_include("stm32c0xx_hal.h")
  #include "stm32c0xx_hal.h"
#elif __has_include("stm32g4xx_hal.h")
  #include "stm32g4xx_hal.h"
#endif

#define I2CLCD_WAKEUP_CMD             0x30
#define I2CLCD_4BIT_CMD               0x02
#define I2CLCD_DISPLAY_CMD            0x08
#define I2CLCD_CLEAR_DISPLAY_CMD      0x01
#define I2CLCD_ENTRY_MODE_CMD         0x04
#define I2CLCD_CURSOR_HOME_CMD        0x02

#define I2CLCD_MAX_BF_POLLS		        127

#define I2CLCD_START_ADDRESS_ROW_1    0x80
#define I2CLCD_START_ADDRESS_ROW_2    0xC0

#define I2CLCD_RS_POS                 (0)
#define I2CLCD_RW_POS                 (1)
#define I2CLCD_E_POS                  (2)
#define I2CLCD_DISPLAY_ON_POS         (2)
#define I2CLCD_functionSet_F_POS      (2)
#define I2CLCD_functionSet_N_POS      (3)
#define I2CLCD_entryMode_ID_POS       (1)
#define I2CLCD_entryMode_S_POS        (0)
#define I2CLCD_SHIFT_RIGHT_POS        (2)
#define I2CLCD_DISPLAY_SHIFT_POS      (3)

#define I2CLCD_RS                     (1 << I2CLCD_RS_POS)
#define I2CLCD_RW                     (1 << I2CLCD_RW_POS)
#define I2CLCD_E                      (1 << I2CLCD_E_POS)
#define I2CLCD_DISPLAY_ON             (1 << I2CLCD_DISPLAY_ON_POS)
#define I2CLCD_SHIFT_RIGHT            (1 << I2CLCD_SHIFT_RIGHT_POS)
#define I2CLCD_DISPLAY_SHIFT          (1 << I2CLCD_DISPLAY_SHIFT_POS)

/**
 * @brief Generate the slave address based on inputs (A0 A1 A2).
 * @param lcd: Pointer to the LCD handle
 */
#define I2CLCD_SLAVE_ADDRESS(lcd) (0x40 | ((lcd)->A0A1A2 << 1))

/**
 * @brief Generate the cursor options bits.
 * @param lcd: Pointer to the LCD handle
 */
#define I2CLCD_CURSOR_OPTIONS(lcd) (((lcd)->cursor << 1) | (lcd)->cursorBlink)

/**
 * @brief Generate LCD Command from provided Signals.
 * @param enBit: Starts data read/write.
 * @param rwBit: Selects read or write.
        0: Write
        1: Read
 * @param rsBit; Selects registers.
        0: Instruction register (for write) Busy flag (for read)
        1: Data register (for write and read)
 */
#define I2CLCD_GENERATE_COMMAND_SIGNALS(enBit, rwBit, rsBit)\
 (0x08 | (enBit << I2CLCD_E_POS) | (rwBit << I2CLCD_RW_POS) | (rsBit  << I2CLCD_RS_POS))

/**
 * @brief i2cLcd holds i2c handle and the i2c address.
 * I/D: Increments (I/D = 1) or decrements (I/D = 0) the DDRAM address by 1 when a character code is
  written into or read from DDRAM.
  The cursor or blinking moves to the right when incremented by 1 and to the left when decremented by 1.
  The same applies to writing and reading of CGRAM.
 * S: Shifts the entire display either to the right (I/D = 0) or to the left (I/D = 1) when S is 1. The display does
  not shift if S is 0.
  If S is 1, it will seem as if the cursor does not move but the display does. The display does not shift when
  reading from DDRAM. Also, writing into or reading out from CGRAM does not shift the display.
 * N: Sets the number of display lines.
 * F: Sets the character font.
  Note: Perform the function at the head of the program before executing any instructions (except for the
  read busy flag and address instruction). From this point, the function set instruction cannot be
  executed unless the interface data length is changed.
 * @attention Refer to figure 1 in readme.md
 */
typedef struct
{
  I2C_HandleTypeDef *hi2c;
  uint8_t A0A1A2;
  bool cursor;
  bool cursorBlink;
  bool functionSet_N;
  bool functionSet_F;
  bool entryMode_ID;
  bool entryMode_S;
} i2cLcd_handle;

/**
 * @brief Initializes the LCD.
 * @param lcd: Pointer to the LCD handle.
 * @retval true if initialization successful else false.
 * @attention For more information refer to figure 2
 */
bool lcdInit(const i2cLcd_handle *lcd);


/**
 * @brief Send a command to the LCD.
 * @param lcd: Pointer to the LCD handle.
 * @param cmd: Command byte to send.
 * @retval true if command was sent successful else false.
 */
bool lcd_sendCmd(const i2cLcd_handle *lcd, const char cmd);

/**
 * @brief Clear Display.
 * @param lcd: Pointer to the LCD handle.
 * @retval true if command was sent successful else false.
 */
bool lcd_clearDisplay(const i2cLcd_handle *lcd);

/**
 * @brief Turn On the display.
 * @param lcd: Pointer to the LCD handle.
 * @retval true if command was sent successful else false.
 */
bool lcd_displayOn(const i2cLcd_handle *lcd);

/**
 * @brief Turn Off the display.
 * @param lcd: Pointer to the LCD handle.
 * @retval true if command was sent successful else false.
 */
bool lcd_displayOff(const i2cLcd_handle *lcd);

/**
 * @brief Update cursor options.
 * @param lcd: Pointer to the LCD handle.
 * @retval true if command was sent successful else false.
 */
bool lcd_updateCursorOprions(const i2cLcd_handle *lcd);

/**
 * @brief I/D: Increments (I/D = 1) or decrements (I/D = 0) the DDRAM address by 1 when a character code is
  written into or read from DDRAM.
  S: Shifts the entire display either to the right (I/D = 0) or to the left (I/D = 1) when S is 1. The display does
  not shift if S is 0.
 * @param lcd: Pointer to the LCD handle.
 * @retval true if command was sent successful else false.
 */
bool lcd_updateEntryMode(const i2cLcd_handle *lcd);

/**
 * @brief Update cursor options.
 * @param lcd: Pointer to the LCD handle.
 * @retval true if command was sent successful else false.
 */
bool lcd_updateCursorOprions(const i2cLcd_handle *lcd);

/**
 * @brief Shift cursor or display by one position.
 * @param lcd: Pointer to the LCD handle.
 * @param shift_right: if true I2CLCD_SHIFT_RIGHT else I2CLCD_SHIFT_LEFT
 * @retval true if command was sent successful else false.
 */
bool lcd_shiftCusorOrDisplay(const i2cLcd_handle *lcd, const bool shift_right, const bool shift_display);

/**
 * @brief  Moves the cursor to a specific position on the LCD.
 * @param  lcd: Pointer to the LCD handle
 * @param  col: Column number (0-39)
 * @param  row: Row number (0 or 1)
 * @retval true if command was sent successful else false.
 */
bool lcd_moveCursor(const i2cLcd_handle *lcd, int row, int col);


/**
 * @brief  Moves the cursor to a row 0, col 0 position on the LCD.
 * @param  lcd: Pointer to the LCD handle
 * @retval true if command was sent successful else false.
 */
bool lcd_moveCursorHome(const i2cLcd_handle *lcd);

/**
 * @brief  Sends data (character) to the LCD.
 * @param  lcd: Pointer to the LCD handle
 * @param  data: Data byte to send
 * @retval true if command was sent successful else false.
 */
bool lcd_sendData(const i2cLcd_handle *lcd, const char data);

/**
 * @brief  Sends a single character to the LCD.
 * @param  lcd: Pointer to the LCD handle
 * @param  ch: Character to send
 * @retval true if command was sent successful else false.
 */
bool lcd_putchar(const i2cLcd_handle *lcd, const char ch);

/**
 * @brief  Sends a string to the LCD.
 * @param  lcd: Pointer to the LCD handle
 * @param  str: Null-terminated string to display
 * @retval true if command was sent successful else false.
 */
bool lcd_puts(const i2cLcd_handle *lcd, const char *str);

#endif /* I2CLCD_H_ */
