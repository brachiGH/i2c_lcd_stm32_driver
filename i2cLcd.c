/*
 * i2cLcd.c
 *
 *  Created on: Aug 3, 2025
 *      Author: BrachiGH
 *  Description : LCD with I2C Interface.
 *	Source-Code : https://github.com/brachiGH/i2c_lcd_stm32_driver
 *  Description : I2C Serial Interface 1602 Hitachi’s HD44780 based character LCD Module STM32 Library
 */

#include "i2cLcd.h"


/**
 * @brief Checks the lcd ready to receive data.
 * @param lcd: Pointer to the LCD handle.
 * @retval HAL Status structures definition.
 */
HAL_StatusTypeDef _lcd_checkBusyFlag(const i2cLcd_handle *lcd)
{
  uint8_t data_t[2];
  data_t[0] =  LCD__GENERATE_COMMAND_SIGNALS(0, 1, 0);  // en=0, rs=0, r/w=1
  data_t[1] =  LCD__GENERATE_COMMAND_SIGNALS(1, 1, 0);  // en=1, rs=0, r/w=1

  HAL_I2C_Master_Transmit(lcd->hi2c, LCD__SLAVE_ADDRESS(lcd), data_t, 2, 100);

  uint8_t data[2];
  HAL_StatusTypeDef status = HAL_I2C_Master_Receive(lcd->hi2c, LCD__SLAVE_ADDRESS(lcd), data, 2, 100);
  if (status == HAL_OK)
  {
	  uint8_t flag = data[0] & 0x80;
	  return  flag == 0 ? HAL_OK : HAL_BUSY;
  }

  return status;
}

/**
 * @brief Send a command to the LCD.
 * @param lcd: Pointer to the LCD handle.
 * @param cmd: Command byte to send.
 * @retval true if command was sent successful else false.
 */
bool lcd_sendCmd(const i2cLcd_handle *lcd, const char cmd)
{
	HAL_StatusTypeDef status = HAL_BUSY;
	while(status == HAL_BUSY)
	{
		status = _lcd_checkBusyFlag(lcd);
	}
	if (status != HAL_OK)
	{
		return false;
	}

    char upper_nibble, lower_nibble;
    uint8_t data_t[4];

    upper_nibble = (cmd & 0xF0);            // Extract upper nibble
    lower_nibble = ((cmd << 4) & 0xF0);     // Extract lower nibble

    data_t[0] = upper_nibble | LCD__GENERATE_COMMAND_SIGNALS(1, 0, 0);  // en=1, rw=0, rs=0
    data_t[1] = upper_nibble | LCD__GENERATE_COMMAND_SIGNALS(0, 0, 0);  // en=0, rw=0, rs=0
    data_t[2] = lower_nibble | LCD__GENERATE_COMMAND_SIGNALS(1, 0, 0);  // en=1, rw=0, rs=0
    data_t[3] = lower_nibble | LCD__GENERATE_COMMAND_SIGNALS(0, 0, 0);  // en=0, rw=0, rs=0

    status = HAL_I2C_Master_Transmit(lcd->hi2c, LCD__SLAVE_ADDRESS(lcd), data_t, 4, 100);

    if (status ==HAL_OK)
    {
    	return true;
    }
	return false;
}

/**
 * @brief Send a 4 bit command to the LCD (only upper nibble).
 * @param lcd: Pointer to the LCD handle.
 * @param cmd: Command byte to send.
 * @retval true if command was sent successful else false.
 */
bool _lcd_send4bitCmd(const i2cLcd_handle *lcd, const char cmd)
{
	HAL_StatusTypeDef status = HAL_BUSY;
	while(status == HAL_BUSY)
	{
		status = _lcd_checkBusyFlag(lcd);
	}
	if (status != HAL_OK)
	{
		return false;
	}

    char upper_nibble;
    uint8_t data_t[2];

    upper_nibble = (cmd & 0xF0);            // Extract upper nibble

    data_t[0] = upper_nibble | LCD__GENERATE_COMMAND_SIGNALS(1, 0, 0);  // en=1, rw=0, rs=0
    data_t[1] = upper_nibble | LCD__GENERATE_COMMAND_SIGNALS(0, 0, 0);  // en=0, rw=0, rs=0

    status = HAL_I2C_Master_Transmit(lcd->hi2c, LCD__SLAVE_ADDRESS(lcd), data_t, 4, 100);

    if (status == HAL_OK)
    {
    	return true;
    }
	return false;
}

