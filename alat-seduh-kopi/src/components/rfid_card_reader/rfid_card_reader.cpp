#include "rfid_card_reader.h"
#include <SPI.h> // Diperlukan untuk komunikasi SPI (biasanya sudah di-include di main.ino, tapi pastikan ada)

MFRC522 mfrc522(SS_PIN, RST_PIN); // Buat objek MFRC522

// Definisi global variables (deklarasi extern di .h)
String currentRfidUid = "Belum Terbaca"; // Default value
unsigned long lastRfidReadMillis = 0;
const long RFID_DISPLAY_DURATION_MS = 5000; // UID akan ditampilkan selama 5 detik

void setupRfidCardReader() {
  // Pastikan SPI.begin() dipanggil sekali di setup() utama (main.ino)
  // Atau bisa juga dipanggil di sini jika yakin tidak ada komponen lain yang menggunakan SPI
  // SPI.begin();
  mfrc522.PCD_Init(); // Inisialisasi MFRC522
  Serial.println("Inisialisasi RFID RC522 selesai.");
}

void handleRfidCardReader(unsigned long currentMillis) {
  // Logika untuk mereset UID setelah durasi tertentu
  if (currentRfidUid != "Belum Terbaca" && (currentMillis - lastRfidReadMillis > RFID_DISPLAY_DURATION_MS)) {
    currentRfidUid = "Belum Terbaca";
    Serial.println("RFID UID direset ke 'Belum Terbaca'.");
  }

  // Periksa apakah ada kartu baru yang hadir
  if (mfrc522.PICC_IsNewCardPresent()) {
    // Pilih salah satu kartu (penting untuk membaca data dari kartu)
    if (mfrc522.PICC_ReadCardSerial()) {
      String uidString = "";
      for (byte i = 0; i < mfrc522.uid.size; i++) {
        uidString += (mfrc522.uid.uidByte[i] < 0x10 ? "0" : ""); // Tambah "0" di depan jika byte < 16
        uidString += String(mfrc522.uid.uidByte[i], HEX); // Konversi byte ke heksadesimal
      }
      uidString.toUpperCase(); // Ubah ke huruf besar

      // Update UID hanya jika berbeda dari yang terakhir dibaca
      if (currentRfidUid != uidString) {
          currentRfidUid = uidString;
          lastRfidReadMillis = currentMillis; // Simpan waktu baca
          Serial.print("Kartu RFID Terdeteksi! UID: ");
          Serial.println(currentRfidUid);
      }
      // Jika UID sama, reset timer untuk tetap menampilkannya
      else {
          lastRfidReadMillis = currentMillis;
      }
    }
  }
}