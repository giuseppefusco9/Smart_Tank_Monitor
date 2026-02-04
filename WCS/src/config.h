#ifndef __CONFIG__
#define __CONFIG__

// ===== Pin Configuration =====
#define SERVO_PIN 3          // Servo motor for valve control
#define BUTTON_PIN 9         // Mode switch button
#define POT_PIN A0           // Potentiometer for manual control

// ===== Serial Configuration =====
#define SERIAL_BAUD 9600   // Match CUS serial configuration

// ===== System Parameters =====
#define VALVE_MIN 0          // Minimum valve percentage
#define VALVE_MAX 100        // Maximum valve percentage
#define POT_MIN 0            // Potentiometer ADC min
#define POT_MAX 1023         // Potentiometer ADC max
#define SERVO_MIN_ANGLE 0    // Servo minimum angle (0% = closed)
#define SERVO_MAX_ANGLE 90   // Servo maximum angle (100% = open) - Per assignment specs

// ===== Update Intervals =====
#define MANUAL_UPDATE_INTERVAL 500  // ms between potentiometer updates
#define SERIAL_CHECK_INTERVAL 50    // ms between serial message checks

// ===== LCD Configuration =====
#define LCD_I2C_ADDRESS 0x27  // I2C address for LCD (try 0x3F if this doesn't work)
#define LCD_COLS 16           // LCD columns
#define LCD_ROWS 2            // LCD rows

// ===== Debug Configuration =====
#define DEBUG_ENABLED true    // Enable/disable debug logging

#endif