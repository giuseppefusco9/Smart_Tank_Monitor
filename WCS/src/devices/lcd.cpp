#include "lcd.h"
#include "config.h"
#include <LiquidCrystal_I2C.h>

// Create LCD object
LiquidCrystal_I2C lcd(LCD_I2C_ADDRESS, LCD_COLS, LCD_ROWS);

Lcd::Lcd() {
    // Initialize LCD
    lcd.init();
    delay(100);  // Give LCD time to initialize
    lcd.backlight();
    delay(50);
    lcd.clear();
    
    modeMessage = "";
    percMessage = "";
}

void Lcd::writeModeMessage(String message) {
    modeMessage = message;
    refresh();
}

void Lcd::writePercMessage(String message) {
    percMessage = message;
    refresh();
}

void Lcd::refresh() {
    lcd.clear();
    
    // Line 1: Mode message
    lcd.setCursor(0, 0);
    lcd.print("Mode: " + modeMessage);
    
    // Line 2: Percentage message
    lcd.setCursor(0, 1);
    lcd.print(percMessage);
}