#ifndef TEMPERATURE_HUMIDITY_H
#define TEMPERATURE_HUMIDITY_H

#include <Arduino.h>
#include <DHT.h> // Library untuk sensor DHT

// Definisi Pin dan Tipe DHT Sensor
// SESUAIKAN DENGAN PIN GPIO ESP32 ANDA YANG TERHUBUNG KE DATA PIN DHT22
#define DHT_PIN 17  // Contoh: GPIO 17 (pin data DHT22)
#define DHT_TYPE DHT22 // Tipe sensor yang digunakan (DHT11, DHT22, DHT21)

// Deklarasi global variable untuk menyimpan hasil pembacaan
extern float currentHumidity;
extern float currentTemperature;

// Prototipe Fungsi
void setupTemperatureHumidity();
void handleTemperatureHumidity(unsigned long currentMillis);

#endif // TEMPERATURE_HUMIDITY_H