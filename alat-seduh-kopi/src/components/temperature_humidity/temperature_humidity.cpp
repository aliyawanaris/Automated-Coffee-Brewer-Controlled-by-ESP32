/*
  src/components/temperature_humidity/temperature_humidity.cpp - Implementasi Modul Sensor DHT22.
  Mengelola pembacaan data suhu dan kelembaban dari sensor DHT22.
  Log serial yang detail dan diformat per baris ditambahkan untuk debugging.
*/

#include "temperature_humidity.h" // Memasukkan file header modul ini
#include <DHT.h>                  // Library DHT sensor oleh Adafruit
#include <Arduino.h>              // Diperlukan untuk fungsi Arduino seperti Serial.println

// --- Definisi dan Inisialisasi Objek DHT ---
// Objek DHT dibuat dengan pin data dan tipe sensor yang telah ditentukan.
DHT dht(DHT_PIN, DHT_TYPE);

// --- Definisi Variabel Global Suhu dan Kelembaban ---
// Variabel-variabel ini akan menyimpan nilai suhu dan kelembaban yang terakhir terbaca.
// Diinisialisasi dengan nilai tidak valid (NaN) atau 0.0.
float currentTemperature = NAN; // Not A Number, menandakan belum ada data valid
float currentHumidity = NAN;    // Not A Number

/**
 * @brief Menginisialisasi sensor DHT22.
 * Fungsi ini memulai komunikasi dengan sensor DHT.
 * Output log akan ditampilkan di Serial Monitor untuk konfirmasi inisialisasi.
 */
void setupDHTSensor() {
  Serial.println("=========================================");
  Serial.println("  Inisialisasi Modul Sensor DHT22");
  Serial.println("=========================================");
  dht.begin(); // Memulai komunikasi dengan sensor DHT
  Serial.println("Sensor DHT22 siap digunakan.");
  Serial.println("-----------------------------------------");
}

/**
 * @brief Membaca data suhu dan kelembaban dari sensor DHT22.
 * Fungsi ini membaca data dari sensor dan memperbarui variabel global
 * currentTemperature dan currentHumidity. Ini juga mencetak log ke Serial Monitor.
 * Jika pembacaan gagal, variabel akan tetap pada nilai sebelumnya atau NaN.
 */
void readDHTSensor() {
  // Pembacaan sensor DHT bisa memakan waktu beberapa milidetik.
  // Delay atau blocking di dalam fungsi ini harus dihindari jika dipanggil sering.
  // DHT library sudah menangani timing internalnya.

  float h = dht.readHumidity();    // Membaca kelembaban
  float t = dht.readTemperature(); // Membaca suhu dalam Celsius (default)

  // Periksa apakah ada pembacaan yang gagal (misalnya, sensor tidak terhubung atau error).
  if (isnan(h) || isnan(t)) {
    Serial.println("DEBUG DHT: Gagal membaca dari sensor DHT. Pastikan koneksi benar.");
  } else {
    // Jika pembacaan berhasil, update variabel global
    currentHumidity = h;
    currentTemperature = t;

    // Cetak hasil ke Serial Monitor
    Serial.print("DEBUG DHT: Kelembaban: ");
    Serial.print(currentHumidity);
    Serial.print(" % | Suhu: ");
    Serial.print(currentTemperature);
    Serial.println(" *C");
  }
}
