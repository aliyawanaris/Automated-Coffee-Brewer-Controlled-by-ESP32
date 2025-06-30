#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_PCF8574.h>

// Inisialisasi objek PCF8574 dengan alamat I2C
// PASTIKAN ALAMAT INI BENAR! Umumnya 0x27 atau 0x3F.
// Jika alamat Anda benar-benar 0x21, gunakan itu.
Adafruit_PCF8574 pcf; // Akan menggunakan alamat default 0x27. Untuk 0x21, gunakan: Adafruit_PCF8574 pcf(0x21);

// Definisikan pin untuk push button dan LED
#define BUTTON_PIN P0
#define LED_PIN P4

void setup() {
  Serial.begin(115200);
  Serial.println("ESP32 dengan PCF8574");

  // Inisialisasi komunikasi I2C
  Wire.begin();

  // Inisialisasi PCF8574
  if (!pcf.begin()) {
    Serial.println("Gagal menemukan PCF8574. Periksa wiring dan alamat I2C!");
    while (1); // Berhenti jika gagal inisialisasi
  }
  Serial.println("PCF8574 ditemukan dan siap.");

  // Atur pin P0 sebagai INPUT untuk push button
  // PCF8574 adalah push-pull. Untuk input, kita harus 'menulis' HIGH ke pin
  // agar dapat 'membaca' status LOW ketika tombol ditekan (pull-down).
  // Jika Anda menggunakan resistor pull-up eksternal, Anda tidak perlu
  // menulis HIGH ke pin ini.
  pcf.pinMode(BUTTON_PIN, INPUT); // Sebagai input
  pcf.digitalWrite(BUTTON_PIN, HIGH); // Mengaktifkan pull-up internal (jika didukung) atau memastikan pin dalam keadaan tinggi

  // Atur pin P4 sebagai OUTPUT untuk LED
  pcf.pinMode(LED_PIN, OUTPUT);

  // Pastikan LED mati saat startup
  pcf.digitalWrite(LED_PIN, LOW);
}

void loop() {
  // Baca status push button
  // PCF8574 adalah inverter, jadi LOW berarti tombol ditekan jika terhubung ke GND.
  bool buttonState = pcf.digitalRead(BUTTON_PIN);

  // Jika tombol ditekan (LOW), nyalakan LED
  if (buttonState == LOW) {
    pcf.digitalWrite(LED_PIN, HIGH); // Nyalakan LED
    Serial.println("Tombol ditekan, LED ON");
  } else {
    // Jika tombol dilepas (HIGH), matikan LED
    pcf.digitalWrite(LED_PIN, LOW); // Matikan LED
    Serial.println("Tombol dilepas, LED OFF");
  }

  // Tambahkan sedikit delay agar tidak terlalu sering membaca dan menulis,
  // meskipun PCF8574 cukup cepat.
  delay(50);
}