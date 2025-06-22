#ifndef TEMPERATURE_HUMIDITY_H
#define TEMPERATURE_HUMIDITY_H

#include <Arduino.h>
#include <DHT.h> // Diperlukan untuk tipe DHT22 dan objek DHT

// Definisi Tipe DHT Sensor
#define DHTTYPE DHT22 // Sensor DHT22

// Deklarasi Pin Data DHT
extern const int TEMPERATURE_HUMIDITY_DATA_PIN;

// Deklarasi Fungsi untuk Modul Suhu & Kelembaban
void temperature_humidity_init();
float temperature_humidity_read_temperature();
float temperature_humidity_read_humidity();

#endif // TEMPERATURE_HUMIDITY_H