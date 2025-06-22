#include "storage_detector.h" // Masukkan file header modul ini

// Definisi Aktual Pin Ultrasonic Sensor (untuk Detektor Penyimpanan)
// Sesuai dengan konfigurasi terakhir yang telah disepakati:
// HC-SR04 #1
const int SD_TRIG_PIN_1 = 27; // GPIO 27 - Checked
const int SD_ECHO_PIN_1 = 34; // GPIO 34 - Checked

// HC-SR04 #2
const int SD_TRIG_PIN_2 = 14; // GPIO 14 - Checked
const int SD_ECHO_PIN_2 = 35; // GPIO 35 - Checked

// HC-SR04 #3
const int SD_TRIG_PIN_3 = 13; // GPIO 13 - Checked
const int SD_ECHO_PIN_3 = 36; // GPIO 36 - Checked
// Catatan: Pin ini harus sesuai dengan koneksi fisik pada board ESP32 Anda.


// Implementasi Fungsi untuk Inisialisasi Semua Sensor Detektor Penyimpanan
void storage_detector_init_all_sensors() { // Ganti nama fungsi
    pinMode(SD_TRIG_PIN_1, OUTPUT);
    pinMode(SD_ECHO_PIN_1, INPUT);
    pinMode(SD_TRIG_PIN_2, OUTPUT);
    pinMode(SD_ECHO_PIN_2, INPUT);
    pinMode(SD_TRIG_PIN_3, OUTPUT);
    pinMode(SD_ECHO_PIN_3, INPUT);

    Serial.println("Semua Sensor Detektor Penyimpanan berhasil diinisialisasi.");
}

// Implementasi Fungsi untuk Mendapatkan Jarak dari HC-SR04
long storage_detector_get_distance(int trigPin, int echoPin) { // Ganti nama fungsi
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);
    long duration = pulseIn(echoPin, HIGH, 30000); // 30ms timeout (maksimal sekitar 5 meter)

    if (duration == 0) {
        return -1; // Mengembalikan -1 jika timeout
    }
    long distance = duration * 0.034 / 2; // Kecepatan suara 0.034 cm/µs
    return distance;
}