/*
  src/components/front_panel/front_panel.h - Header untuk Komponen Front Panel
  Mendefinisikan antarmuka (deklarasi) untuk kontrol Push Button dan LED
  pada PCF8574 kedua (0x21).
  Definisi semua pin tetap ada, logika kontrol difokuskan pada PB1 & LED1.
*/

#ifndef FRONT_PANEL_H
#define FRONT_PANEL_H

#include <Arduino.h>
#include <Adafruit_PCF8574.h> // Diperlukan untuk objek PCF8574

// --- Deklarasi Objek PCF8574 Kedua ---
extern Adafruit_PCF8574 pcf2;

// --- Definisi Pin pada PCF8574 Kedua (0x21) ---
// Push Button Inputs
const int FP_PB1_PIN = 0; // P0 = Push button 1
const int FP_PB2_PIN = 1; // P1 = Push button 2
const int FP_PB3_PIN = 2; // P2 = Push button 3
const int FP_PB4_PIN = 3; // P3 = Push button 4

// LED Outputs
const int FP_LED1_PIN = 4; // P4 = LED 1
const int FP_LED2_PIN = 5; // P5 = LED 2

// --- Deklarasi Variabel Global untuk Debounce ---
extern unsigned long lastDebounceTime[4];
extern unsigned long debounceDelay;
extern int lastButtonState[4];
extern int currentButtonState[4]; // Ini akan menyimpan status stabil tombol (LOW/HIGH)

// --- Prototipe Fungsi ---
/**
 * @brief Menginisialisasi pin-pin Front Panel (PCF8574).
 * @param pcf2_address Alamat I2C PCF8574 yang digunakan untuk front panel.
 */
void setupFrontPanel(uint8_t pcf2_address);

/**
 * @brief Menangani pembacaan status tombol dan kontrol LED.
 * Fungsi ini harus dipanggil berulang kali di loop() Arduino.
 */
void handleFrontPanel();

/**
 * @brief Membaca status push button dengan debounce.
 * @param buttonPin Pin PCF8574 yang terhubung ke tombol.
 * @param buttonIndex Indeks tombol (0-3) untuk array status.
 * @return true jika tombol baru saja ditekan (transisi LOW ke HIGH), false jika tidak.
 */
bool readPushButton(int buttonPin, int buttonIndex);

#endif // FRONT_PANEL_H
