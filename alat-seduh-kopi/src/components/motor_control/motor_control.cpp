#include "motor_control.h"
#include <Wire.h> // Diperlukan untuk komunikasi I2C

// Definisi Objek PCF8574
Adafruit_PCF8574 pcf;

/**
 * @brief Menginisialisasi pin-pin kontrol motor pada PCF8574.
 * @param pcf_address Alamat I2C PCF8574 yang digunakan untuk motor control.
 */
void setupMotorControl(uint8_t pcf_address) {
  Serial.print("Menginisialisasi PCF8574 (0x");
  if (pcf_address < 16) Serial.print("0");
  Serial.print(pcf_address, HEX);
  Serial.println(" - Motor Control)... ");

  if (!pcf.begin(pcf_address, &Wire)) {
    Serial.println("PCF8574 (0x20) Inisialisasi GAGAL!");
    Serial.print("FATAL ERROR: PCF8574 (0x");
    if (pcf_address < 16) Serial.print("0");
    Serial.print(pcf_address, HEX);
    Serial.println(") TIDAK DITEMUKAN. Cek alamat & koneksi!");
    while (true); // Hentikan program jika PCF8574 tidak ditemukan (fatal)
  }
  Serial.println("PCF8574 (Motor Control) OK!");

  // --- Pin Setup untuk Motor Dinamo 1 | Storage 1 ---
  pcf.pinMode(MOTOR_STORAGE_1_IN1_PIN, OUTPUT);
  pcf.pinMode(MOTOR_STORAGE_1_IN2_PIN, OUTPUT);
  pcf.digitalWrite(MOTOR_STORAGE_1_IN1_PIN, LOW); // Pastikan motor OFF di awal
  pcf.digitalWrite(MOTOR_STORAGE_1_IN2_PIN, LOW);

  // --- Pin Setup untuk Motor Dinamo 2 | Storage 2 ---
  pcf.pinMode(MOTOR_STORAGE_2_IN1_PIN, OUTPUT);
  pcf.pinMode(MOTOR_STORAGE_2_IN2_PIN, OUTPUT);
  pcf.digitalWrite(MOTOR_STORAGE_2_IN1_PIN, LOW); // Pastikan motor OFF di awal
  pcf.digitalWrite(MOTOR_STORAGE_2_IN2_PIN, LOW);

  // --- Pin Setup untuk Motor Dinamo 3 | Storage 3 ---
  pcf.pinMode(MOTOR_STORAGE_3_IN1_PIN, OUTPUT);
  pcf.pinMode(MOTOR_STORAGE_3_IN2_PIN, OUTPUT);
  pcf.digitalWrite(MOTOR_STORAGE_3_IN1_PIN, LOW); // Pastikan motor OFF di awal
  pcf.digitalWrite(MOTOR_STORAGE_3_IN2_PIN, LOW);

  // --- Pin Setup untuk Motor Dinamo 4 | Mixer---
  pcf.pinMode(MOTOR_MIXER_IN1_PIN, OUTPUT);
  pcf.pinMode(MOTOR_MIXER_IN2_PIN, OUTPUT);
  pcf.digitalWrite(MOTOR_MIXER_IN1_PIN, LOW); // Pastikan motor OFF di awal
  pcf.digitalWrite(MOTOR_MIXER_IN2_PIN, LOW);

  // --- BARU: Pin Setup untuk Motor Pump 2 (GPIO25) ---
  pinMode(MOTOR_PUMP_GALON_RELAY_PIN, OUTPUT);
  digitalWrite(MOTOR_PUMP_GALON_RELAY_PIN, HIGH);
  pinMode(MOTOR_PUMP_HOT_WATER_RELAY_PIN, OUTPUT);
  digitalWrite(MOTOR_PUMP_HOT_WATER_RELAY_PIN, HIGH);
  pinMode(MOTOR_PUMP_SEDUH_KOPI_RELAY_PIN, OUTPUT);
  digitalWrite(MOTOR_PUMP_SEDUH_KOPI_RELAY_PIN, HIGH);

  Serial.println("Motor Control pins configured (including Motor Pump 2 on GPIO25).");
}

