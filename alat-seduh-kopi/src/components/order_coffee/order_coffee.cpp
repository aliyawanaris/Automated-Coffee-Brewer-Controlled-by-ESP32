/*
  src/components/order_coffee/order_coffee.cpp - Implementasi Komponen Order Kopi
  Mengelola input Push Button, output LED pada PCF8574 kedua (0x21),
  serta seluruh logika pemilihan dan pemrosesan menu kopi.
*/

#include "order_coffee.h"
#include <Wire.h>
#include <Arduino.h>
#include "components/lcd_display/lcd_display.h"
#include "components/motor_control/motor_control.h"
#include "components/rfid_card_reader/rfid_card_reader.h"

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
bool menuActive = false;    // True jika sedang dalam mode pemilihan/konfirmasi menu
unsigned long menuProcessStartTime = 0; // Waktu mulai proses menu (untuk delay 5 detik)

// --- Definisi Variabel Global untuk Kontrol LED Blink ---
unsigned long lastBlinkMillis = 0;
bool ledState = HIGH; // Status LED saat ini (HIGH=OFF, LOW=ON untuk Common Anode)
bool blinkingStoppedMessagePrinted = false; // Untuk mencegah pesan berulang

// --- Variabel Global untuk Mode Menu RFID ---
bool rfidMenuMode = false; // True jika menu diaktifkan melalui RFID

// --- Fungsi Baru: Menampilkan Tampilan Menu Idle/Awal ---
void displayIdleMenu() {
    Serial.println("[LCD] Menampilkan menu idle...");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Coffee WD           "); // Baris 0
    lcd.setCursor(0, 1);
    lcd.print("                    "); // Baris 1 dikosongkan
    lcd.setCursor(0, 2);
    lcd.print("Silahkan Tap Kartu  "); // Baris 2
    lcd.setCursor(0, 3);
    lcd.print("                    "); // Baris 3 dikosongkan
}

// --- Implementasi Fungsi setupOrderCoffee ---
void setupOrderCoffee(uint8_t pcf2_address) {
    Serial.print("[OrderCoffee] Menginisialisasi PCF8574 (0x");
    if (pcf2_address < 16) Serial.print("0");
    Serial.print(pcf2_address, HEX);
    Serial.println(" - Order Coffee Front Panel)... ");

    if (!pcf2.begin(pcf2_address, &Wire)) {
        Serial.println("[OrderCoffee] FATAL ERROR: PCF8574 (0x" + String(pcf2_address, HEX) + ") TIDAK DITEMUKAN. Cek alamat & koneksi!");
        while(true); // Hentikan eksekusi jika PCF8574 tidak ditemukan
    }
    Serial.println("[OrderCoffee] PCF8574 (Order Coffee Front Panel) OK!");

    // Set pinmode untuk Push Button (Input)
    pcf2.pinMode(FP_PB1_PIN, INPUT);
    pcf2.pinMode(FP_PB2_PIN, INPUT);
    pcf2.pinMode(FP_PB3_PIN, INPUT);
    pcf2.pinMode(FP_PB4_PIN, INPUT);

    // Set pinmode untuk LED (Output)
    pcf2.pinMode(FP_LED1_PIN, OUTPUT);
    pcf2.pinMode(FP_LED2_PIN, OUTPUT);

    // Matikan semua LED di awal (HIGH untuk Common Anode)
    pcf2.digitalWrite(FP_LED1_PIN, HIGH);
    pcf2.digitalWrite(FP_LED2_PIN, HIGH);
    Serial.println("[OrderCoffee] Order Coffee Front Panel pins configured.");
}

// --- Implementasi Fungsi readPushButton ---
bool readPushButton(int buttonPin, int buttonIndex) {
    int reading = pcf2.digitalRead(buttonPin);

    // Jika terjadi perubahan state, reset timer debounce
    if (reading != lastButtonState[buttonIndex]) {
        lastDebounceTime[buttonIndex] = millis();
    }

    // Jika waktu debounce sudah lewat dan state masih sama
    if ((millis() - lastDebounceTime[buttonIndex]) > debounceDelay) {
        if (reading != currentButtonState[buttonIndex]) {
            currentButtonState[buttonIndex] = reading; // Update state tombol
            if (currentButtonState[buttonIndex] == HIGH) { // Asumsi Active HIGH
                Serial.println("[OrderCoffee] Tombol PB" + String(buttonIndex + 1) + " ditekan.");
                return true;
            } else {
                Serial.println("[OrderCoffee] Tombol PB" + String(buttonIndex + 1) + " dilepas.");
            }
        }
    }
    lastButtonState[buttonIndex] = reading; // Simpan state terakhir untuk debounce
    return false;
}

