#include "temperature_humidity.h" // Masukkan file header modul ini

// Definisi Aktual Pin Data DHT
const int TEMPERATURE_HUMIDITY_DATA_PIN = 15;

// Inisialisasi objek DHT
DHT dht(TEMPERATURE_HUMIDITY_DATA_PIN, DHTTYPE);

// Implementasi Fungsi untuk Inisialisasi Sensor DHT
void temperature_humidity_init() {
    dht.begin();
    Serial.println("Sensor DHT22 (Temperature & Humidity) berhasil diinisialisasi.");
}

// Implementasi Fungsi untuk Membaca Suhu
float temperature_humidity_read_temperature() {
    float temperature = dht.readTemperature();
    if (isnan(temperature)) {
        Serial.println("Gagal membaca suhu dari sensor DHT!");
        return -999.0; // Mengembalikan nilai error atau 0, sesuaikan kebutuhan Anda
    }
    return temperature;
}

// Implementasi Fungsi untuk Membaca Kelembaban
float temperature_humidity_read_humidity() {
    float humidity = dht.readHumidity();
    if (isnan(humidity)) {
        Serial.println("Gagal membaca kelembaban dari sensor DHT!");
        return -999.0; // Mengembalikan nilai error atau 0, sesuaikan kebutuhan Anda
    }
    return humidity;
}