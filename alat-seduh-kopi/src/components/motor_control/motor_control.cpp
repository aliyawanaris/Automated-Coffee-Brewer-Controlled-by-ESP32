#include "motor_control.h"
#include <Wire.h> // Diperlukan untuk komunikasi I2C

// Definisi Objek PCF8574
Adafruit_PCF8574 pcf;

// --- Deklarasi variabel status global untuk motor/pompa ---
// Variabel ini dideklarasikan sebagai 'extern' di motor_control.h
// dan didefinisikan di sini. Mereka digunakan untuk melacak status
// ON/OFF setiap aktuator untuk keperluan logging atau tampilan UI.
bool motorPumpGalonActive = false;
bool motorPumpHotWaterActive = false;
bool motorMixerActive = false;
bool motorPumpSeduhKopiActive = false;
bool motorStorage1Active = false;
bool motorStorage2Active = false;
bool motorStorage3Active = false;

/**
 * @brief Menginisialisasi pin-pin kontrol motor pada PCF8574 dan GPIO.
 * @param pcf_address Alamat I2C PCF8574 yang digunakan untuk motor control.
 */
void setupMotorControl(uint8_t pcf_address) {
    Serial.print("[MOTOR_CONTROL] Menginisialisasi PCF8574 (0x");
    if (pcf_address < 16) Serial.print("0");
    Serial.print(pcf_address, HEX);
    Serial.println(" - Motor Control)... ");

    if (!pcf.begin(pcf_address, &Wire)) {
        Serial.println("[MOTOR_CONTROL] FATAL ERROR: PCF8574 (0x" + String(pcf_address, HEX) + ") TIDAK DITEMUKAN. Cek alamat & koneksi!");
        while (true); // Hentikan program jika PCF8574 tidak ditemukan (fatal)
    }
    Serial.println("[MOTOR_CONTROL] PCF8574 (Motor Control) OK!");

    // --- Pin Setup untuk Motor Dinamo (melalui PCF8574 & LM298N) ---
    Serial.println("[MOTOR_CONTROL] Mengatur pin motor dinamo via PCF8574...");
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
    Serial.println("[MOTOR_CONTROL] Pin motor dinamo dikonfigurasi.");

    // --- Pin Setup untuk ENA/ENB LM298N (langsung ke GPIO ESP32) ---
    Serial.println("[MOTOR_CONTROL] Mengatur pin ENA/ENB LM298N via GPIO...");
    pinMode(LM298N1_ENA_PIN, OUTPUT);
    digitalWrite(LM298N1_ENA_PIN, LOW); // Pastikan OFF di awal
    pinMode(LM298N1_ENB_PIN, OUTPUT);
    digitalWrite(LM298N1_ENB_PIN, LOW); // Pastikan OFF di awal

    pinMode(LM298N2_ENA_PIN, OUTPUT);
    digitalWrite(LM298N2_ENA_PIN, LOW); // Pastikan OFF di awal
    pinMode(LM298N2_ENB_PIN, OUTPUT);
    digitalWrite(LM298N2_ENB_PIN, LOW); // Pastikan OFF di awal
    Serial.println("[MOTOR_CONTROL] Pin ENA/ENB LM298N dikonfigurasi.");

    // --- Pin Setup untuk Motor Pump (Relay langsung ke GPIO ESP32) ---
    Serial.println("[MOTOR_CONTROL] Mengatur pin pompa (relay) via GPIO...");
    pinMode(MOTOR_PUMP_GALON_RELAY_PIN, OUTPUT);
    digitalWrite(MOTOR_PUMP_GALON_RELAY_PIN, HIGH); // Asumsi HIGH = OFF untuk relay
    motorPumpGalonActive = false; // Inisialisasi status

    pinMode(MOTOR_PUMP_HOT_WATER_RELAY_PIN, OUTPUT);
    digitalWrite(MOTOR_PUMP_HOT_WATER_RELAY_PIN, HIGH); // Asumsi HIGH = OFF untuk relay
    motorPumpHotWaterActive = false; // Inisialisasi status

    pinMode(MOTOR_PUMP_SEDUH_KOPI_RELAY_PIN, OUTPUT);
    digitalWrite(MOTOR_PUMP_SEDUH_KOPI_RELAY_PIN, HIGH); // Asumsi HIGH = OFF untuk relay
    motorPumpSeduhKopiActive = false; // Inisialisasi status
    Serial.println("[MOTOR_CONTROL] Pin pompa dikonfigurasi.");

    Serial.println("[MOTOR_CONTROL] Semua pin kontrol motor berhasil dikonfigurasi.");
}

// --- Implementasi Fungsi Kontrol Motor Dinamo ---
// Catatan: Nilai speed 0-255

// Motor Storage 1 (LM298N #2, ENA: GPIO17)
void motor_storage_1_start(int speed) {
    speed = constrain(speed, 0, 255); // Pastikan speed dalam rentang 0-255
    Serial.println("[MOTOR_CONTROL] Motor Storage 1 START (Speed: " + String(speed) + ") [ENA: GPIO17]");
    pcf.digitalWrite(MOTOR_STORAGE_1_IN1_PIN, HIGH);
    pcf.digitalWrite(MOTOR_STORAGE_1_IN2_PIN, LOW);
    analogWrite(LM298N2_ENA_PIN, speed); // Kontrol kecepatan via ENA
    motorStorage1Active = true; // Update status
}

void motor_storage_1_stop() {
    Serial.println("[MOTOR_CONTROL] Motor Storage 1 STOP");
    pcf.digitalWrite(MOTOR_STORAGE_1_IN1_PIN, LOW);
    pcf.digitalWrite(MOTOR_STORAGE_1_IN2_PIN, LOW);
    analogWrite(LM298N2_ENA_PIN, 0); // Matikan motor via ENA
    motorStorage1Active = false; // Update status
}

