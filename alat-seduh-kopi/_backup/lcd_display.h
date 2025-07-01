#ifndef LCD_DISPLAY_H
#define LCD_DISPLAY_H

#include <Arduino.h>
#include <LiquidCrystal_I2C.h>

// --- Definisi Pin LCD I2C ---
#define LCD_ADDRESS   0x27 // Sesuaikan jika alamat I2C Anda berbeda
#define LCD_COLUMNS   20   // <<< Pastikan ini 20
#define LCD_ROWS      4    // <<< Pastikan ini 4

// Deklarasi objek LCD sebagai 'extern' agar bisa diakses dari file lain
extern LiquidCrystal_I2C lcd;

// Deklarasi fungsi inisialisasi LCD
void setupLCD();

#endif // LCD_DISPLAY_H