/**
 * @brief Sets interface data length
			(DL), number of display lines
			(N), and character font (F).
 * @attention For more information refer to diagram 1.3
 * @param lcd: Pointer to the LCD handle.
 * @retval true if command was sent successful else false.
 */
bool _lcd_functionSet(const i2cLcd_handle *lcd)
{
    bool status = lcd_sendCmd(lcd, 0x20 | (lcd->functionSet_N << 3) | (lcd->functionSet_F << 2));  // Set to 4-bit mode
    HAL_Delay(1);
    return status;
}

/**
 * @brief Initializes the LCD.
 * @param lcd: Pointer to the LCD handle
 * @retval true if initialization successful else false.
 * @attention For more information refer to figure 2
 */
bool lcdInit(const i2cLcd_handle *lcd)
{
	bool status = true;

    HAL_Delay(15);            // Wait for LCD power-up
    status &= _lcd_send4bitCmd(lcd, 0x30);  // Wake up command
    HAL_Delay(5);
    status &= _lcd_send4bitCmd(lcd, 0x30);  // Wake up command
    HAL_Delay(1);
    status &= _lcd_send4bitCmd(lcd, 0x30);  // Wake up command
    HAL_Delay(1);
    status &= _lcd_send4bitCmd(lcd, 0x20);  // Set to 4-bit mode
    HAL_Delay(1);

    // LCD configuration commands
    status &= _lcd_functionSet(lcd);		// 4-bit mode, functionSet_N, functionSet_F
    status &= lcd_clearDisplay(lcd); 	// Clear display
    status &= lcd_updateEntryMode(lcd);	// Entry mode: cursor moves and display shift position
    status &=  lcd_moveCursorHome(lcd);	// Move cursor to row: 0 col: 0
    status &= lcd_displayOn(lcd);		// Display on

    return status;
}

/**
 * @brief Turn On the display.
 * @param lcd: Pointer to the LCD handle.
 * @retval true if command was sent successful else false.
 */
bool lcd_displayOn(const i2cLcd_handle *lcd)
{
    bool status = lcd_sendCmd(lcd, 0x00);  // Display on, lcd->cursor, lcd->cursorBlink
    HAL_Delay(1);
    status &= lcd_sendCmd(lcd, 0x0C | LCD__CURSOR_OPTIONS(lcd));  // Display on, lcd->cursor, lcd->cursorBlink
    HAL_Delay(1);
    return status;
}

/**
 * @brief Turn Off the display.
 * @param lcd: Pointer to the LCD handle.
 * @retval true if command was sent successful else false.
 */
bool lcd_displayOff(const i2cLcd_handle *lcd)
{
    bool status = lcd_sendCmd(lcd, 0x08);  // Display off, cursor off, blink off
    HAL_Delay(1);
    return status;
}

/**
 * @brief Update cursor options.
 * @param lcd: Pointer to the LCD handle.
 * @retval true if command was sent successful else false.
 */
bool lcd_updateCursorOprions(const i2cLcd_handle *lcd)
{
    return lcd_displayOn(lcd);
}

/**
 * @brief Clear Display.
 * @param lcd: Pointer to the LCD handle.
 * @retval true if command was sent successful else false.
 */
bool lcd_clearDisplay(const i2cLcd_handle *lcd)
{
    bool status = lcd_sendCmd(lcd, 0x01);  // Clear display
    HAL_Delay(2);
    return status;
}

/**
 * @brief I/D: Increments (I/D = 1) or decrements (I/D = 0) the DDRAM address by 1 when a character code is
	written into or read from DDRAM.
	The cursor or blinking moves to the right when incremented by 1 and to the left when decremented by 1.
	The same applies to writing and reading of CGRAM.
	S: Shifts the entire display either to the right (I/D = 0) or to the left (I/D = 1) when S is 1. The display does
	not shift if S is 0.
	If S is 1, it will seem as if the cursor does not move but the display does. The display does not shift when
	reading from DDRAM. Also, writing into or reading out from CGRAM does not shift the display.
 * @param lcd: Pointer to the LCD handle.
 * @retval true if command was sent successful else false.
 */
bool lcd_updateEntryMode(const i2cLcd_handle *lcd)
{
    bool status = lcd_sendCmd(lcd, 0x04 | (lcd->entryMode_ID << 1) | lcd->entryMode_S);
    HAL_Delay(1);
    return status;
}

