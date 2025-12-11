#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "lcd.h"

/**
 * @brief Implementation an ESP32 with a 1602 LCD display (I2C)  and the following aspects:
 *     - Circuit diagram (with I2C connection)
 *     - ESP32 device <----> 1602 LCD device  with I2C interface
 *     - Requirement: Installation of the LiquidCrystal_I2C libraries on VS Code
 * LCD (I2C) / ESP32
 * GND	GND
 * VCC	5V
 * SDA	GPIO21
 * SCL	GPIO22
 *
 */
LiquidCrystal_I2C lcd(0x27, 16, 2); // set the LCD address to 0x27 for a 16 chars and 2 line display

/**
 * @brief Function to initialize the LCD display
 *
 * @param non
 *
 */
void LCD_init()
{
  lcd.init();
  lcd.backlight();
  lcd.clear();
}

/**
 * @brief Function to prints a given String to an LCD (Display) at a specified position
 *
 * @param posx sets the cursor position on the LCD to the position 1
 * @param posy sets the cursor position on the LCD to the position 2
 * @param Text message to print
 */
void printlcd(uint8_t posx, uint8_t posy, String Text)
{
  lcd.setCursor(posx, posy);
  lcd.print(Text);
}
