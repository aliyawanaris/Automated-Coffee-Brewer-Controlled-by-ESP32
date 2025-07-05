#include "motor_control.h"
#include <Wire.h> // Diperlukan untuk komunikasi I2C
#include "components/lcd_display/lcd_display.h" // Diperlukan untuk akses ke objek lcd

// Definisi objek PCF8574 untuk motor
Adafruit_PCF8574 pcfMotor;

// Variabel internal untuk menyimpan alamat PCF8574 motor
static uint8_t currentMotorPcfAddress;

void setupMotorControl(uint8_t motorPcfAddress_param) {
    currentMotorPcfAddress = motorPcfAddress_param; // Simpan alamat yang diterima

    // --- Inisialisasi PCF8574 0x20 ---
    Serial.print(">> Inisialisasi PCF8574 (0x");
    if (currentMotorPcfAddress < 16) Serial.print("0");
    Serial.print(currentMotorPcfAddress, HEX);
    Serial.println(" - Motor Control)... ");

    if (!pcfMotor.begin(currentMotorPcfAddress, &Wire)) {
        Serial.println(">> PCF8574 (0x20) tidak ditemukan atau gagal inisialisasi.");
        lcd.setCursor(0, 3);
        lcd.print("PCF1 init FAILED!"); // Sesuai format Anda
        while(true); // Hentikan program jika fatal error
    } else {
        Serial.println(">> PCF8574 (0x20) OK!");
    }

    // Set semua pin sebagai OUTPUT untuk mengontrol L298N
    pcfMotor.pinMode(M1_IN1_PIN, OUTPUT);
    pcfMotor.pinMode(M1_IN2_PIN, OUTPUT);
    pcfMotor.pinMode(M2_IN3_PIN, OUTPUT);
    pcfMotor.pinMode(M2_IN4_PIN, OUTPUT);
    pcfMotor.pinMode(M3_IN1_PIN, OUTPUT);
    pcfMotor.pinMode(M3_IN2_PIN, OUTPUT);
    pcfMotor.pinMode(M4_IN3_PIN, OUTPUT);
    pcfMotor.pinMode(M4_IN4_PIN, OUTPUT);

    // Pastikan semua motor berhenti di awal
    stopAllMotors();
    Serial.println("Motor Control pins configured. All motors stopped.");
}

void stopAllMotors() {
    pcfMotor.digitalWrite(M1_IN1_PIN, LOW);
    pcfMotor.digitalWrite(M1_IN2_PIN, LOW);
    pcfMotor.digitalWrite(M2_IN3_PIN, LOW);
    pcfMotor.digitalWrite(M2_IN4_PIN, LOW);
    pcfMotor.digitalWrite(M3_IN1_PIN, LOW);
    pcfMotor.digitalWrite(M3_IN2_PIN, LOW);
    pcfMotor.digitalWrite(M4_IN3_PIN, LOW);
    pcfMotor.digitalWrite(M4_IN4_PIN, LOW);
}

// --- Implementasi Fungsi Kontrol untuk Motor Dinamo 1 ---
void motor1_start() {
    // Asumsi arah maju. Sesuaikan HIGH/LOW jika perlu membalik arah.
    pcfMotor.digitalWrite(M1_IN1_PIN, HIGH);
    pcfMotor.digitalWrite(M1_IN2_PIN, LOW);
    Serial.println("Motor Dinamo 1: ON");
}

void motor1_stop() {
    pcfMotor.digitalWrite(M1_IN1_PIN, LOW);
    pcfMotor.digitalWrite(M1_IN2_PIN, LOW);
    Serial.println("Motor Dinamo 1: OFF");
}

// --- Implementasi Fungsi Kontrol untuk Motor Dinamo 2 ---
void motor2_start() {
    pcfMotor.digitalWrite(M2_IN3_PIN, HIGH);
    pcfMotor.digitalWrite(M2_IN4_PIN, LOW);
    Serial.println("Motor Dinamo 2: ON");
}

void motor2_stop() {
    pcfMotor.digitalWrite(M2_IN3_PIN, LOW);
    pcfMotor.digitalWrite(M2_IN4_PIN, LOW);
    Serial.println("Motor Dinamo 2: OFF");
}

// --- Implementasi Fungsi Kontrol untuk Motor Dinamo 3 ---
void motor3_start() {
    pcfMotor.digitalWrite(M3_IN1_PIN, HIGH);
    pcfMotor.digitalWrite(M3_IN2_PIN, LOW);
    Serial.println("Motor Dinamo 3: ON");
}

void motor3_stop() {
    pcfMotor.digitalWrite(M3_IN1_PIN, LOW);
    pcfMotor.digitalWrite(M3_IN2_PIN, LOW);
    Serial.println("Motor Dinamo 3: OFF");
}

// --- Implementasi Fungsi Kontrol untuk Motor Dinamo 4 ---
void motor4_start() {
    pcfMotor.digitalWrite(M4_IN3_PIN, HIGH);
    pcfMotor.digitalWrite(M4_IN4_PIN, LOW);
    Serial.println("Motor Dinamo 4: ON");
}

void motor4_stop() {
    pcfMotor.digitalWrite(M4_IN3_PIN, LOW);
    pcfMotor.digitalWrite(M4_IN4_PIN, LOW);
    Serial.println("Motor Dinamo 4: OFF");
}