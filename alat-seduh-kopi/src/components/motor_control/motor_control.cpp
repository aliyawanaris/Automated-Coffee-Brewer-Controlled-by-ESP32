#include "motor_control.h"
#include <Wire.h> // Diperlukan untuk komunikasi I2C

// Definisi Objek PCF8574
Adafruit_PCF8574 pcf;

/**
 * @brief Menginisialisasi pin-pin kontrol motor pada PCF8574 dan GPIO.
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

  // --- Pin Setup untuk Motor Dinamo (melalui PCF8574 & LM298N) ---
  // LM298N #1 (Storage 2 & 3)
  pcf.pinMode(MOTOR_STORAGE_2_IN1_PIN, OUTPUT);
  pcf.pinMode(MOTOR_STORAGE_2_IN2_PIN, OUTPUT);
  pcf.digitalWrite(MOTOR_STORAGE_2_IN1_PIN, LOW);
  pcf.digitalWrite(MOTOR_STORAGE_2_IN2_PIN, LOW);

  pcf.pinMode(MOTOR_STORAGE_3_IN1_PIN, OUTPUT);
  pcf.pinMode(MOTOR_STORAGE_3_IN2_PIN, OUTPUT);
  pcf.digitalWrite(MOTOR_STORAGE_3_IN1_PIN, LOW);
  pcf.digitalWrite(MOTOR_STORAGE_3_IN2_PIN, LOW);

  // LM298N #2 (Storage 1 & Mixer)
  pcf.pinMode(MOTOR_STORAGE_1_IN1_PIN, OUTPUT);
  pcf.pinMode(MOTOR_STORAGE_1_IN2_PIN, OUTPUT);
  pcf.digitalWrite(MOTOR_STORAGE_1_IN1_PIN, LOW);
  pcf.digitalWrite(MOTOR_STORAGE_1_IN2_PIN, LOW);

  pcf.pinMode(MOTOR_MIXER_IN1_PIN, OUTPUT);
  pcf.pinMode(MOTOR_MIXER_IN2_PIN, OUTPUT);
  pcf.digitalWrite(MOTOR_MIXER_IN1_PIN, LOW);
  pcf.digitalWrite(MOTOR_MIXER_IN2_PIN, LOW);

  // --- Pin Setup untuk ENA/ENB LM298N (langsung ke GPIO ESP32) ---
  pinMode(LM298N1_ENA_PIN, OUTPUT);
  digitalWrite(LM298N1_ENA_PIN, LOW); // Pastikan OFF di awal
  pinMode(LM298N1_ENB_PIN, OUTPUT);
  digitalWrite(LM298N1_ENB_PIN, LOW); // Pastikan OFF di awal

  pinMode(LM298N2_ENA_PIN, OUTPUT);
  digitalWrite(LM298N2_ENA_PIN, LOW); // Pastikan OFF di awal
  pinMode(LM298N2_ENB_PIN, OUTPUT);
  digitalWrite(LM298N2_ENB_PIN, LOW); // Pastikan OFF di awal

  // --- Pin Setup untuk Motor Pump (langsung ke GPIO ESP32) ---
  pinMode(MOTOR_PUMP_GALON_RELAY_PIN, OUTPUT);
  digitalWrite(MOTOR_PUMP_GALON_RELAY_PIN, HIGH); // Asumsi HIGH = OFF untuk relay

  pinMode(MOTOR_PUMP_HOT_WATER_RELAY_PIN, OUTPUT);
  digitalWrite(MOTOR_PUMP_HOT_WATER_RELAY_PIN, HIGH); // Asumsi HIGH = OFF untuk relay

  pinMode(MOTOR_PUMP_SEDUH_KOPI_RELAY_PIN, OUTPUT);
  digitalWrite(MOTOR_PUMP_SEDUH_KOPI_RELAY_PIN, HIGH); // Asumsi HIGH = OFF untuk relay

  Serial.println("Motor Control pins configured (including LM298N ENA/ENB & all pumps).");
}

// --- Implementasi Fungsi Kontrol Motor Dinamo ---
// Catatan: Nilai speed 0-255

// Motor Storage 1 (LM298N #2, ENA: GPIO17)
void motor_storage_1_start(int speed) {
  speed = constrain(speed, 0, 255); // Pastikan speed dalam rentang 0-255
  Serial.print("Motor Storage 1 START (Speed: "); Serial.print(speed); Serial.println(")");
  pcf.digitalWrite(MOTOR_STORAGE_1_IN1_PIN, HIGH);
  pcf.digitalWrite(MOTOR_STORAGE_1_IN2_PIN, LOW);
  analogWrite(LM298N2_ENA_PIN, speed); // Kontrol kecepatan via ENA
}

void motor_storage_1_stop() {
  Serial.println("Motor Storage 1 STOP");
  pcf.digitalWrite(MOTOR_STORAGE_1_IN1_PIN, LOW);
  pcf.digitalWrite(MOTOR_STORAGE_1_IN2_PIN, LOW);
  analogWrite(LM298N2_ENA_PIN, 0); // Matikan motor via ENA
}

// Motor Storage 2 (LM298N #1, ENA: GPIO4)
void motor_storage_2_start(int speed) {
  speed = constrain(speed, 0, 255);
  Serial.print("Motor Storage 2 START (Speed: "); Serial.print(speed); Serial.println(")");
  pcf.digitalWrite(MOTOR_STORAGE_2_IN1_PIN, HIGH);
  pcf.digitalWrite(MOTOR_STORAGE_2_IN2_PIN, LOW);
  analogWrite(LM298N1_ENA_PIN, speed); // Kontrol kecepatan via ENA
}

void motor_storage_2_stop() {
  Serial.println("Motor Storage 2 STOP");
  pcf.digitalWrite(MOTOR_STORAGE_2_IN1_PIN, LOW);
  pcf.digitalWrite(MOTOR_STORAGE_2_IN2_PIN, LOW);
  analogWrite(LM298N1_ENA_PIN, 0); // Matikan motor via ENA
}

// Motor Storage 3 (LM298N #1, ENB: GPIO16)
void motor_storage_3_start(int speed) {
  speed = constrain(speed, 0, 255);
  Serial.print("Motor Storage 3 START (Speed: "); Serial.print(speed); Serial.println(")");
  pcf.digitalWrite(MOTOR_STORAGE_3_IN1_PIN, HIGH);
  pcf.digitalWrite(MOTOR_STORAGE_3_IN2_PIN, LOW);
  analogWrite(LM298N1_ENB_PIN, speed); // Kontrol kecepatan via ENB
}

void motor_storage_3_stop() {
  Serial.println("Motor Storage 3 STOP");
  pcf.digitalWrite(MOTOR_STORAGE_3_IN1_PIN, LOW);
  pcf.digitalWrite(MOTOR_STORAGE_3_IN2_PIN, LOW);
  analogWrite(LM298N1_ENB_PIN, 0); // Matikan motor via ENB
}

// Motor Mixer (LM298N #2, ENB: GPIO12)
void motor_mixer_start(int speed) {
  speed = constrain(speed, 0, 255);
  Serial.print("Motor Mixer START (Speed: "); Serial.print(speed); Serial.println(")");
  pcf.digitalWrite(MOTOR_MIXER_IN1_PIN, HIGH);
  pcf.digitalWrite(MOTOR_MIXER_IN2_PIN, LOW);
  analogWrite(LM298N2_ENB_PIN, speed); // Kontrol kecepatan via ENB
}

void motor_mixer_stop() {
  Serial.println("Motor Mixer STOP");
  pcf.digitalWrite(MOTOR_MIXER_IN1_PIN, LOW);
  pcf.digitalWrite(MOTOR_MIXER_IN2_PIN, LOW);
  analogWrite(LM298N2_ENB_PIN, 0); // Matikan motor via ENB
}

// --- Implementasi Fungsi Kontrol Motor Pump (Relay langsung ke GPIO) ---
void motor_pump_galon_start() {
  Serial.println("Motor Pump Galon ON (GPIO32)");
  digitalWrite(MOTOR_PUMP_GALON_RELAY_PIN, LOW); // Asumsi LOW = ON untuk relay
}

void motor_pump_galon_stop() {
  Serial.println("Motor Pump Galon OFF (GPIO32)");
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