// --- Implementasi Fungsi startBlinkingLEDs ---
void startBlinkingLEDs() {
    stopBlinkingLEDs(); // Pastikan LED mati sebelum mulai blinking
    lastBlinkMillis = millis();
    ledState = LOW; // LOW untuk ON pada common anode
    pcf2.digitalWrite(FP_LED1_PIN, ledState);
    pcf2.digitalWrite(FP_LED2_PIN, ledState);
    Serial.println("[OrderCoffee] LEDs mulai blinking.");
    blinkingStoppedMessagePrinted = false; // Reset flag agar pesan "berhenti" bisa dicetak lagi
}

// --- Implementasi Fungsi stopBlinkingLEDs ---
void stopBlinkingLEDs() {
    // Hanya matikan LED jika mereka saat ini menyala (LOW) atau sedang blinking
    if (pcf2.digitalRead(FP_LED1_PIN) == LOW || pcf2.digitalRead(FP_LED2_PIN) == LOW || !blinkingStoppedMessagePrinted) {
        pcf2.digitalWrite(FP_LED1_PIN, HIGH); // Matikan LED (HIGH untuk common anode)
        pcf2.digitalWrite(FP_LED2_PIN, HIGH);
        ledState = HIGH; // Set status ke mati

        if (!blinkingStoppedMessagePrinted) {
            Serial.println("[OrderCoffee] LEDs berhenti blinking.");
            blinkingStoppedMessagePrinted = true; // Tandai bahwa pesan sudah dicetak
        }
    }
}

// --- Implementasi Fungsi updateBlinkingLEDs ---
void updateBlinkingLEDs(unsigned long currentMillis) {
    if (currentMillis - lastBlinkMillis >= BLINK_INTERVAL) {
      lastBlinkMillis = currentMillis;
      ledState = !ledState; // Toggle LED state
      pcf2.digitalWrite(FP_LED1_PIN, ledState);
      pcf2.digitalWrite(FP_LED2_PIN, ledState);
      blinkingStoppedMessagePrinted = false; // Reset agar pesan bisa dicetak lagi jika blinking berhenti
    }
}

// --- Implementasi Fungsi selectCoffeeMenu ---
void selectCoffeeMenu(int menuId) {
    if (!menuConfirmed) { // Hanya bisa memilih menu jika belum dikonfirmasi
        selectedMenu = menuId;
        menuActive = true;
        startBlinkingLEDs(); // Mulai blinking LED saat menu dipilih

        Serial.print("[OrderCoffee] Menu: ");
        lcd.clear(); // Hapus tampilan sebelumnya
        lcd.setCursor(0, 0);

        switch (menuId) {
            case 1:
                Serial.println("Kopi Torabika dipilih.");
                lcd.print("Kopi Torabika       ");
                break;
            case 2:
                Serial.println("Kopi Good Day dipilih.");
                lcd.print("Kopi Good Day       ");
                break;
            case 3:
                Serial.println("Kopi ABC Susu dipilih.");
                lcd.print("Kopi ABC Susu       ");
                break;
            default:
                Serial.println("Pilihan menu tidak valid.");
                selectedMenu = 0;
                menuActive = false;
                stopBlinkingLEDs(); // Berhenti blinking jika pilihan tidak valid
                lcd.print("Pilihan tidak valid!");
                break;
        }

        if (selectedMenu != 0) { // Jika pilihan valid, tampilkan "> Seduh kopi"
            lcd.setCursor(0, 1);
            lcd.print("> Seduh kopi        ");
            lcd.setCursor(0, 2); lcd.print("                    "); // Bersihkan baris 2
            lcd.setCursor(0, 3); lcd.print("                    "); // Bersihkan baris 3
        }
    } else {
        Serial.println("[OrderCoffee] Sistem sedang dalam proses menu atau sudah dikonfirmasi. Tidak dapat memilih menu baru.");
    }
}

