#include "temperature_humidity.h" // Include header komponen ini

// Inisialisasi objek DHT
DHT dht(DHT_PIN, DHT_TYPE);

// Definisi global variables (deklarasi extern di .h)
float currentHumidity = 0.0;
float currentTemperature = 0.0;

// Konstanta untuk interval pembacaan DHT
const long DHT_READ_INTERVAL_MS = 5000; // Baca setiap 5 detik
unsigned long lastDhtReadMillis = 0;

/**
 * @brief Menginisialisasi sensor DHT.
 * Fungsi ini harus dipanggil sekali di setup() utama.
 */
void setupTemperatureHumidity() {
  dht.begin();
  Serial.println("Sensor DHT diinisialisasi.");
}

/**
 * @brief Menangani pembacaan data dari sensor DHT.
 * Harus dipanggil berulang kali di loop().
 * @param currentMillis Waktu millis() saat ini.
 */
void handleTemperatureHumidity(unsigned long currentMillis) {
  if (currentMillis - lastDhtReadMillis >= DHT_READ_INTERVAL_MS) {
    lastDhtReadMillis = currentMillis;

    // Baca kelembaban dan suhu
    float h = dht.readHumidity();
    float t = dht.readTemperature();

    // Periksa apakah pembacaan berhasil
    if (isnan(h) || isnan(t)) {
      Serial.println("Gagal membaca dari sensor DHT!");
      // Biarkan nilai terakhir jika gagal, atau set ke 0 jika ingin indikasi error
      // currentHumidity = 0.0;
      // currentTemperature = 0.0;
    } else {
      currentHumidity = h;
      currentTemperature = t;
      Serial.print("Kelembaban: ");
      Serial.print(currentHumidity);
      Serial.print(" %  Suhu: ");
      Serial.print(currentTemperature);
      Serial.println(" *C");
    }
  }
}