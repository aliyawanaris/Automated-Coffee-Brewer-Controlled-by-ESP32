/*
  src/components/storage_detector/storage_detector.cpp - Implementasi Modul Detektor Penyimpanan
  Modul ini bertanggung jawab untuk mengelola sensor jarak ultrasonik (HC-SR04)
  guna mendeteksi level atau kapasitas pada wadah penyimpanan (biji kopi, air, ampas).
  Setiap sensor akan diinisialisasi dan hasil pengukurannya dapat diambil melalui fungsi.
*/

#include "storage_detector.h" // Memasukkan file header modul ini
#include <Arduino.h>          // Diperlukan untuk fungsi Arduino seperti pinMode, digitalWrite, pulseIn, Serial.println

// --- Definisi Pin Aktual untuk Sensor Ultrasonik HC-SR04 ---
// Pin-pin ini harus sesuai dengan koneksi fisik HC-SR04 ke ESP32.
// Penting: Pastikan pin yang dipilih tidak bentrok dengan modul lain (misalnya I2C, SPI).

// HC-SR04 #1: Digunakan untuk mengukur Kapasitas Kopi 1
const int SD_TRIG_PIN_1 = 27; // Pin Trigger HC-SR04 #1 (OUTPUT: mengirim pulsa suara)
const int SD_ECHO_PIN_1 = 34; // Pin Echo HC-SR04 #1 (INPUT: menerima pulsa pantulan suara)

// HC-SR04 #2: Digunakan untuk mengukur Kapasitas Kopi 2
const int SD_TRIG_PIN_2 = 14; // Pin Trigger HC-SR04 #2
const int SD_ECHO_PIN_2 = 35; // Pin Echo HC-SR04 #2

// HC-SR04 #3: Digunakan untuk mengukur Kapasitas Kopi 3
const int SD_TRIG_PIN_3 = 13; // Pin Trigger HC-SR04 #3
const int SD_ECHO_PIN_3 = 36; // Pin Echo HC-SR04 #3


/**
 * @brief Menginisialisasi semua pin untuk sensor detektor penyimpanan.
 * Fungsi ini mengatur pin TRIG sebagai OUTPUT dan pin ECHO sebagai INPUT
 * untuk setiap sensor ultrasonik yang didefinisikan.
 */
void storage_detector_init_all_sensors() {
    // Inisialisasi pin untuk HC-SR04 #1
    pinMode(SD_TRIG_PIN_1, OUTPUT);
    pinMode(SD_ECHO_PIN_1, INPUT);
    Serial.println("[STORAGE_DETECTOR] Sensor 1 (Trig:" + String(SD_TRIG_PIN_1) + ", Echo:" + String(SD_ECHO_PIN_1) + ") diinisialisasi.");

    // Inisialisasi pin untuk HC-SR04 #2
    digitalWrite(SD_TRIG_PIN_2, LOW); // Pastikan LOW sebelum pinMode agar tidak ada pulsa awal
    pinMode(SD_TRIG_PIN_2, OUTPUT);
    pinMode(SD_ECHO_PIN_2, INPUT);
    Serial.println("[STORAGE_DETECTOR] Sensor 2 (Trig:" + String(SD_TRIG_PIN_2) + ", Echo:" + String(SD_ECHO_PIN_2) + ") diinisialisasi.");

    // Inisialisasi pin untuk HC-SR04 #3
    digitalWrite(SD_TRIG_PIN_3, LOW); // Pastikan LOW sebelum pinMode agar tidak ada pulsa awal
    pinMode(SD_TRIG_PIN_3, OUTPUT);
    pinMode(SD_ECHO_PIN_3, INPUT);
    Serial.println("[STORAGE_DETECTOR] Sensor 3 (Trig:" + String(SD_TRIG_PIN_3) + ", Echo:" + String(SD_ECHO_PIN_3) + ") diinisialisasi.");

    Serial.println("[STORAGE_DETECTOR] Semua Sensor Detektor Penyimpanan berhasil diinisialisasi.");
}

