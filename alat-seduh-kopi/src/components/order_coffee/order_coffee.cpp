/*
  src/components/order_coffee/order_coffee.cpp - Implementasi Komponen Order Kopi
  Mengelola input Push Button, output LED pada PCF8574 kedua (0x21),
  serta seluruh logika pemilihan dan pemrosesan menu kopi.
*/

#include "order_coffee.h" // Ubah include header ini sendiri
#include <Wire.h>        // Diperlukan untuk komunikasi I2C oleh Adafruit_PCF8574
#include <Arduino.h>     // Untuk Serial.print, dll.
#include "components/lcd_display/lcd_display.h" // Diperlukan untuk akses ke objek lcd

// --- Definisi Objek PCF8574 Kedua ---
Adafruit_PCF8574 pcf2;

// --- Definisi Variabel Global untuk Debounce ---
  unsigned long lastDebounceTime[4] = {0};
  unsigned long debounceDelay = 50;
  int lastButtonState[4] = {LOW, LOW, LOW, LOW};
  int currentButtonState[4] = {LOW, LOW, LOW, LOW};

// --- Definisi Variabel Global untuk Logika Menu Kopi ---
  int selectedMenu = 0; // 0=none, 1=Torabika, 2=Good Day, 3=ABC Susu
  bool menuConfirmed = false; // True jika tombol konfirmasi ditekan
  bool menuActive = false;   // True jika sedang dalam mode pemilihan/konfirmasi menu
  unsigned long menuProcessStartTime = 0; // Waktu mulai proses menu (untuk delay 5 detik)

// --- Definisi Variabel Global untuk Kontrol LED Blink ---
  unsigned long lastBlinkMillis = 0;
  bool ledState = LOW; // Status LED saat ini (LOW=ON untuk Common Anode)
  bool blinkingStoppedMessagePrinted = false;

// --- Implementasi Fungsi ---
  /**
   * @brief Menginisialisasi pin-pin front panel yang kini menjadi bagian dari modul order_coffee.
   * @param pcf2_address Alamat I2C PCF8574 yang digunakan untuk front panel.
   */

void setupOrderCoffee(uint8_t pcf2_address) { // Ubah nama fungsi setup
  // --- Inisialisasi Serial untuk debugging
  Serial.print("Menginisialisasi PCF8574 (0x");
  if (pcf2_address < 16) Serial.print("0");
  Serial.print(pcf2_address, HEX);
  Serial.println(" - Order Coffee Front Panel)... ");

  if (!pcf2.begin(pcf2_address, &Wire)) {
    Serial.println("PCF8574 (0x21) Inisialisasi GAGAL!");
    Serial.print("FATAL ERROR: PCF8574 (0x");
    if (pcf2_address < 16) Serial.print("0");
    Serial.print(pcf2_address, HEX);
    Serial.println(") TIDAK DITEMUKAN. Cek alamat & koneksi!");
    while(true); // Hentikan program jika PCF8574 kedua tidak ditemukan (fatal)
  }
  Serial.println("PCF8574 (Order Coffee Front Panel) OK!");

  // --- Pin Setup untuk Push Button
    // Mengatur pin-pin Push Button sebagai INPUT
    pcf2.pinMode(FP_PB1_PIN, INPUT); // P0 = Push button 1
    pcf2.pinMode(FP_PB2_PIN, INPUT); // P1 = Push button 2
    pcf2.pinMode(FP_PB3_PIN, INPUT); // P2 = Push button 3
    pcf2.pinMode(FP_PB4_PIN, INPUT); // P3 = Push button 4

  // --- Pin Setup untuk LED ---
    // Mengatur pin-pin LED sebagai OUTPUT
    pcf2.pinMode(FP_LED1_PIN, OUTPUT); // P4 = LED 1
    pcf2.pinMode(FP_LED2_PIN, OUTPUT); // P5 = LED 2

    // Pastikan semua LED mati di awal (HIGH untuk Common Anode, LOW untuk Common Cathode)
    pcf2.digitalWrite(FP_LED1_PIN, HIGH); // Matikan LED
    pcf2.digitalWrite(FP_LED2_PIN, HIGH); // Matikan LED
    Serial.println("Order Coffee Front Panel pins configured.");

  // --- Pin Setup untuk Motor Dinamo ---
    // Mengatur pin Dinamo sebagai OUTPUT
    pcf2.pinMode(DYNAMO_PIN, OUTPUT); // P0 = Dinamo
    pcf2.digitalWrite(DYNAMO_PIN, HIGH); // Pastikan dinamo OFF di awal (asumsi HIGH=OFF)


  }

