#ifndef CARD_READER_H
#define CARD_READER_H

#include <MFRC522.h> // Library RFID RC522

// --- Definisi Pin RFID Reader (MFRC522) ---
// Pin-pin ini harus sesuai dengan tabel pinout proyek Anda
#define SS_PIN    5   // ESP32 GPIO 5 (Slave Select/Chip Select)
#define RST_PIN   2   // ESP32 GPIO 2 (Reset Pin - Opsional, tapi disarankan)

// Deklarasi objek MFRC522 global
extern MFRC522 mfrc522;

// Fungsi untuk menginisialisasi modul RFID
void card_reader_init();

// Fungsi untuk mengecek apakah ada kartu baru terdeteksi
bool card_reader_is_new_card_present();

// Fungsi untuk membaca UID (Unique ID) kartu
String card_reader_read_card_uid();

// **Perubahan di sini:** Parameter UID dikembalikan ke MFRC522::Uid*
bool card_reader_authenticate_block(byte blockAddr, MFRC522::MIFARE_Key* key, MFRC522::Uid* uid, MFRC522::StatusCode* status);

// Fungsi untuk membaca data dari blok (jika diperlukan untuk mode R/W)
MFRC522::StatusCode card_reader_read_block(byte blockAddr, byte* buffer, byte* bufferSize);

// Fungsi untuk menulis data ke blok (jika diperlukan untuk mode R/W)
MFRC522::StatusCode card_reader_write_block(byte blockAddr, byte* buffer, byte bufferSize);

// Fungsi untuk menghentikan enkripsi (membuat kartu siap dibaca lagi)
void card_reader_halt();

#endif // CARD_READER_H