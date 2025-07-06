#ifndef MOTOR_CONTROL_H
#define MOTOR_CONTROL_H

#include <Arduino.h>
#include <Adafruit_PCF8574.h> // Asumsi menggunakan Adafruit_PCF8574 untuk motor control

// --- Definisi Pin Motor pada PCF8574 (0x20) ---
// LM298N #1
#define MOTOR_STORAGE_2_IN1_PIN 0 // P0 - Storage 2
#define MOTOR_STORAGE_2_IN2_PIN 1 // P1 - Storage 2
#define MOTOR_STORAGE_3_IN1_PIN 2 // P2 - Storage 3
#define MOTOR_STORAGE_3_IN2_PIN 3 // P3 - Storage 3
// LM298N #2
#define MOTOR_STORAGE_1_IN1_PIN 4 // P4 - Motor Dinamo 1
#define MOTOR_STORAGE_1_IN2_PIN 5 // P5 - Motor Dinamo 1
#define MOTOR_MIXER_IN1_PIN 6 // P6 - Motor Dinamo 4
#define MOTOR_MIXER_IN2_PIN 7 // P7 - Motor Dinamo 4

// --- Definisi Pin Motor Pump (Relay) pada ESP32 ---
#define MOTOR_PUMP_GALON_RELAY_PIN 32 // Pompa Air Galon
#define MOTOR_PUMP_HOT_WATER_RELAY_PIN 25 // Pompa Air Panan - Water Heater
#define MOTOR_PUMP_SEDUH_KOPI_RELAY_PIN 33 // Pompa Air Seduh Kopi

// --- Deklarasi Objek PCF8574 sebagai extern ---
extern Adafruit_PCF8574 pcf; // Objek PCF8574 global untuk motor control

// --- Prototipe Fungsi Motor Control ---

// Fungsi inisialisasi motor control
void setupMotorControl(uint8_t pcf_address);

// Fungsi untuk mengontrol motor storage 2
void motor_storage_1_start();
void motor_storage_1_stop();

// Fungsi untuk mengontrol motor storage 2
void motor_mixer_start();
void motor_mixer_stop();

// Fungsi untuk mengontrol motor storage 2
void motor_pump_galon_start();
void motor_pump_galon_stop();

// Fungsi untuk mengontrol motor storage 2
void motor_pump_hot_water_start();
void motor_pump_hot_water_stop();

// Fungsi untuk mengontrol motor storage 2
void motor_pump_seduh_kopi_start();
void motor_pump_seduh_kopi_stop();

#endif // MOTOR_CONTROL_H