// --- Implementasi Fungsi untuk Membaca Push Button dengan Debounce ---
  /**
   * @brief Membaca status push button dengan debounce.
   * @param buttonPin Pin PCF8574 yang terhubung ke tombol.
   * @param buttonIndex Indeks tombol (0-3) untuk array status.
   * @return true jika tombol baru saja ditekan (transisi LOW ke HIGH), false jika tidak.
   */

  bool readPushButton(int buttonPin, int buttonIndex) {
    // Validasi indeks tombol
    int reading = pcf2.digitalRead(buttonPin); // Baca status pin dari PCF8574

    if (reading != lastButtonState[buttonIndex]) {
      lastDebounceTime[buttonIndex] = millis();
    }

    if ((millis() - lastDebounceTime[buttonIndex]) > debounceDelay) {
      if (reading != currentButtonState[buttonIndex]) {
        currentButtonState[buttonIndex] = reading;
        // Deteksi tekanan saat HIGH (transisi dari LOW ke HIGH)
        if (currentButtonState[buttonIndex] == HIGH) { // Tombol baru saja ditekan
          Serial.print("Tombol PB");
          Serial.print(buttonIndex + 1);
          Serial.println(" ditekan (HIGH)");
          return true; // Mengembalikan true hanya saat transisi LOW ke HIGH
        } else {
          Serial.print("Tombol PB");
          Serial.print(buttonIndex + 1);
          Serial.println(" dilepas (LOW)");
        }
      }
    }
    lastButtonState[buttonIndex] = reading;
    return false;
  }


// --- Implementasi Fungsi untuk Kontrol LED Blink ---
  /**
   * @brief Memulai blinking untuk LED1 dan LED2.
   */

  void startBlinkingLEDs() {
      stopBlinkingLEDs(); // Pastikan berhenti jika sudah blinking
      lastBlinkMillis = millis();
      ledState = LOW; // LED ON (untuk Common Anode)
      pcf2.digitalWrite(FP_LED1_PIN, ledState);
      pcf2.digitalWrite(FP_LED2_PIN, ledState);
      Serial.println("LEDs mulai blinking.");
      blinkingStoppedMessagePrinted = false; // RESET FLAG DI SINI
  }

// --- Implementasi Fungsi untuk Kontrol LED Blink ---
  /**
   * @brief Menghentikan blinking dan mematikan LED1 dan LED2.
   */

  void stopBlinkingLEDs() {
    if (pcf2.digitalRead(FP_LED1_PIN) == LOW || pcf2.digitalRead(FP_LED2_PIN) == LOW || !blinkingStoppedMessagePrinted) {
      pcf2.digitalWrite(FP_LED1_PIN, HIGH); // LED OFF (untuk Common Anode)
      pcf2.digitalWrite(FP_LED2_PIN, HIGH); // LED OFF
      ledState = HIGH; // Set state ke OFF

      if (!blinkingStoppedMessagePrinted) { // CETAK HANYA JIKA BELUM DICETAK
          Serial.println("LEDs berhenti blinking.");
          blinkingStoppedMessagePrinted = true; // SET FLAG KE TRUE
      }
    }
  }

