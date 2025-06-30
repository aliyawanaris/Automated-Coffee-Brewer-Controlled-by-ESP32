/*
  src/components/front_panel/front_panel.cpp - Implementasi Komponen Front Panel
  Mengelola input Push Button dan output LED pada PCF8574 kedua (0x21).
  Diperbarui untuk tombol dengan default state LOW (HIGH saat ditekan).
  Fokus hanya pada Push Button 1 dan LED 1 (aktif selama tombol ditekan).
  Menambahkan logging status tombol ke Serial Monitor.
*/

#include "front_panel.h" // Include header komponen ini
#include <Wire.h>          // Diperlukan untuk komunikasi I2C oleh Adafruit_PCF8574
#include <Arduino.h>       // Untuk Serial.print, dll.

// --- Definisi Objek PCF8574 Kedua ---
Adafruit_PCF8574 pcf2;

// --- Definisi Variabel Global untuk Debounce ---
unsigned long lastDebounceTime[4] = {0};
unsigned long debounceDelay = 50;
int lastButtonState[4] = {LOW, LOW, LOW, LOW}; // Default state LOW
int currentButtonState[4] = {LOW, LOW, LOW, LOW}; // Default state LOW


// --- Implementasi Fungsi ---

void setupFrontPanel(uint8_t pcf2_address) {
  Serial.print("Menginisialisasi PCF8574 (0x");
  if (pcf2_address < 16) Serial.print("0");
  Serial.print(pcf2_address, HEX);
  Serial.print(" - Front Panel)... ");

  if (!pcf2.begin(pcf2_address, &Wire)) {
    Serial.println("GAGAL!");
    Serial.print("FATAL ERROR: PCF8574 (0x");
    if (pcf2_address < 16) Serial.print("0");
    Serial.print(pcf2_address, HEX);
    Serial.println(") TIDAK DITEMUKAN. Cek alamat & koneksi!");
    while(true);
  }
  Serial.println("OK!");

  pcf2.pinMode(FP_PB1_PIN, INPUT);
  pcf2.pinMode(FP_PB2_PIN, INPUT);
  pcf2.pinMode(FP_PB3_PIN, INPUT);
  pcf2.pinMode(FP_PB4_PIN, INPUT);

  pcf2.pinMode(FP_LED1_PIN, OUTPUT);
  pcf2.pinMode(FP_LED2_PIN, OUTPUT);

  pcf2.digitalWrite(FP_LED1_PIN, HIGH); // OFF
  pcf2.digitalWrite(FP_LED2_PIN, HIGH); // OFF
  Serial.println("PCF8574 Front Panel pins configured.");
}

// Fungsi helper untuk membaca status push button dengan debounce.
// Mengembalikan TRUE jika button baru saja ditekan (transisi LOW ke HIGH)
// Juga memperbarui currentButtonState untuk cek status terus-menerus.
bool readPushButton(int buttonPin, int buttonIndex) {
  int reading = pcf2.digitalRead(buttonPin);

  if (reading != lastButtonState[buttonIndex]) {
    lastDebounceTime[buttonIndex] = millis();
  }

  if ((millis() - lastDebounceTime[buttonIndex]) > debounceDelay) {
    if (reading != currentButtonState[buttonIndex]) {
      // *** PERUBAHAN DI SINI: Logging perubahan status tombol ***
      Serial.print("Tombol PB");
      Serial.print(buttonIndex + 1); // Indeks 0 = PB1, dst.
      Serial.print(" berubah ke: ");
      Serial.println(reading == HIGH ? "HIGH" : "LOW");
      // *********************************************************

      currentButtonState[buttonIndex] = reading; // Perbarui status stabil
      if (currentButtonState[buttonIndex] == HIGH) { // Tombol baru saja ditekan (LOW ke HIGH)
        return true;
      }
    }
  }
  lastButtonState[buttonIndex] = reading;
  return false;
}

void handleFrontPanel() {
  // Panggil readPushButton untuk SEMUA pin yang didefinisikan sebagai tombol.
  // Ini penting agar mekanisme debounce terus bekerja dan 'currentButtonState'
  // selalu diperbarui dengan status stabil dari setiap tombol.
  bool pb1JustPressed = readPushButton(FP_PB1_PIN, 0); // PB1
  bool pb2JustPressed = readPushButton(FP_PB2_PIN, 1); // PB2
  bool pb3JustPressed = readPushButton(FP_PB3_PIN, 2); // PB3
  bool pb4JustPressed = readPushButton(FP_PB4_PIN, 3); // PB4

  // --- Logika untuk Push Button 1 (LED 1 aktif SELAMA ditekan) ---
  if (currentButtonState[0] == HIGH) { // Jika PB1 sedang ditekan (state HIGH)
      pcf2.digitalWrite(FP_LED1_PIN, LOW); // LOW = ON (untuk Common Anode LED)
  } else { // Jika PB1 tidak ditekan (state LOW)
      pcf2.digitalWrite(FP_LED1_PIN, HIGH); // HIGH = OFF
  }

  // --- Pastikan LED2 selalu mati (jika tidak ada logika khusus) ---
  pcf2.digitalWrite(FP_LED2_PIN, HIGH); // HIGH = OFF
}
