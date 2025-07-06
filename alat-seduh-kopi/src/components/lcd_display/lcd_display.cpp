#include "lcd_display.h" // Sertakan header file ini
#include <Wire.h>        // Library untuk komunikasi I2C

// Definisi objek LCD
LiquidCrystal_I2C lcd(LCD_ADDRESS, LCD_COLUMNS, LCD_ROWS);

// // Fungsi untuk menginisialisasi LCD
// void setupLCD() {
//   Wire.begin(); // Memulai komunikasi I2C (penting untuk LCD I2C)
//   lcd.init();   // Inisialisasi LCD
//   lcd.backlight(); // Nyalakan lampu latar LCD
//   lcd.clear();  // Bersihkan layar
//   lcd.setCursor(0, 0);
//   lcd.print("Coffee Machine");
//   lcd.setCursor(0, 1);
//   lcd.print("Initializing...");
//   Serial.println("LCD terinisialisasi.");
// }