// Motor Storage 2 (LM298N #1, ENA: GPIO4)
void motor_storage_2_start(int speed) {
    speed = constrain(speed, 0, 255);
    Serial.println("[MOTOR_CONTROL] Motor Storage 2 START (Speed: " + String(speed) + ") [ENA: GPIO4]");
    pcf.digitalWrite(MOTOR_STORAGE_2_IN1_PIN, HIGH);
    pcf.digitalWrite(MOTOR_STORAGE_2_IN2_PIN, LOW);
    analogWrite(LM298N1_ENA_PIN, speed); // Kontrol kecepatan via ENA
    motorStorage2Active = true; // Update status
}

void motor_storage_2_stop() {
    Serial.println("[MOTOR_CONTROL] Motor Storage 2 STOP");
    pcf.digitalWrite(MOTOR_STORAGE_2_IN1_PIN, LOW);
    pcf.digitalWrite(MOTOR_STORAGE_2_IN2_PIN, LOW);
    analogWrite(LM298N1_ENA_PIN, 0); // Matikan motor via ENA
    motorStorage2Active = false; // Update status
}

// Motor Storage 3 (LM298N #1, ENB: GPIO16)
void motor_storage_3_start(int speed) {
    speed = constrain(speed, 0, 255);
    Serial.println("[MOTOR_CONTROL] Motor Storage 3 START (Speed: " + String(speed) + ") [ENB: GPIO16]");
    pcf.digitalWrite(MOTOR_STORAGE_3_IN1_PIN, HIGH);
    pcf.digitalWrite(MOTOR_STORAGE_3_IN2_PIN, LOW);
    analogWrite(LM298N1_ENB_PIN, speed); // Kontrol kecepatan via ENB
    motorStorage3Active = true; // Update status
}

void motor_storage_3_stop() {
    Serial.println("[MOTOR_CONTROL] Motor Storage 3 STOP");
    pcf.digitalWrite(MOTOR_STORAGE_3_IN1_PIN, LOW);
    pcf.digitalWrite(MOTOR_STORAGE_3_IN2_PIN, LOW);
    analogWrite(LM298N1_ENB_PIN, 0); // Matikan motor via ENB
    motorStorage3Active = false; // Update status
}

// Motor Mixer (LM298N #2, ENB: GPIO12)
void motor_mixer_start(int speed) {
    speed = constrain(speed, 0, 255);
    Serial.println("[MOTOR_CONTROL] Motor Mixer START (Speed: " + String(speed) + ") [ENB: GPIO12]");
    pcf.digitalWrite(MOTOR_MIXER_IN1_PIN, HIGH);
    pcf.digitalWrite(MOTOR_MIXER_IN2_PIN, LOW);
    analogWrite(LM298N2_ENB_PIN, speed); // Kontrol kecepatan via ENB
    motorMixerActive = true; // Update status
}

void motor_mixer_stop() {
    Serial.println("[MOTOR_CONTROL] Motor Mixer STOP");
    pcf.digitalWrite(MOTOR_MIXER_IN1_PIN, LOW);
    pcf.digitalWrite(MOTOR_MIXER_IN2_PIN, LOW);
    analogWrite(LM298N2_ENB_PIN, 0); // Matikan motor via ENB
    motorMixerActive = false; // Update status
}

// --- Implementasi Fungsi Kontrol Motor Pump (Relay langsung ke GPIO) ---
void motor_pump_galon_start() {
    Serial.println("[MOTOR_CONTROL] Pompa Galon ON (GPIO32)");
    digitalWrite(MOTOR_PUMP_GALON_RELAY_PIN, LOW); // Asumsi LOW = ON untuk relay
    motorPumpGalonActive = true; // Update status
}

void motor_pump_galon_stop() {
    Serial.println("[MOTOR_CONTROL] Pompa Galon OFF (GPIO32)");
    digitalWrite(MOTOR_PUMP_GALON_RELAY_PIN, HIGH); // Asumsi HIGH = OFF untuk relay
    motorPumpGalonActive = false; // Update status
}

void motor_pump_hot_water_start() {
    Serial.println("[MOTOR_CONTROL] Pompa Air Panas ON (GPIO25)");
    digitalWrite(MOTOR_PUMP_HOT_WATER_RELAY_PIN, LOW); // Asumsi LOW = ON untuk relay
    motorPumpHotWaterActive = true; // Update status
}

void motor_pump_hot_water_stop() {
    Serial.println("[MOTOR_CONTROL] Pompa Air Panas OFF (GPIO25)");
    digitalWrite(MOTOR_PUMP_HOT_WATER_RELAY_PIN, HIGH); // Asumsi HIGH = OFF untuk relay
    motorPumpHotWaterActive = false; // Update status
}

void motor_pump_seduh_kopi_start() {
    Serial.println("[MOTOR_CONTROL] Selenoid Seduh Kopi ON (GPIO33)");
    digitalWrite(MOTOR_PUMP_SEDUH_KOPI_RELAY_PIN, LOW); // Asumsi LOW = ON untuk relay
    motorPumpSeduhKopiActive = true; // Update status
}

void motor_pump_seduh_kopi_stop() {
    Serial.println("[MOTOR_CONTROL] Selenoid Seduh Kopi OFF (GPIO33)");
    digitalWrite(MOTOR_PUMP_SEDUH_KOPI_RELAY_PIN, HIGH); // Asumsi HIGH = OFF untuk relay
    motorPumpSeduhKopiActive = false; // Update status
}