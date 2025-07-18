#ifndef MOTOR_CONTROL_H
#define MOTOR_CONTROL_H

#include <Arduino.h>
#include <Adafruit_PCF8574.h> // Asumsi menggunakan Adafruit_PCF8574 untuk motor control

// --- Definisi Pin Motor pada PCF8574 (0x20) ---
// PCF8574 (0x20) & LM298N #1
#define MOTOR_STORAGE_2_IN1_PIN 0 // P0 - Storage 2
#define MOTOR_STORAGE_2_IN2_PIN 1 // P1 - Storage 2
#define MOTOR_STORAGE_3_IN1_PIN 2 // P2 - Storage 3
#define MOTOR_STORAGE_3_IN2_PIN 3 // P3 - Storage 3
// PCF8574 (0x20) & LM298N #2
#define MOTOR_STORAGE_1_IN1_PIN 4 // P4 - Motor Dinamo 1
#define MOTOR_STORAGE_1_IN2_PIN 5 // P5 - Motor Dinamo 1
#define MOTOR_MIXER_IN1_PIN 6 // P6 - Motor Dinamo 4
#define MOTOR_MIXER_IN2_PIN 7 // P7 - Motor Dinamo 4

// --- Definisi Pin ENA/ENB LM298N pada ESP32 (untuk kontrol kecepatan/enable) ---
#define LM298N1_ENA_PIN 4  // GPIO4 untuk ENA LM298N #1 (mengontrol Storage 2)
#define LM298N1_ENB_PIN 16 // GPIO16 untuk ENB LM298N #1 (mengontrol Storage 3)
#define LM298N2_ENA_PIN 17 // GPIO17 untuk ENA LM298N #2 (mengontrol Storage 1)
#define LM298N2_ENB_PIN 12 // GPIO12 untuk ENB LM298N #2 (mengontrol Mixer)

// --- Definisi Pin Motor Pump (Relay) pada ESP32 ---
#define MOTOR_PUMP_GALON_RELAY_PIN 25      // IN1 - Pompa Air Galon
#define MOTOR_PUMP_HOT_WATER_RELAY_PIN 33  // IN2 - Pompa Air Panas - Water Heater
#define MOTOR_PUMP_SEDUH_KOPI_RELAY_PIN 32 // IN4 - Pompa Air Seduh Kopi

// --- Deklarasi Objek PCF8574 sebagai extern ---
extern Adafruit_PCF8574 pcf; // Objek PCF8574 global untuk motor control

// --- Deklarasi Variabel Status Motor (Extern) ---
// Variabel ini didefinisikan di motor_control.cpp dan diakses di tempat lain melalui 'extern'
extern bool dynamoIsActive;    // Motor dinamo (pomp galon)
extern bool storage1IsActive;  // Motor storage 1 (Torabika)
extern bool storage2IsActive;  // Motor storage 2 (Good Day)
extern bool storage3IsActive;  // Motor storage 3 (ABC Susu)
extern bool mixerIsActive;     // Motor mixer
extern bool pumpGalonIsActive; // Tambahan: status pompa galon
extern bool pumpHotWaterIsActive; // Tambahan: status pompa air panas
extern bool pumpSeduhKopiIsActive; // Tambahan: status selenoid seduh kopi

// Fungsi inisialisasi motor control
void setupMotorControl(uint8_t pcf_address);

// Fungsi untuk mengontrol motor storage 1 (terhubung ke LM298N #2)
void motor_storage_1_start(int speed = 255); // Ditambah parameter speed, default full speed
void motor_storage_1_stop();

// Fungsi untuk mengontrol motor storage 2 (terhubung ke LM298N #1)
void motor_storage_2_start(int speed = 255); // Ditambah parameter speed, default full speed
void motor_storage_2_stop();

// Fungsi untuk mengontrol motor storage 3 (terhubung ke LM298N #1)
void motor_storage_3_start(int speed = 255); // Ditambah parameter speed, default full speed
void motor_storage_3_stop();

// Fungsi untuk mengontrol motor mixer (terhubung ke LM298N #2)
void motor_mixer_start(int speed = 200); // Ditambah parameter speed, default full speed
void motor_mixer_stop();

// Fungsi untuk mengontrol pompa air galon
void motor_pump_galon_start();
void motor_pump_galon_stop();

// Fungsi untuk mengontrol pompa air panas
void motor_pump_hot_water_start();
void motor_pump_hot_water_stop();

// Fungsi untuk mengontrol pompa air seduh kopi
void motor_pump_seduh_kopi_start();
void motor_pump_seduh_kopi_stop();

#endif // MOTOR_CONTROL_H