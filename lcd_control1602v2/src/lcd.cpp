// https://www.youtube.com/watch?v=WxELHnnlBmU
// https://randomnerdtutorials.com/esp32-esp8266-i2c-lcd-arduino-ide/

#include <Wire.h>  
#include <LiquidCrystal_I2C.h>  
#include "lcd.h"

/**
 * @brief Beispiel, wie du ein ESP32 mit einem 1602-LCD-Display (I2C) programmieren 
 *     Schaltplan (mit I2C-Verbindung) 
 *     ESP32 <----> 1602-LCD mit I2C-Interface
 * Install libraries on VS-code LiquidCrystal_I2C
 * LCD (I2C)	ESP32
 * GND	GND
 * VCC	5V
 * SDA	GPIO21
 * SCL	GPIO22
 * 
 */
LiquidCrystal_I2C lcd(0x27,20,4); // set the LCD address to 0x27 for a 16 chars and 2 line display

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
  printlcd(0, 0, MESSAGE_LCD);
  printlcd(0, 1, "Done!!");

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
  lcd.setCursor (posx, posy);
  lcd.print(Text);
}
