#include "temperature_humidity.h"
#include "components/lcd_display/lcd_display.h" // Include LCD display header untuk update LCD

// Definisi objek DHT
// Pastikan pin dan tipe sesuai dengan definisi di .h
DHT_Unified dht(DHT_PIN, DHT_TYPE);

// Definisi variabel global yang dideklarasikan di .h
float currentTemperature = 0.0;
float currentHumidity = 0.0;

// Variabel untuk interval pembacaan (hanya di .cpp, tidak perlu extern)
static unsigned long lastDHTReadMillis = 0;
const long DHT_READ_INTERVAL = 5000; // Baca setiap 5 detik (5000 ms)

void setupTemperatureHumidity() {
  dht.begin();
  Serial.println("DHT22 Sensor diinisialisasi.");
  // Opsional: delay sebentar untuk memastikan sensor siap
  delay(100);
}

void handleTemperatureHumidity(unsigned long currentMillis) {
  // Hanya baca sensor jika sudah melewati interval yang ditentukan
  if (currentMillis - lastDHTReadMillis >= DHT_READ_INTERVAL) {
    lastDHTReadMillis = currentMillis;

    // Baca kelembaban
    sensors_event_t event;
    dht.humidity().getEvent(&event);
    if (isnan(event.relative_humidity)) {
      Serial.println(F("Gagal membaca kelembaban dari sensor DHT!"));
      currentHumidity = NAN; // Set ke NAN jika gagal
    } else {
      currentHumidity = event.relative_humidity;
      Serial.print(F("Kelembaban: "));
      Serial.print(currentHumidity, 0); // Tampilkan tanpa desimal di Serial
      Serial.println(F(" %"));
    }

    // Baca suhu
    dht.temperature().getEvent(&event);
    if (isnan(event.temperature)) {
      Serial.println(F("Gagal membaca suhu dari sensor DHT!"));
      currentTemperature = NAN; // Set ke NAN jika gagal
    } else {
      currentTemperature = event.temperature;
      Serial.print(F("Suhu: "));
      Serial.print(currentTemperature, 1); // Tampilkan dengan 1 desimal di Serial
      Serial.println(F(" °C"));
    }

    // (Opsional) Tampilkan di LCD jika perlu, namun Anda sudah memiliki rotasi display di main.cpp
    // Anda bisa membuat fungsi helper di lcd_display.cpp untuk ini jika ingin mengisolasi logika display
    // Contoh: updateLCDDHT(currentTemperature, currentHumidity);
  }
}