// --- Implementasi Fungsi Kontrol Motor Dinamo 1 ---
void motor_storage_1_start() {
  Serial.println("Motor 1 START");
  pcf.digitalWrite(MOTOR_STORAGE_1_IN1_PIN, HIGH);
  pcf.digitalWrite(MOTOR_STORAGE_1_IN2_PIN, LOW);
}

void motor_storage_1_stop() {
  Serial.println("Motor 1 STOP");
  pcf.digitalWrite(MOTOR_STORAGE_1_IN1_PIN, LOW);
  pcf.digitalWrite(MOTOR_STORAGE_1_IN2_PIN, LOW);
}

void motor_storage_2_start() {
  Serial.println("Motor 2 START");
  pcf.digitalWrite(MOTOR_STORAGE_2_IN1_PIN, HIGH);
  pcf.digitalWrite(MOTOR_STORAGE_2_IN2_PIN, LOW);
}

void motor_storage_2_stop() {
  Serial.println("Motor 2 STOP");
  pcf.digitalWrite(MOTOR_STORAGE_2_IN1_PIN, LOW);
  pcf.digitalWrite(MOTOR_STORAGE_2_IN2_PIN, LOW);
}

void motor_storage_3_start() {
  Serial.println("Motor 3 START");
  pcf.digitalWrite(MOTOR_STORAGE_3_IN1_PIN, HIGH);
  pcf.digitalWrite(MOTOR_STORAGE_3_IN2_PIN, LOW);
}

void motor_storage_3_stop() {
  Serial.println("Motor 3 STOP");
  pcf.digitalWrite(MOTOR_STORAGE_3_IN1_PIN, LOW);
  pcf.digitalWrite(MOTOR_STORAGE_3_IN2_PIN, LOW);
}

// --- Implementasi Fungsi Kontrol Motor Dinamo 4 ---
void motor_mixer_start() {
  Serial.println("Motor 4 START");
  pcf.digitalWrite(MOTOR_MIXER_IN1_PIN, HIGH);
  pcf.digitalWrite(MOTOR_MIXER_IN2_PIN, LOW);
}

void motor_mixer_stop() {
  Serial.println("Motor 4 STOP");
  pcf.digitalWrite(MOTOR_MIXER_IN1_PIN, LOW);
  pcf.digitalWrite(MOTOR_MIXER_IN2_PIN, LOW);
}

// Fungsi untuk Motor Pump 1 tidak ada di sini karena dikelola di order_coffee.cpp

void motor_pump_galon_start() {
  Serial.println("Motor Pump Galon ON (GPIO25)");
  digitalWrite(MOTOR_PUMP_GALON_RELAY_PIN, LOW); // Asumsi LOW = ON untuk relay
}

void motor_pump_galon_stop() {
  Serial.println("Motor Pump Galon OFF (GPIO25)");
  digitalWrite(MOTOR_PUMP_GALON_RELAY_PIN, HIGH); // Asumsi HIGH = OFF untuk relay
}

void motor_pump_hot_water_start() {
  Serial.println("Motor Pump Hot Water ON (GPIO25)");
  digitalWrite(MOTOR_PUMP_HOT_WATER_RELAY_PIN, LOW); // Asumsi LOW = ON untuk relay
}

void motor_pump_hot_water_stop() {
  Serial.println("Motor Pump Hot Water OFF (GPIO25)");
  digitalWrite(MOTOR_PUMP_HOT_WATER_RELAY_PIN, HIGH); // Asumsi HIGH = OFF untuk relay
}

void motor_pump_seduh_kopi_start() {
  Serial.println("Motor Pump Seduh Kopi ON (GPIO33)");
  digitalWrite(MOTOR_PUMP_SEDUH_KOPI_RELAY_PIN, LOW); // Asumsi LOW = ON untuk relay
}

void motor_pump_seduh_kopi_stop() {
  Serial.println("Motor Pump Seduh Kopi OFF (GPIO33)");
  digitalWrite(MOTOR_PUMP_SEDUH_KOPI_RELAY_PIN, HIGH); // Asumsi HIGH = OFF untuk relay
}