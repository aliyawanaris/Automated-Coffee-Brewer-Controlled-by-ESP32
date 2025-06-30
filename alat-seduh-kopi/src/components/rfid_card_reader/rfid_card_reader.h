/*
  src/components/rfid_card_reader/rfid_card_reader.h - Header untuk Komponen RFID Card Reader
  Mendefinisikan antarmuka (deklarasi) untuk modul RFID RC522,
  termasuk fungsi-fungsi untuk pembacaan dasar, autentikasi, dan operasi blok.
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

// --- Fungsi-fungsi tambahan dari skrip lama, tersedia sebagai API publik komponen ---
// Mengecek apakah kartu baru ada dan sudah dibaca UID-nya
bool isNewCardPresentAndRead();

// Mendapatkan UID kartu yang sedang terbaca sebagai string
String getCardUid();

// Menghentikan komunikasi dengan PICC dan menghentikan enkripsi PCD
void haltPICC();

// Fungsi otentikasi blok MIFARE
// Mengembalikan true jika otentikasi berhasil
bool authenticateBlock(byte blockAddr, MFRC522::MIFARE_Key* key, MFRC522::Uid* uid);

// Fungsi membaca blok data MIFARE
// Mengembalikan status kode MFRC522 (STATUS_OK jika berhasil)
MFRC522::StatusCode readBlock(byte blockAddr, byte* buffer, byte* bufferSize);

// Fungsi menulis blok data MIFARE
// Mengembalikan status kode MFRC522 (STATUS_OK jika berhasil)
MFRC522::StatusCode writeBlock(byte blockAddr, byte* buffer, byte bufferSize);


#endif // RFID_CARD_READER_H
