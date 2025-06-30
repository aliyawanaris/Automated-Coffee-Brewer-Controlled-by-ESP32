/*
  src/components/rfid_card_reader/rfid_card_reader.cpp - Implementasi Komponen RFID Card Reader
  Mengelola modul RFID RC522, membaca UID kartu, dan menyimpan statusnya.
*/

#include "rfid_card_reader.h" // Include header komponen ini
#include <Arduino.h>          // Untuk Serial.print, dll.

// --- Definisi Objek MFRC522 ---
MFRC522 mfrc522(SS_PIN, RST_PIN);

// --- Definisi Variabel Global untuk RFID ---
String currentRfidUid = "Belum Terbaca";
unsigned long lastCardReadTime = 0;
const long RFID_UID_CLEAR_DELAY_MS = 5000; // UID akan dihapus setelah 5 detik jika kartu tidak ada


// --- Implementasi Fungsi ---

void setupRfidCardReader() {
  Serial.println("Menginisialisasi RFID Reader...");
  SPI.begin();           // Inisialisasi bus SPI
  mfrc522.PCD_Init();    // Inisialisasi modul MFRC522
  mfrc522.PCD_DumpVersionToSerial(); // Opsional: Tampilkan informasi versi RC522 ke Serial Monitor
  Serial.println("RFID Reader siap.");
}

void handleRfidCardReader(unsigned long currentMillis) {
  // Cek apakah ada kartu baru dan baca serialnya
  if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
    String uid = "";
    for (byte i = 0; i < mfrc522.uid.size; i++) {
      uid += (mfrc522.uid.uidByte[i] < 0x10 ? "0" : ""); // Tambah "0" jika kurang dari 16 (hex 1 digit)
      uid += String(mfrc522.uid.uidByte[i], HEX);      // Konversi byte ke hex string
    }
    uid.toUpperCase(); // Konversi ke huruf besar

    if (uid != currentRfidUid) { // Hanya update jika UID berubah
      currentRfidUid = uid;
      Serial.print("RFID UID: ");
      Serial.println(currentRfidUid);
    }
    lastCardReadTime = currentMillis; // Perbarui waktu terakhir kartu terbaca
    mfrc522.PICC_HaltA(); // Hentikan PICC untuk mencegah pembacaan berulang
  } else {
    // Jika tidak ada kartu baru dan UID tidak "Belum Terbaca" dan sudah lewat delay
    if (currentRfidUid != "Belum Terbaca" && currentMillis - lastCardReadTime >= RFID_UID_CLEAR_DELAY_MS) {
      currentRfidUid = "Belum Terbaca";
      Serial.println("RFID UID dihapus (kartu dilepas)");
    }
  }
  // Tambahkan delay kecil untuk menghindari pembacaan berulang yang cepat
  delay(50);
}