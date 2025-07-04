#ifndef LCD_DISPLAY_H
#define LCD_DISPLAY_H

#include <Arduino.h>
#include <LiquidCrystal_I2C.h> // Pastikan library ini sudah terinstal di platformio.ini

// --- Definisi Pin LCD I2C ---
#define LCD_ADDRESS   0x27 // Alamat I2C umum untuk LCD 16x2 dengan modul PCF8574
#define LCD_COLUMNS   16   // Jumlah kolom LCD Anda
#define LCD_ROWS      2    // Jumlah baris LCD Anda

// Deklarasi objek LCD sebagai 'extern' agar bisa diakses dari file lain
extern LiquidCrystal_I2C lcd;

// Deklarasi fungsi inisialisasi LCD
void setupLCD();
void showWelcomeScene(); // Fungsi untuk menampilkan scene selamat datang
void showConfirmationScene(int menuIndex);
void showProcessingScene(); // Fungsi untuk menampilkan scene proses seduh

#endif // LCD_DISPLAY_H