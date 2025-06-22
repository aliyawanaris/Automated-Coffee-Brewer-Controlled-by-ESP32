#include "card_reader.h"
#include <SPI.h> // Diperlukan untuk komunikasi SPI

// Inisialisasi objek MFRC522 dengan pin SS dan RST yang telah didefinisikan
MFRC522 mfrc522(SS_PIN, RST_PIN);

void card_reader_init() {
    SPI.begin();        // Inisialisasi bus SPI
    mfrc522.PCD_Init(); // Inisialisasi modul MFRC522
    Serial.println("Card Reader (RFID RC522) diinisialisasi.");
    Serial.print("RFID Reader Version: ");
    mfrc522.PCD_DumpVersionToSerial(); // Menampilkan versi firmware RFID
}

bool card_reader_is_new_card_present() {
    // Mencari tag baru
    if ( ! mfrc522.PICC_IsNewCardPresent()) {
        return false;
    }
    // Memilih tag (membaca UID)
    if ( ! mfrc522.PICC_ReadCardSerial()) {
        return false;
    }
    return true;
}

String card_reader_read_card_uid() {
    String uidString = "";
    for (byte i = 0; i < mfrc522.uid.size; i++) {
        uidString += (mfrc522.uid.uidByte[i] < 0x10 ? "0" : "");
        uidString += String(mfrc522.uid.uidByte[i], HEX);
    }
    uidString.toUpperCase();
    return uidString;
}

bool card_reader_authenticate_block(byte blockAddr, MFRC522::MIFARE_Key* key, byte* uid, MFRC522::StatusCode* status) {
  // Fungsi otentikasi, jika Anda perlu membaca/menulis blok data
  // Pastikan untuk menghentikan enkripsi setelah operasi selesai
  *status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, blockAddr, key, uid);
  if (*status != MFRC522::STATUS_OK) {
      Serial.print(F("PCD_Authenticate() failed: "));
      // Menggunakan GetStatusText untuk mendapatkan deskripsi teks dari status code
      Serial.println(mfrc522.GetStatusText(*status));
      return false;
  }
  return true;
}

MFRC522::StatusCode card_reader_read_block(byte blockAddr, byte* buffer, byte* bufferSize) {
    // Fungsi membaca blok data, jika diperlukan
    return mfrc522.MIFARE_Read(blockAddr, buffer, bufferSize);
}

MFRC522::StatusCode card_reader_write_block(byte blockAddr, byte* buffer, byte bufferSize) {
    // Fungsi menulis blok data, jika diperlukan
    return mfrc522.MIFARE_Write(blockAddr, buffer, bufferSize);
}


void card_reader_halt() {
    mfrc522.PICC_HaltA();      // Menghentikan PICC yang dipilih
    mfrc522.PCD_StopCrypto1(); // Menghentikan enkripsi PCD
}