// --- Implementasi Fungsi handleOrderCoffee ---
void handleOrderCoffee() {
  unsigned long currentMillis = millis();

  // --- [1] Logika Reset Sistem Setelah Proses Menu Selesai ---
  // Menu dikonfirmasi dan sudah 5 detik berlalu sejak proses dimulai
  if (menuConfirmed && (currentMillis - menuProcessStartTime >= 5000)) {
    Serial.println("[OrderCoffee] Proses menu selesai (5 detik). Mereset sistem ke mode idle.");

    selectedMenu = 0;
    menuConfirmed = false;
    menuActive = false;
    rfidMenuMode = false; // Reset mode RFID
    rfidErrorActive = false; // Pastikan error RFID juga direset

    stopBlinkingLEDs();   // Berhenti blinking setelah proses selesai
    displayIdleMenu();    // Kembali ke tampilan idle setelah proses selesai
  }

  // --- [2] Update Status LED Blinking ---
  // LED hanya blinking saat menu aktif tapi belum dikonfirmasi
  if (menuActive && !menuConfirmed) {
      updateBlinkingLEDs(currentMillis);
  } else {
      stopBlinkingLEDs();
  }

  // --- [3] Pembacaan Tombol Front Panel ---
  bool pb1Pressed = readPushButton(FP_PB1_PIN, 0); // Tombol menu 1
  bool pb2Pressed = readPushButton(FP_PB2_PIN, 1); // Tombol menu 2
  bool pb3Pressed = readPushButton(FP_PB3_PIN, 2); // Tombol menu 3
  bool pb4Pressed = readPushButton(FP_PB4_PIN, 3); // Tombol konfirmasi

  // --- [4] Logika Deteksi Kartu RFID dan Masuk Mode Pemilihan Menu ---
  // Masuk mode pemilihan kopi HANYA jika kartu RFID terdeteksi DAN sistem benar-benar idle
  // (Tidak ada menu aktif, belum dikonfirmasi, bukan mode menu RFID yang sedang berlangsung,
  // dan tidak ada error RFID aktif)
  if (!currentRfidUid.isEmpty() && !menuActive && !menuConfirmed && !rfidMenuMode && !rfidErrorActive) {
      // Serial.println("[OrderCoffee] Kartu RFID terdeteksi: " + currentRfidUid + ". Memproses kartu...");

      // processRfidMenuSelection akan mengecek apakah kartu terdaftar
      // Fungsi ini yang akan mengeset rfidMenuMode dan menuActive jika kartu terdaftar
      processRfidMenuSelection(currentRfidUid);

      // Jika processRfidMenuSelection berhasil mengaktifkan mode menu RFID
      if (rfidMenuMode) {
        Serial.println("[OrderCoffee] Mode pemilihan menu RFID diaktifkan. Tampilkan pilihan kopi di LCD.");
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Pilih Kopi:");
        lcd.setCursor(0, 1);
        lcd.print("1. Torabika");
        lcd.setCursor(0, 2);
        lcd.print("2. Good Day");
        lcd.setCursor(0, 3);
        lcd.print("3. ABC Susu");
        startBlinkingLEDs(); // Mulai blinking LED saat masuk mode pemilihan
      } else {
        // Jika kartu terdeteksi tapi tidak valid/terdaftar
        // Serial.println("[OrderCoffee] Kartu RFID terdeteksi, tapi tidak terdaftar atau tidak valid.");
        // Logika untuk menampilkan pesan error di LCD untuk durasi tertentu ada di rfid_card_reader.cpp
      }
      // currentRfidUid tidak direset di sini; akan direset oleh rfid_card_reader.cpp setelah durasi tampil.
  }

  // --- [5] Logika Pemilihan Menu oleh Pengguna (Tombol 1, 2, 3) ---
  // Hanya jika sudah dalam mode pemilihan menu (menuActive) dan belum dikonfirmasi
  if (menuActive && !menuConfirmed) {
      if (pb1Pressed) {
          selectCoffeeMenu(1);
          // Logikanya sudah di dalam selectCoffeeMenu
      } else if (pb2Pressed) {
          selectCoffeeMenu(2);
          // Logikanya sudah di dalam selectCoffeeMenu
      } else if (pb3Pressed) {
          selectCoffeeMenu(3);
          // Logikanya sudah di dalam selectCoffeeMenu
      }
  }

  // --- [6] Logika Konfirmasi Menu (Tombol 4) ---
  // Konfirmasi hanya jika menu aktif, belum dikonfirmasi, dan ada menu yang sudah dipilih
  if (menuActive && !menuConfirmed && pb4Pressed && selectedMenu != 0) {
    menuConfirmed = true;
    menuProcessStartTime = currentMillis;
    stopBlinkingLEDs(); // Berhenti blinking setelah konfirmasi

    Serial.println("--- [OrderCoffee] Menu " + String(selectedMenu) + " dikonfirmasi! Memulai proses kopi... ---");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Memproses Kopi...");
    lcd.setCursor(0, 1); lcd.print("                ");
    lcd.setCursor(0, 2); lcd.print("                ");
    lcd.setCursor(0, 3); lcd.print("                ");

    // --- [6.1] Proses Kontrol Motor ---
    Serial.println("[MotorControl] Memulai pompa galon...");
    motor_pump_galon_start();
    delay(7000);
    motor_pump_galon_stop();
    Serial.println("[MotorControl] Pompa galon selesai.");
    delay(1000);

    Serial.println("[MotorControl] Memulai pompa air panas...");
    motor_pump_hot_water_start();
    delay(7000);
    motor_pump_hot_water_stop();
    Serial.println("[MotorControl] Pompa air panas selesai.");
    delay(1000);

    Serial.println("[MotorControl] Memutar motor storage 1 (Kopi Torabika)...");
    motor_storage_1_start(250);
    delay(1000);
    motor_storage_1_stop();
    Serial.println("[MotorControl] Motor storage 1 selesai.");
    delay(1000);

    Serial.println("[MotorControl] Memutar motor storage 2 (Kopi Good Day)...");
    motor_storage_2_start(250);
    delay(1000);
    motor_storage_2_stop();
    Serial.println("[MotorControl] Motor storage 2 selesai.");
    delay(1000);

    Serial.println("[MotorControl] Memutar motor storage 3 (Kopi ABC Susu)...");
    motor_storage_3_start(250);
    delay(1000);
    motor_storage_3_stop();
    Serial.println("[MotorControl] Motor storage 3 selesai.");
    delay(1000);

    Serial.println("[MotorControl] Memulai mixer pertama...");
    motor_mixer_start(200);
    delay(5000);
    motor_mixer_stop();
    Serial.println("[MotorControl] Mixer pertama selesai.");
    delay(2000);

    Serial.println("[MotorControl] Memulai mixer kedua...");
    motor_mixer_start(200);
    delay(5000);
    motor_mixer_stop();
    Serial.println("[MotorControl] Mixer kedua selesai.");
    delay(1000);

    // Mengingat "Selenoid valve GPIO33 adalah 'Selenoid seduh kopi'."
    Serial.println("[MotorControl] Memulai Selenoid seduh kopi (GPIO33)...");
    motor_pump_seduh_kopi_start();
    delay(1000);
    motor_pump_seduh_kopi_stop();
    Serial.println("[MotorControl] Selenoid seduh kopi selesai.");
    delay(1000);

    // Setelah proses selesai, tampilkan "Kopi Siap!" di LCD
    Serial.print("[OrderCoffee] Kopi ");
    switch (selectedMenu) {
      case 1:
        lcd.setCursor(0, 0); lcd.print("Kopi Torabika   ");
        Serial.println("Torabika Siap!"); break;
      case 2:
        lcd.setCursor(0, 0); lcd.print("Kopi Good Day   ");
        Serial.println("Good Day Siap!"); break;
      case 3:
        lcd.setCursor(0, 0); lcd.print("Kopi ABC Susu   ");
        Serial.println("ABC Susu Siap!"); break;
      default:
        lcd.setCursor(0, 0); lcd.print("Kopi Siap!      ");
        Serial.println("Siap! (Menu tidak diketahui)"); break;
    }
    lcd.setCursor(0, 1); lcd.print("Siap!           ");
    lcd.setCursor(0, 2); lcd.print("                ");
    lcd.setCursor(0, 3); lcd.print("                ");
  }

  // --- [7] Kontrol Tampilan Idle Menu ---
  // displayIdleMenu hanya akan dipanggil jika sistem benar-benar idle
  // dan tidak ada pesan error RFID yang sedang aktif
  static bool idleMenuDisplayed = false; // Flag untuk mencegah refresh berlebihan

  if (!menuActive && !menuConfirmed && !rfidErrorActive && !idleMenuDisplayed) {
      displayIdleMenu();
      // Log sudah ada di displayIdleMenu()
      idleMenuDisplayed = true;
  } else if (menuActive || menuConfirmed || rfidErrorActive) {
      if (idleMenuDisplayed) { // Hanya log jika flag direset
          Serial.println("[OrderCoffee] Aktivitas terdeteksi. Menyembunyikan Idle Menu.");
      }
      idleMenuDisplayed = false; // Reset flag saat ada aktivitas
  }
}