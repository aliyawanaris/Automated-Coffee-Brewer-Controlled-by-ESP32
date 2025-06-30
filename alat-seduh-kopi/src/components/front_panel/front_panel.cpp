/*
  src/components/front_panel/front_panel.cpp - Implementasi Komponen Front Panel
  Mengelola input Push Button dan output LED pada PCF8574 kedua (0x21).
*/

#include "front_panel.h" // Include header komponen ini
#include <Wire.h>          // Diperlukan untuk komunikasi I2C oleh Adafruit_PCF8574
#include <Arduino.h>       // Untuk Serial.print, dll.

// --- Definisi Objek PCF8574 Kedua ---
Adafruit_PCF8574 pcf2;

// --- Definisi Variabel Global untuk Debounce dan Blink ---
unsigned long lastDebounceTime[4] = {0};
unsigned long debounceDelay = 50;
int lastButtonState[4] = {HIGH, HIGH, HIGH, HIGH};
int currentButtonState[4] = {HIGH, HIGH, HIGH, HIGH};

bool led1BlinkActive = false;
unsigned long lastLed1BlinkMillis = 0;
const long led1BlinkInterval = 5; // 5 ms untuk blink (sangat cepat)

// --- Definisi Variabel Global untuk Durasi LED (BARU DITAMBAHKAN) ---
bool isLed1On = false;
unsigned long led1ActiveStartTime = 0;
const long LED_ON_DURATION_MS = 1000; // Durasi LED menyala: 1 detik (1000 ms)


// --- Implementasi Fungsi ---

// Fungsi untuk menginisialisasi pin-pin front panel.
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
    while(true); // Hentikan program jika PCF8574 kedua tidak ditemukan (fatal)
  }
  Serial.println("OK!");

  // Mengatur pin-pin Push Button sebagai INPUT
  pcf2.pinMode(FP_PB1_PIN, INPUT); // P0 = Push button 1
  pcf2.pinMode(FP_PB2_PIN, INPUT); // P1 = Push button 2
  pcf2.pinMode(FP_PB3_PIN, INPUT); // P2 = Push button 3
  pcf2.pinMode(FP_PB4_PIN, INPUT); // P3 = Push button 4

  // Mengatur pin-pin LED sebagai OUTPUT
  pcf2.pinMode(FP_LED1_PIN, OUTPUT); // P4 = LED 1
  pcf2.pinMode(FP_LED2_PIN, OUTPUT); // P5 = LED 2

  // Pastikan LED mati di awal (HIGH untuk Common Anode)
  pcf2.digitalWrite(FP_LED1_PIN, HIGH);
  pcf2.digitalWrite(FP_LED2_PIN, HIGH);
  Serial.println("PCF8574 Front Panel pins configured.");
}

// Fungsi helper untuk membaca status push button dengan debounce.
bool readPushButton(int buttonPin, int buttonIndex) {
  int reading = pcf2.digitalRead(buttonPin); // Baca status pin dari PCF8574

  if (reading != lastButtonState[buttonIndex]) {
    lastDebounceTime[buttonIndex] = millis();
  }

  if ((millis() - lastDebounceTime[buttonIndex]) > debounceDelay) {
    if (reading != currentButtonState[buttonIndex]) {
      currentButtonState[buttonIndex] = reading;
      if (currentButtonState[buttonIndex] == LOW) { // Tombol ditekan (HIGH ke LOW)
        return true;
      }
    }
  }
  lastButtonState[buttonIndex] = reading;
  return false;
}

// Fungsi untuk menangani logika tombol dan LED di loop utama.
void handleFrontPanel() {
  // Baca status setiap tombol dengan debounce
  bool pb1Pressed = readPushButton(FP_PB1_PIN, 0);
  bool pb2Pressed = readPushButton(FP_PB2_PIN, 1);
  bool pb3Pressed = readPushButton(FP_PB3_PIN, 2);
  bool pb4Pressed = readPushButton(FP_PB4_PIN, 3);

  // --- Logika untuk Push Button 1 (LED 1 ON selama durasi) ---
  if (pb1Pressed) {
      Serial.println("PB1 Ditekan: Menyalakan LED 1 selama 1 detik");
      pcf2.digitalWrite(FP_LED1_PIN, LOW); // LOW = ON
      isLed1On = true; // Set status LED menjadi aktif
      led1ActiveStartTime = millis(); // Catat waktu LED mulai menyala
      led1BlinkActive = false; // Pastikan blink non-aktif
  }

  // --- Logika untuk mematikan LED 1 setelah durasi jika PB4 tidak aktif dan timer sedang berjalan ---
  else if (isLed1On && (millis() - led1ActiveStartTime >= LED_ON_DURATION_MS)) {
      Serial.println("Durasi 1 detik telah berlalu -> Mematikan LED 1 otomatis");
      pcf2.digitalWrite(FP_LED1_PIN, HIGH); // HIGH untuk mematikan LED (Common Anode)
      isLed1On = false; // Set status LED menjadi tidak aktif
  }
}