/**
 * @brief Shift cursor or display by one position.
 * @param lcd: Pointer to the LCD handle.
 * @param shift: LCD__SHIFT_RIGHT or LCD__SHIFT_LEFT
 * @attention The cursor follows the display shift.
 * @retval true if command was sent successful else false.
 */
bool lcd_shiftCusorOrDisplay(const i2cLcd_handle *lcd, const LCD__SHIFT shift, const bool shift_display)
{
	uint8_t cmd = 0x10; // By Default shifts to the left.
	if (shift == LCD__SHIFT_RIGHT)
	{
		cmd |= 1 << 2;	// Shifts only the cursor position to the right.
	}
	if (shift_display)
	{
		cmd |= 1 << 3;	// Shifts the display and the cursor position to the right.
	}
	bool status = lcd_sendCmd(lcd, cmd);	//Shifts only the cursor position to the left.
	HAL_Delay(1);
	return status;
}

/**
 * @brief  Moves the cursor to a specific position on the LCD.
 * @param  lcd: Pointer to the LCD handle
 * @param  col: Column number (0-39)
 * @param  row: Row number (0 or 1)
 * @retval true if command was sent successful else false.
 */
bool lcd_moveCursor(const i2cLcd_handle *lcd, int row, int col)
{
    uint8_t address;

    switch (row)
    {
        case 0: address = 0x80 + col; break;	// First row
        case 1: address = 0xC0 + col; break;	// Second row
        default: return false;  				// invalid row numbers
    }

	bool status = lcd_sendCmd(lcd, address);
	HAL_Delay(1);
    return status;
}

/**
 * @brief  Moves the cursor to a row 0, col 0 position on the LCD.
 * @param  lcd: Pointer to the LCD handle
 * @retval true if command was sent successful else false.
 */
bool lcd_moveCursorHome(const i2cLcd_handle *lcd)
{
	bool status = lcd_sendCmd(lcd, 0x02);
	HAL_Delay(1);
	return status;
}


/**
 * @brief  Sends data (character) to the LCD.
 * @param  lcd: Pointer to the LCD handle
 * @param  data: Data byte to send
 * @retval true if command was sent successful else false.
 */
bool lcd_sendData(const i2cLcd_handle *lcd, const char data)
{
	HAL_StatusTypeDef status = HAL_BUSY;
	while(status == HAL_BUSY)
	{
		status = _lcd_checkBusyFlag(lcd);
	}
	if (status != HAL_OK)
	{
		return false;
	}

    char upper_nibble, lower_nibble;
    uint8_t data_t[4];

    upper_nibble = (data & 0xF0);            // Extract upper nibble
    lower_nibble = ((data << 4) & 0xF0);     // Extract lower nibble

    data_t[0] = upper_nibble | LCD__GENERATE_COMMAND_SIGNALS(1, 0, 1);  // en=1, rw=0, rs=1
    data_t[1] = upper_nibble | LCD__GENERATE_COMMAND_SIGNALS(0, 0, 1);  // en=0, rw=0, rs=1
    data_t[2] = lower_nibble | LCD__GENERATE_COMMAND_SIGNALS(1, 0, 1);  // en=1, rw=0, rs=1
    data_t[3] = lower_nibble | LCD__GENERATE_COMMAND_SIGNALS(0, 0, 1);  // en=0, rw=0, rs=1

    status = HAL_I2C_Master_Transmit(lcd->hi2c, LCD__SLAVE_ADDRESS(lcd), data_t, 4, 100);

    if (status ==HAL_OK)
    {
    	return true;
    }
	return false;
}

/**
 * @brief  Sends a single character to the LCD.
 * @param  lcd: Pointer to the LCD handle
 * @param  ch: Character to send
 * @retval true if command was sent successful else false.
 */
bool lcd_putchar(const i2cLcd_handle *lcd, const char ch)
{
	return lcd_sendData(lcd, ch);  // Send the character to the display
}

/**
 * @brief  Sends a string to the LCD.
 * @param  lcd: Pointer to the LCD handle
 * @param  str: Null-terminated string to display
 * @retval true if command was sent successful else false.
 */
bool lcd_puts(const i2cLcd_handle *lcd, const char *str)
{
	bool status = true;
    while (*str) status = lcd_sendData(lcd, *str++);  // Send each character in the string
    return status;
}
