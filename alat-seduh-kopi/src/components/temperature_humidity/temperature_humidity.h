/*
  src/components/temperature_humidity/temperature_humidity.h - Header untuk Modul Sensor DHT22.
  Mendefinisikan antarmuka (deklarasi fungsi dan variabel global)
  untuk membaca data suhu dan kelembaban dari sensor DHT22.
*/

#ifndef TEMPERATURE_HUMIDITY_H
#define TEMPERATURE_HUMIDITY_H

#include <Arduino.h> // Diperlukan untuk tipe data seperti float

// --- Definisi Pin Hardware DHT22 ---
// Sesuaikan pin ini dengan koneksi fisik sensor DHT22 ke ESP32 Anda.
#define DHT_PIN   15    // GPIO 15 untuk data DHT22
#define DHT_TYPE  DHT22 // Tipe sensor DHT yang digunakan (DHT11, DHT21, DHT22)

// --- Deklarasi Variabel Global Suhu dan Kelembaban ---
// Variabel-variabel ini akan menyimpan nilai suhu dan kelembaban yang terakhir terbaca.
// Menggunakan 'extern' karena definisi aktualnya ada di temperature_humidity.cpp.
extern float currentTemperature; // Menyimpan suhu dalam derajat Celsius
extern float currentHumidity;    // Menyimpan kelembaban dalam persentase (%)

// --- Prototipe Fungsi Modul DHT22 ---
/**
 * @brief Menginisialisasi sensor DHT22.
 * Fungsi ini harus dipanggil di dalam setup() utama program.
 * Ini akan memulai komunikasi dengan sensor DHT.
 */
void setupDHTSensor();

/**
 * @brief Membaca data suhu dan kelembaban dari sensor DHT22.
 * Fungsi ini harus dipanggil secara berulang di dalam loop() utama program
 * pada interval tertentu. Ini akan memperbarui variabel global currentTemperature
 * dan currentHumidity.
 */
void readDHTSensor();

#endif // TEMPERATURE_HUMIDITY_H