/**
 * @brief Mengukur jarak menggunakan sensor ultrasonik HC-SR04.
 * Fungsi ini mengirimkan pulsa suara dan menghitung waktu pantulan
 * untuk menentukan jarak ke objek terdekat.
 * @param trigPin Pin Trigger HC-SR04.
 * @param echoPin Pin Echo HC-SR04.
 * @return Jarak dalam centimeter (cm), atau -1 jika terjadi timeout (tidak ada pantulan).
 */
long storage_detector_get_distance(int trigPin, int echoPin) {
    // Memastikan pin TRIG dalam keadaan LOW sebelum memulai pengukuran
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2); // Menunggu 2 mikrosekon untuk memastikan pin LOW

    // Mengirim pulsa HIGH selama 10 mikrosekon pada pin TRIG
    // Ini akan memicu sensor untuk mengirim 8 siklus burst ultrasonik
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW); // Menghentikan pulsa TRIG

    // Mengukur durasi pulsa HIGH pada pin ECHO
    // Durasi ini adalah waktu yang dibutuhkan suara untuk pergi dan kembali
    // Timeout 30000 mikrosekon (30 ms) digunakan untuk menghindari hang jika tidak ada pantulan
    // 30 ms kira-kira setara dengan jarak 5 meter pulang-pergi.
    long duration = pulseIn(echoPin, HIGH, 30000);

    // Jika pulseIn mengembalikan 0, berarti terjadi timeout (tidak ada pantulan)
    if (duration == 0) {
        Serial.println("[STORAGE_DETECTOR] Sensor (Trig:" + String(trigPin) + ", Echo:" + String(echoPin) + ") Timeout: Tidak ada objek terdeteksi dalam jangkauan.");
        return -1; // Mengembalikan -1 untuk menunjukkan error atau di luar jangkauan
    }

    // Menghitung jarak berdasarkan durasi pulsa
    // Kecepatan suara di udara adalah sekitar 343 meter/detik atau 0.0343 cm/mikrosekon
    // Jarak = (Durasi * Kecepatan Suara) / 2 (karena suara pergi dan kembali)
    long distance = duration * 0.034 / 2; // Menggunakan 0.034 cm/µs untuk penyederhanaan

    // --- Output Debugging ke Serial Monitor ---
    Serial.println("[STORAGE_DETECTOR] Sensor (Trig:" + String(trigPin) + ", Echo:" + String(echoPin) + ") Pengukuran: " + String(distance) + " cm");
    // ------------------------------------------

    return distance; // Mengembalikan jarak yang terukur
}

/**
 * @brief Mendapatkan status stok kopi dalam bentuk teks.
 * Mengukur jarak menggunakan pin sensor yang diberikan dan menginterpretasikannya
 * menjadi pesan status "Stok Kopi Menipis/Habis" atau "Stok Tersedia".
 * @param trigPin Pin Trigger HC-SR04.
 * @param echoPin Pin Echo HC-SR04.
 * @return String yang berisi status stok kopi.
 */
String getCoffeeStockStatus(int trigPin, int echoPin) {
    long distance = storage_detector_get_distance(trigPin, echoPin); // Ambil jarak sensor kopi

    // Threshold 12 cm untuk stok kopi: >=12cm menipis/habis, <12cm tersedia
    const int COFFEE_STOCK_THRESHOLD = 8;

    if (distance == -1) {
        return "Sensor Error/N/A";
    } else if (distance >= COFFEE_STOCK_THRESHOLD) {
        return "Stok Menipis/Habis";
    } else { // distance < COFFEE_STOCK_THRESHOLD
        return "Stok Tersedia";
    }
}

// Fungsi getCoffeeStockStatus() yang spesifik untuk setiap sensor
String getCoffee1StockStatus() {
    return getCoffeeStockStatus(SD_TRIG_PIN_1, SD_ECHO_PIN_1);
}

String getCoffee2StockStatus() {
    return getCoffeeStockStatus(SD_TRIG_PIN_2, SD_ECHO_PIN_2);
}

String getCoffee3StockStatus() {
    return getCoffeeStockStatus(SD_TRIG_PIN_3, SD_ECHO_PIN_3);
}