// --- Implementasi Fungsi untuk Memperbarui Status Blinking LED ---
  /**
   * @brief Memperbarui status blinking LED. Harus dipanggil di loop().
   * @param currentMillis Waktu millis() saat ini.
   */
  void updateBlinkingLEDs(unsigned long currentMillis) {
      if (currentMillis - lastBlinkMillis >= BLINK_INTERVAL) {
        lastBlinkMillis = currentMillis;
        ledState = !ledState; // Toggle state
        pcf2.digitalWrite(FP_LED1_PIN, ledState);
        pcf2.digitalWrite(FP_LED2_PIN, ledState);
        blinkingStoppedMessagePrinted = false; // RESET FLAG SAAT MULAI BLINK LAGI
      }
  }

// --- Fungsi Utama untuk Menangani Proses Order Kopi ---
  void handleOrderCoffee() { // Ubah nama fungsi handle
    unsigned long currentMillis = millis();

    // --- Logika Proses Menu (Delay 5 detik setelah konfirmasi) ---
    if (menuConfirmed && (currentMillis - menuProcessStartTime >= 5000)) {
      Serial.println("Proses menu 5 detik selesai. Kembali ke tampilan default.");
      selectedMenu = 0; // Reset menu
      menuConfirmed = false; // Reset konfirmasi
      menuActive = false; // Keluar dari mode menu
      stopBlinkingLEDs(); // Pastikan LED mati
      lcd.clear(); // Bersihkan LCD
      // LCD akan diupdate oleh main.ino loop() ke tampilan default
    }

    // --- Update status blinking LED ---
    // Panggil updateBlinkingLEDs hanya jika menu aktif dan belum dikonfirmasi
    if (menuActive && !menuConfirmed) {
        updateBlinkingLEDs(currentMillis);
    } else {
        stopBlinkingLEDs(); // Pastikan LED mati jika tidak dalam mode menu
    }

    // --- Baca status tombol dan deteksi tekan ---
    bool pb1Pressed = readPushButton(FP_PB1_PIN, 0);
    bool pb2Pressed = readPushButton(FP_PB2_PIN, 1);
    bool pb3Pressed = readPushButton(FP_PB3_PIN, 2);
    bool pb4Pressed = readPushButton(FP_PB4_PIN, 3);

    // --- Logika Pemilihan Menu (Hanya jika belum ada menu aktif atau sudah dikonfirmasi) ---
    if (!menuActive && !menuConfirmed) {
      if (pb1Pressed) {
        selectedMenu = 1; // Kopi Torabika
        menuActive = true;
        startBlinkingLEDs(); // Memulai blinking
        Serial.println("Menu: Kopi Torabika dipilih.");
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Kopi Torabika");
        lcd.setCursor(0, 3);
        lcd.print("> Seduh kopi");
      } else if (pb2Pressed) {
        selectedMenu = 2; // Kopi Good Day
        menuActive = true;
        startBlinkingLEDs(); // Memulai blinking
        Serial.println("Menu: Kopi Good Day dipilih.");
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Kopi Good Day");
        lcd.setCursor(0, 3);
        lcd.print("> Seduh kopi");
      } else if (pb3Pressed) {
        selectedMenu = 3; // Kopi ABC Susu
        menuActive = true;
        startBlinkingLEDs(); // Memulai blinking
        Serial.println("Menu: Kopi ABC Susu dipilih.");
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Kopi ABC Susu");
        lcd.setCursor(0, 3);
        lcd.print("> Seduh kopi");
      }
    }

    // --- Logika Konfirmasi (PB4) ---
    if (menuActive && !menuConfirmed && pb4Pressed) {
      menuConfirmed = true;
      menuProcessStartTime = currentMillis; // Catat waktu konfirmasi
      stopBlinkingLEDs(); // Hentikan blinking (pesan akan dicetak di sini)
      Serial.println("Menu dikonfirmasi! Memulai proses...");
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Memproses Kopi...");
      lcd.setCursor(0, 1);
      switch (selectedMenu) {
        case 1: lcd.print("Torabika"); break;
        case 2: lcd.print("Good Day"); break;
        case 3: lcd.print("ABC Susu"); break;
      }
      lcd.setCursor(0, 2); lcd.print("                    "); // Clear baris 2 & 3
      lcd.setCursor(0, 3); lcd.print("                    ");
    }
  }