#include "rfid_card_reader.h" // Mengimpor definisi dari header file ini

// --- Definisi Objek MFRC522 ---
// Objek 'rfid' didefinisikan di sini, menggunakan pin SS dan RST yang telah ditentukan.
MFRC522 rfid(SS_PIN, RST_PIN);

// --- Definisi Variabel Global ---
// Variabel untuk menyimpan UID kartu yang terbaca.
// Diinisialisasi dengan status "Belum Terbaca".
String currentRfidUid = "Belum Terbaca";

// --- Variabel Internal untuk Logika RFID ---
// Ini adalah variabel internal untuk komponen RFID, tidak perlu 'extern'
unsigned long lastCardDetectedMillis = 0;
const long CARD_DISPLAY_DURATION = 5000; // Tampilkan UID selama 5 detik setelah kartu dilepas

// --- Implementasi Fungsi Inisialisasi RFID ---
void setupRfidCardReader() {
  // Catatan: SPI.begin() dipanggil di main.cpp, jadi tidak perlu di sini lagi.

  // Inisialisasi modul MFRC522.
  // Ini mempersiapkan chip untuk operasi pembacaan kartu.
  rfid.PCD_Init();

  // --- Pemeriksaan Inisialisasi Modul RFID ---
  // Membaca register versi firmware chip MFRC522.
  // Nilai 0x00 atau 0xFF biasanya menunjukkan masalah koneksi atau modul rusak.
  byte version = rfid.PCD_ReadRegister(MFRC522::VersionReg);
  if (version == 0x00 || version == 0xFF) {
    Serial.println("==========================================");
    Serial.println("  RFID RC522 Inisialisasi GAGAL!");
    Serial.println("  *** MOHON PERIKSA KONEKSI HARDWARE ANDA: ***");
    Serial.println("  1. Pastikan semua pin SPI (SS, SCK, MISO, MOSI, RST) terhubung dengan benar.");
    Serial.println("     (SS_PIN: GPIO5, RST_PIN: GPIO2, SCK: GPIO18, MISO: GPIO19, MOSI: GPIO23)");
    Serial.println("  2. Pastikan daya (VCC) terhubung ke 3.3V dari ESP32, BUKAN 5V.");
    Serial.println("  3. Pastikan GND terhubung dengan baik.");
    Serial.println("  4. Coba gunakan kabel jumper yang berbeda atau lebih pendek.");
    Serial.println("  5. Pertimbangkan kemungkinan modul RFID rusak jika semua di atas sudah benar.");
    Serial.println("==========================================");
    // Hentikan eksekusi program di sini untuk mempermudah debugging masalah hardware.
    while (true);
  } else {
    // Jika inisialisasi berhasil, tampilkan versi firmware chip.
    Serial.println("==========================================");
    Serial.println("  RFID RC522 Inisialisasi Berhasil!");
    Serial.print("  RFID Reader Firmware Version: ");
    rfid.PCD_DumpVersionToSerial(); // Fungsi library untuk mencetak versi firmware
    Serial.println("  Siap memindai kartu...");
    Serial.println("==========================================");
  }
}

// --- Implementasi Fungsi Penanganan RFID di Loop ---
void handleRfidCardReader(unsigned long currentMillis) {
  // --- Langkah 1: Periksa apakah ada kartu RFID/NFC baru di dekat antena ---
  if (rfid.PICC_IsNewCardPresent()) {
    // --- Langkah 2: Jika kartu terdeteksi, coba baca UID (NUID) dari kartu tersebut ---
    if (rfid.PICC_ReadCardSerial()) {
      // UID berhasil dibaca, update currentRfidUid
      String newUid = "";
      for (byte i = 0; i < rfid.uid.size; i++) {
        if (rfid.uid.uidByte[i] < 0x10) {
          newUid += "0";
        }
        newUid += String(rfid.uid.uidByte[i], HEX);
      }
      newUid.toUpperCase(); // Ubah ke huruf besar untuk konsistensi

      if (newUid != currentRfidUid) { // Hanya update jika UID berbeda
        currentRfidUid = newUid;
        Serial.print("UID Kartu Terbaca: ");
        Serial.println(currentRfidUid);

        MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);
        Serial.print("Tipe PICC (Kartu): ");
        Serial.println(rfid.PICC_GetTypeName(piccType));
        Serial.println("-----------------------------------------");
      }
      lastCardDetectedMillis = currentMillis; // Reset timer karena kartu masih ada

      // --- Menghentikan Komunikasi dengan Kartu ---
      // Penting agar kartu yang sama tidak terbaca berulang kali di setiap iterasi loop
      // dan memungkinkan deteksi kartu baru ketika kartu sebelumnya dipindahkan.
      rfid.PICC_HaltA();
      rfid.PCD_StopCrypto1();
    }
  } else {
    // Jika tidak ada kartu baru yang terdeteksi, periksa apakah ada kartu yang sebelumnya terbaca
    // dan sudah waktunya untuk menghapus tampilannya.
    if (currentRfidUid != "Belum Terbaca" && (currentMillis - lastCardDetectedMillis > CARD_DISPLAY_DURATION)) {
      Serial.println("Kartu dilepas atau tidak terdeteksi lagi. Mereset UID.");
      currentRfidUid = "Belum Terbaca";
    }
  }
}