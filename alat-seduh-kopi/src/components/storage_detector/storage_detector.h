#ifndef STORAGE_DETECTOR_H
#define STORAGE_DETECTOR_H

#include <Arduino.h>

// --- Deklarasi Pin (extern agar bisa diakses dari file lain) ---
extern const int SD_TRIG_PIN_1;
extern const int SD_ECHO_PIN_1;
extern const int SD_TRIG_PIN_2;
extern const int SD_ECHO_PIN_2;
extern const int SD_TRIG_PIN_3;
extern const int SD_ECHO_PIN_3;

// --- Deklarasi Fungsi Modul ---
void storage_detector_init_all_sensors();
long storage_detector_get_distance(int trigPin, int echoPin);

// --- Deklarasi Fungsi untuk Status Teks Kopi ---
// Fungsi general untuk mendapatkan status stok kopi
String getCoffeeStockStatus(int trigPin, int echoPin);

// Fungsi spesifik untuk setiap sensor kopi
String getCoffee1StockStatus(); // <--- Diperbarui
String getCoffee2StockStatus(); // <--- Diperbarui
String getCoffee3StockStatus(); // <--- Diperbarui

#endif // STORAGE_DETECTOR_H