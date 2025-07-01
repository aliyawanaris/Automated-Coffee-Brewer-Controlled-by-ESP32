#ifndef RFID_CARD_READER_H
#define RFID_CARD_READER_H

#include <Arduino.h> // Diperlukan untuk tipe data Arduino seperti String, byte
#include <SPI.h>     // Diperlukan untuk komunikasi SPI
#include <MFRC522.h> // Library untuk modul RFID/NFC Reader RC522

// --- Definisi Pin Modul RFID RC522 ---
// Sesuaikan pin-pin ini dengan koneksi fisik Anda ke ESP32.
// Anda menyebutkan RST pin 2, jadi kita gunakan itu.
#define SS_PIN    5   // Pin SDA (Chip Select) dari RC522 terhubung ke GPIO 5 ESP32
#define RST_PIN   2   // Pin RST (Reset) dari RC522 terhubung ke GPIO 2 ESP32

// --- Deklarasi Eksternal (Global) ---
// Deklarasikan objek MFRC522 sebagai 'extern' agar bisa diakses dari file lain (main.cpp).
// Objek ini akan didefinisikan di rfid_card_reader.cpp.
extern MFRC522 rfid;

// Deklarasikan variabel String untuk menyimpan UID kartu yang terbaca.
// Ini juga 'extern' agar bisa diakses dan ditampilkan di main.cpp (misalnya di LCD atau Web).
extern String currentRfidUid;

// --- Prototipe Fungsi ---
// Fungsi inisialisasi untuk modul RFID.
void setupRfidCardReader();

// Fungsi untuk menangani logika pembacaan kartu RFID di setiap loop.
void handleRfidCardReader(unsigned long currentMillis);

#endif // RFID_CARD_READER_H