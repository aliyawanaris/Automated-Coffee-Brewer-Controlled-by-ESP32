/*
  src/components/rfid_card_reader/rfid_card_reader.h - Header untuk Komponen RFID Card Reader
  Mendefinisikan antarmuka (deklarasi) untuk modul RFID RC522.
*/

#ifndef RFID_CARD_READER_H
#define RFID_CARD_READER_H

#include <Arduino.h>
#include <SPI.h>     // Diperlukan untuk komunikasi SPI
#include <MFRC522.h> // Diperlukan untuk objek MFRC522

// --- Definisi Pin RFID RC522 ---
// Sesuai image_27db9a.png
#define SS_PIN      5           // SDA (CS) Pin for RC522
#define RST_PIN     0xFF        // RST Pin - 'x' di diagram, 0xFF berarti tidak terhubung/software reset

// --- Deklarasi Objek MFRC522 ---
// Gunakan 'extern' karena objek akan didefinisikan di rfid_card_reader.cpp
extern MFRC522 mfrc522;

// --- Deklarasi Variabel Global untuk RFID ---
extern String currentRfidUid; // Menyimpan UID yang terbaca
extern unsigned long lastCardReadTime;
extern const long RFID_UID_CLEAR_DELAY_MS; // UID akan dihapus setelah delay jika kartu dilepas

// --- Prototipe Fungsi ---
// Fungsi untuk menginisialisasi modul RFID.
void setupRfidCardReader();

// Fungsi untuk menangani logika pembacaan kartu RFID di loop utama.
// Membutuhkan waktu milidetik saat ini untuk manajemen delay.
void handleRfidCardReader(unsigned long currentMillis);

#endif // RFID_CARD_READER_H
