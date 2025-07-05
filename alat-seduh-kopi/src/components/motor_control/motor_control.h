#ifndef MOTOR_CONTROL_H
#define MOTOR_CONTROL_H

#include <Arduino.h>
#include <Adafruit_PCF8574.h> // Diperlukan untuk PCF8574

// Pin-pin pada PCF8574 (0x20) yang terhubung ke L298N
// L298N Ke-1
#define M1_IN1_PIN 0 // P0 = Motor Dinamo 1 (IN1)
#define M1_IN2_PIN 1 // P1 = Motor Dinamo 1 (IN2)
#define M2_IN3_PIN 2 // P2 = Motor Dinamo 2 (IN3)
#define M2_IN4_PIN 3 // P3 = Motor Dinamo 2 (IN4)

// L298N Ke-2
#define M3_IN1_PIN 4 // P4 = Motor Dinamo 3 (IN1)
#define M3_IN2_PIN 5 // P5 = Motor Dinamo 3 (IN2)
#define M4_IN3_PIN 6 // P6 = Motor Dinamo 4 (IN3)
#define M4_IN4_PIN 7 // P7 = Motor Dinamo 4 (IN4)

// Deklarasi objek PCF8574 untuk motor sebagai extern
extern Adafruit_PCF8574 pcfMotor;

// --- Prototipe Fungsi Kontrol Motor ---

/**
 * @brief Menginisialisasi PCF8574 yang terhubung ke motor.
 */
void setupMotorControl();

/**
 * @brief Menghentikan semua motor dinamo.
 * Ini akan menyetel semua pin IN ke LOW untuk memastikan motor berhenti.
 */
void stopAllMotors();

// Fungsi kontrol untuk Motor Dinamo 1 (Contoh: Penggiling Kopi)
void motor1_start();
void motor1_stop();

// Fungsi kontrol untuk Motor Dinamo 2 (Contoh: Pompa Air)
void motor2_start();
void motor2_stop();

// Fungsi kontrol untuk Motor Dinamo 3 (Contoh: Pengaduk)
void motor3_start();
void motor3_stop();

// Fungsi kontrol untuk Motor Dinamo 4 (Contoh: Pembuang Ampas)
void motor4_start();
void motor4_stop();

#endif // MOTOR_CONTROL_H