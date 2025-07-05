#ifndef TEMPERATURE_HUMIDITY_H
#define TEMPERATURE_HUMIDITY_H

#include <DHT.h>
#include <DHT_U.h>
#include <Adafruit_Sensor.h>

// Definisikan pin data untuk DHT22
// SESUAIKAN DENGAN KONEKSI FISIK ANDA KE ESP32.
#define DHT_PIN 15   // Pin GPIO 15 untuk data DHT <--- PASTIKAN INI 15
#define DHT_TYPE DHT22

extern DHT_Unified dht;
extern float currentTemperature;
extern float currentHumidity;

void setupTemperatureHumidity();
void handleTemperatureHumidity(unsigned long currentMillis);

#endif // TEMPERATURE_HUMIDITY_H