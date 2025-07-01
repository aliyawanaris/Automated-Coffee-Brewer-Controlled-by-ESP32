/*
  src/components/front_panel/front_panel.cpp - Implementasi Komponen Front Panel
  Mengelola input Push Button dan output LED pada PCF8574 kedua (0x21).
  Diperbarui untuk tombol dengan default state LOW (HIGH saat ditekan).
  Fokus hanya pada Push Button 1 dan LED 1 (aktif selama tombol ditekan).
*/

#include "front_panel.h" // Include header komponen ini
#include <Wire.h>          // Diperlukan untuk komunikasi I2C oleh Adafruit_PCF8574
#include <Arduino.h>       // Untuk Serial.print, dll.

// --- Definisi Objek PCF8574 Kedua ---
Adafruit_PCF8574 pcf2;

// --- Definisi Variabel Global untuk Debounce ---
unsigned long lastDebounceTime[4] = {0};
unsigned long debounceDelay = 50;
// Inisialisasi menjadi LOW (default state saat tidak ditekan)
int lastButtonState[4] = {LOW, LOW, LOW, LOW};
int currentButtonState[4] = {LOW, LOW, LOW, LOW};


// --- Implementasi Fungsi ---

/**
 * @brief Menginisialisasi pin-pin front panel.
 * @param pcf2_address Alamat I2C PCF8574 yang digunakan untuk front panel.
 */
void setupFrontPanel(uint8_t pcf2_address) {
  Serial.print("Menginisialisasi PCF8574 (0x");
  if (pcf2_address < 16) Serial.print("0");
  Serial.print(pcf2_address, HEX);
  Serial.println(" - Front Panel)... "); // Log rapi

  if (!pcf2.begin(pcf2_address, &Wire)) {
    Serial.println("PCF8574 (0x20) Inisialisasi GAGAL!"); // Log rapi
    Serial.print("FATAL ERROR: PCF8574 (0x");
    if (pcf2_address < 16) Serial.print("0");
    Serial.print(pcf2_address, HEX);
    Serial.println(") TIDAK DITEMUKAN. Cek alamat & koneksi!"); // Log rapi
    while(true); // Hentikan program jika PCF8574 kedua tidak ditemukan (fatal)
  }
  Serial.println("PCF8574 (Front Panel) OK!"); // Log rapi

  // Mengatur pin-pin Push Button sebagai INPUT
  pcf2.pinMode(FP_PB1_PIN, INPUT); // P0 = Push button 1
  pcf2.pinMode(FP_PB2_PIN, INPUT); // P1 = Push button 2
  pcf2.pinMode(FP_PB3_PIN, INPUT); // P2 = Push button 3
  pcf2.pinMode(FP_PB4_PIN, INPUT); // P3 = Push button 4

  // Mengatur pin-pin LED sebagai OUTPUT
  pcf2.pinMode(FP_LED1_PIN, OUTPUT); // P4 = LED 1
  pcf2.pinMode(FP_LED2_PIN, OUTPUT); // P5 = LED 2

  // Pastikan semua LED mati di awal (HIGH untuk Common Anode)
  pcf2.digitalWrite(FP_LED1_PIN, HIGH);
  pcf2.digitalWrite(FP_LED2_PIN, HIGH);
  Serial.println("PCF8574 Front Panel pins configured."); // Log rapi
}

/**
 * @brief Membaca status push button dengan debounce.
 * @param buttonPin Pin PCF8574 yang terhubung ke tombol.
 * @param buttonIndex Indeks tombol (0-3) untuk array status.
 * @return true jika tombol baru saja ditekan (transisi LOW ke HIGH), false jika tidak.
 */
bool readPushButton(int buttonPin, int buttonIndex) {
  int reading = pcf2.digitalRead(buttonPin); // Baca status pin dari PCF8574

  if (reading != lastButtonState[buttonIndex]) {
    lastDebounceTime[buttonIndex] = millis();
  }

  if ((millis() - lastDebounceTime[buttonIndex]) > debounceDelay) {
    if (reading != currentButtonState[buttonIndex]) {
      currentButtonState[buttonIndex] = reading;
      // Deteksi tekanan saat HIGH (transisi dari LOW ke HIGH)
      if (currentButtonState[buttonIndex] == HIGH) { // Tombol baru saja ditekan
        Serial.print("Tombol PB");
        Serial.print(buttonIndex + 1);
        Serial.println(" ditekan (HIGH)"); // Log rapi
        return true; // Mengembalikan true hanya saat transisi LOW ke HIGH
      } else {
        Serial.print("Tombol PB");
        Serial.print(buttonIndex + 1);
        Serial.println(" dilepas (LOW)"); // Log rapi
      }
    }
  }
  lastButtonState[buttonIndex] = reading;
  return false;
}

/**
 * @brief Menangani pembacaan status tombol dan kontrol LED.
 * Fungsi ini harus dipanggil berulang kali di loop() Arduino.
 */
void handleFrontPanel() {
  // Panggil readPushButton untuk SEMUA pin yang didefinisikan sebagai tombol.
  // Ini penting agar mekanisme debounce terus bekerja dan 'currentButtonState'
  // selalu diperbarui dengan status stabil dari setiap tombol.
  readPushButton(FP_PB1_PIN, 0);
  readPushButton(FP_PB2_PIN, 1);
  readPushButton(FP_PB3_PIN, 2);
  readPushButton(FP_PB4_PIN, 3);

  // --- Logika untuk Push Button 1 (LED 1 aktif SELAMA ditekan) ---
  // Kita langsung memeriksa status stabil 'currentButtonState[0]' (indeks 0 untuk PB1).
  if (currentButtonState[0] == HIGH) { // Jika PB1 sedang ditekan (state HIGH)
      pcf2.digitalWrite(FP_LED1_PIN, LOW); // LOW = ON (untuk Common Anode LED)
  } else { // Jika PB1 tidak ditekan (state LOW)
      pcf2.digitalWrite(FP_LED1_PIN, HIGH); // HIGH = OFF
  }

  // --- Pastikan LED2 selalu mati ---
  // Ini memastikan pin P5 selalu OFF jika tidak ada logika khusus untuknya.
  pcf2.digitalWrite(FP_LED2_PIN, HIGH); // HIGH = OFF
}
