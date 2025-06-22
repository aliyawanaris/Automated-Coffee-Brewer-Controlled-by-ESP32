#ifndef STORAGE_DETECTOR_H // Perbarui include guard
#define STORAGE_DETECTOR_H

#include <Arduino.h> // Diperlukan untuk tipe data seperti long, int, dan fungsi Arduino (pinMode, digitalWrite, dll.)

// Deklarasi Pin Sensor Ultrasonik (untuk Detektor Penyimpanan)
extern const int SD_TRIG_PIN_1;
extern const int SD_ECHO_PIN_1;

extern const int SD_TRIG_PIN_2;
extern const int SD_ECHO_PIN_2;

extern const int SD_TRIG_PIN_3;
extern const int SD_ECHO_PIN_3;

// Deklarasi Fungsi-fungsi untuk Modul Detektor Penyimpanan
void storage_detector_init_all_sensors(); // Ganti nama fungsi
long storage_detector_get_distance(int trigPin, int echoPin); // Ganti nama fungsi

#endif // STORAGE_DETECTOR_H