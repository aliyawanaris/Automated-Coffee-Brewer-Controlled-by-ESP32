#include "rfid_card_reader.h"
#include "components/lcd_display/lcd_display.h" // Untuk update LCD
#include "components/order_coffee/order_coffee.h" // Untuk memanggil fungsi pemilihan menu

MFRC522 mfrc522(SS_PIN, RST_PIN); // Buat objek MFRC522

// Definisi global variables (deklarasi extern di .h)
String currentRfidUid = "Belum Terbaca"; // Default value
unsigned long lastRfidReadMillis = 0;
const long RFID_DISPLAY_DURATION_MS = 5000; // UID akan ditampilkan selama 5 detik

bool rfidErrorActive = false; // Flag untuk menunjukkan pesan error RFID aktif
unsigned long rfidErrorStartTime = 0;
const long RFID_ERROR_DISPLAY_DURATION_MS = 3000; // Durasi tampilan pesan error RFID

void setupRfidCardReader() {
    mfrc522.PCD_Init(); // Inisialisasi MFRC522
    Serial.println("[RFID] Inisialisasi RFID RC522 selesai.");
}

void handleRfidCardReader(unsigned long currentMillis) {
    // --- [1] Logika Reset UID dan Pesan Error RFID ---
    // Reset UID setelah durasi tertentu jika tidak ada kartu yang terdeteksi lagi
    if (currentRfidUid != "Belum Terbaca" && (currentMillis - lastRfidReadMillis > RFID_DISPLAY_DURATION_MS)) {
        Serial.println("[RFID] RFID UID direset ke 'Belum Terbaca' setelah " + String(RFID_DISPLAY_DURATION_MS) + "ms.");
        delay(1000); // Delay untuk memberi waktu pembaca kartu
        currentRfidUid = "Belum Terbaca";
        // Tidak langsung update LCD di sini karena displayIdleMenu() di handleOrderCoffee() akan menanganinya
    }

    // Reset pesan error RFID setelah durasi tertentu
    if (rfidErrorActive && (currentMillis - rfidErrorStartTime > RFID_ERROR_DISPLAY_DURATION_MS)) {
        Serial.println("[RFID] Pesan error RFID direset setelah " + String(RFID_ERROR_DISPLAY_DURATION_MS) + "ms.");
        delay(1000); // Delay untuk memberi waktu pembaca kartu
        rfidErrorActive = false;
        // LCD akan kembali ke idle menu melalui handleOrderCoffee()
    }

    // --- [2] Deteksi dan Pembacaan Kartu RFID ---
    // Periksa apakah ada kartu baru yang hadir atau kartu yang sama masih ada
    if (mfrc522.PICC_IsNewCardPresent() || mfrc522.PICC_ReadCardSerial()) {
        if (mfrc522.PICC_ReadCardSerial()) {
            String uidString = "";
            for (byte i = 0; i < mfrc522.uid.size; i++) {
                uidString += (mfrc522.uid.uidByte[i] < 0x10 ? "0" : "");
                uidString += String(mfrc522.uid.uidByte[i], HEX);
            }
            uidString.toUpperCase();

            // Hanya proses jika UID yang terbaca berbeda dari yang terakhir
            // atau jika currentRfidUid sudah direset ("Belum Terbaca")
            if (currentRfidUid != uidString || currentRfidUid == "Belum Terbaca") {
                currentRfidUid = uidString;
                lastRfidReadMillis = currentMillis; // Perbarui waktu terakhir baca
                Serial.println("[RFID] Kartu RFID Terdeteksi! UID: " + currentRfidUid);

                // Panggil fungsi untuk memproses pilihan menu dari RFID.
                // Fungsi inilah yang sekarang akan menangani lcd.clear() dan tampilan ID.
                processRfidMenuSelection(currentRfidUid);

            } else {
                // Kartu yang sama masih terdeteksi, perbarui waktu baca agar tidak direset
                lastRfidReadMillis = currentMillis;
            }

            mfrc522.PICC_HaltA(); // Hentikan PICC untuk mencegah pembacaan ganda
            mfrc522.PCD_StopCrypto1(); // Hentikan enkripsi untuk kartu yang dipilih
        }
    }
}

// --- Implementasi fungsi untuk memproses pilihan menu dari RFID ---
void processRfidMenuSelection(String rfidUid) {
    bool isCardRegistered = false; // Flag untuk mengecek apakah kartu terdaftar
    int menuId = -1; // Variabel untuk menyimpan ID menu yang dipilih

    if (rfidUid == "091CD54B") { // Ganti dengan UID Kartu Torabika Anda
        Serial.println("[RFID] Kartu 'Torabika' terdeteksi. Memilih menu 1.");
        menuId = 1;
        isCardRegistered = true;
    } else if (rfidUid == "A903E84B") { // Ganti dengan UID Kartu Good Day Anda
        Serial.println("[RFID] Kartu 'Good Day' terdeteksi. Memilih menu 2.");
        menuId = 1;
        isCardRegistered = true;
    } else if (rfidUid == "C915EAA3") { // Ganti dengan UID Kartu ABC Susu Anda
        Serial.println("[RFID] Kartu 'ABC Susu' terdeteksi. Memilih menu 3.");
        menuId = 1;
        isCardRegistered = true;
    } else {
        Serial.println("[RFID] Kartu RFID tidak dikenal: " + rfidUid + ". Menampilkan pesan error.");
        delay(1000); // Delay untuk memberi waktu pembaca kartu
        rfidErrorActive = true; // Aktifkan flag error
        rfidErrorStartTime = millis(); // Catat waktu mulai error
        isCardRegistered = false; // Kartu tidak terdaftar
    }

    // Tampilkan ID di baris ke-2 fisik LCD (indeks 1 di kode) untuk semua kasus
    // Ini adalah baris yang Anda maksud sebagai "baris 4" sebelumnya
    // lcd.clear(); // Bersihkan layar LCD sebelum menampilkan ID
    // lcd.setCursor(0, 1);
    // lcd.print("ID: " + rfidUid + "    "); // Pastikan space cukup untuk menimpa
    // delay(2000); // Tampilkan ID selama 2 detik

    if (isCardRegistered) {
        // KARTU TERDAFTAR: JANGAN LAKUKAN lcd.clear()
        // Langsung panggil fungsi selectCoffeeMenu dan atur mode RFID
        Serial.println("[RFID] Mengarahkan ke menu ID: " + String(menuId));
        selectCoffeeMenu(menuId); // Memilih menu dan menampilkan di LCD
        setRfidMenuMode(true); // Mengatur flag agar sistem tahu menu dipilih via RFID
    } else {
        setRfidMenuMode(false); // Pastikan mode RFID dimatikan jika ada error
    }
}