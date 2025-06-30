/*
  Judul Proyek: Sistem Monitoring & Kontrol Mesin Kopi Berbasis Web & LCD I2C

  Deskripsi:
  Kode ini mengimplementasikan server web asinkron pada ESP32 untuk:
  1. Menghubungkan ke jaringan Wi-Fi.
  2. Melayani file web (index.html) dari SPIFFS.
  3. Menggunakan WebSocket untuk komunikasi real-time dua arah.
  4. Mengontrol status sebuah relay (yang terhubung ke motor atau perangkat lain)
     melalui pesan WebSocket dari klien web.
  5. Membaca data dari sensor jarak (asumsi HC-SR04 atau sejenisnya)
     melalui library 'storage_detector' dan mengirimkannya ke klien web secara periodik.
  6. Menampilkan status sistem, IP Address, dan data sensor pada LCD I2C 20x4.
  7. Melakukan pemindaian I2C saat startup untuk mendeteksi perangkat I2C yang terhubung
     dan mengirimkan hasilnya ke klien web.
  8. Mengelola input dari 4 push button dan mengontrol 2 LED pada PCF8574 kedua (0x21)
     melalui modul 'front_panel' yang terpisah, dan menampilkan status tombol di web.
  9. Membaca UID dari modul RFID RC522 melalui modul 'rfid_card_reader' yang terpisah,
     dan menampilkannya pada Serial Monitor, LCD, dan halaman web.

  Asumsi:
  - Ada file 'index.html' yang tersimpan di SPIFFS.
  - Library 'storage_detector' berfungsi untuk membaca sensor jarak.
  - 'motorPin' (GPIO 4) terhubung ke relay yang mengontrol motor,
    dengan logika aktif LOW (HIGH = OFF, LOW = ON).
  - Pin-pin untuk sensor jarak (SD_TRIG_PIN_1, SD_ECHO_PIN_1, dst.)
    didefinisikan di tempat lain, kemungkinan di dalam 'storage_detector.h' atau file serupa.
  - LCD I2C 20x4 terhubung ke GPIO 21 (SDA) dan GPIO 22 (SCL).
  - LED pada PCF8574 menggunakan konfigurasi Common Anode (LOW = ON, HIGH = OFF).
  - Push button pada PCF8574 menggunakan koneksi pull-down eksternal (HIGH saat ditekan, LOW saat dilepas).
  - RFID RC522 terhubung sesuai image_27db9a.png.
*/

// --- Bagian 1: Inklusi Library & Komponen ---
#include <Arduino.h>
#include <WiFi.h>
#include <SPIFFS.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include <vector> // Diperlukan untuk std::vector
#include <Wire.h> // Diperlukan untuk komunikasi I2C
#include <LiquidCrystal_I2C.h> // Diperlukan untuk mengontrol LCD I2C

// Inklusi Komponen Lokal
#include "components/storage_detector/storage_detector.h"
#include "components/front_panel/front_panel.h"
#include "components/rfid_card_reader/rfid_card_reader.h"


// --- Bagian 2: Konfigurasi Wi-Fi ---
const char* ssid = "Pikan Miku";
const char* password = "jasamaru123";

// --- Bagian 3: Konfigurasi Perangkat Keras & Variabel Status ---
const int motorPin = 4;
bool motorState = LOW;


// --- Bagian 4: Objek Server Web & WebSocket ---
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

// --- Bagian 5: Pengaturan Interval Pembacaan Sensor ---
unsigned long lastSensorReadMillis = 0;
const long sensorReadInterval = 2000;

// --- Bagian 6: Objek LCD I2C ---
#define LCD_ADDRESS 0x27
#define LCD_COLUMNS 20
#define LCD_ROWS    4
LiquidCrystal_I2C lcd(LCD_ADDRESS, LCD_COLUMNS, LCD_ROWS);

// --- Bagian 7: Definisi Alamat PCF8574 ---
#define PCF8574_ADDRESS_1 0x20 // Alamat I2C PCF8574 pertama (jika digunakan)
#define PCF8574_ADDRESS_2 0x21 // Alamat I2C PCF8574 kedua (untuk Front Panel)
Adafruit_PCF8574 pcf1; // Dideklarasikan di main.cpp jika masih diperlukan di sini


// --- Bagian 8: GLOBAL VARIABLE UNTUK MENYIMPAN HASIL I2C SCAN ---
String i2cScanResultsJson = "{\"type\":\"i2cScan\",\"addresses\":[]}";


// --- Bagian 9: Fungsi Callback WebSocket ---
void onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client,
               AwsEventType type, void *arg, uint8_t *data, size_t len) {
    if(type == WS_EVT_CONNECT){
        Serial.printf("WebSocket client #%u connected\n", client->id());
        String jsonRelay = "{\"relayState\":";
        jsonRelay += (motorState ? "true" : "false"); // Kirim sebagai string "true"/"false"
        jsonRelay += "}";
        client->text(jsonRelay);
        client->text(i2cScanResultsJson); // Kirim hasil I2C scan
    }
    else if(type == WS_EVT_DISCONNECT){
        Serial.printf("WebSocket client #%u disconnected\n", client->id());
    }
    else if(type == WS_EVT_DATA){
        String msg = (char*)data;
        Serial.println("Pesan dari client: " + msg);
        if(msg == "toggleRelay"){
            motorState = !motorState;
            digitalWrite(motorPin, motorState ? LOW : HIGH);
            Serial.printf("Motor diubah ke: %s\n", motorState ? "ON" : "OFF");
            String json = "{\"relayState\":";
            json += (motorState ? "true" : "false");
            json += "}";
            ws.textAll(json);
            // Update LCD status motor (jika tidak ada kartu RFID terbaca, ini akan tampil)
            if (currentRfidUid == "Belum Terbaca") { // Cek status RFID dari komponen (variabel global)
                lcd.setCursor(0, 3);
                lcd.print("Motor: ");
                lcd.print(motorState ? "ON " : "OFF");
                lcd.print("             "); // Clear sisa baris
            }
        }
    }
}

// --- Bagian 10: Fungsi I2C Scanner ---
std::vector<String> i2cScanner() {
  Serial.println("\n--- Memulai I2C Scanner ---");
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Scanning I2C...");

  byte error, address;
  int nDevices = 0;
  std::vector<String> foundAddresses;

  for(address = 1; address < 127; address++ ) {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();

    if (error == 0) {
      String addrStr = "0x";
      if (address < 16) addrStr += "0";
      addrStr += String(address, HEX);

      foundAddresses.push_back(addrStr);

      Serial.print("I2C device found at address ");
      Serial.println(addrStr);

      lcd.setCursor(nDevices % 2 == 0 ? 0 : 10, (nDevices / 2) + 1);
      lcd.print(addrStr);
      nDevices++;
      if (nDevices >= 4) { // Pindah ke baris baru setelah 4 alamat
          delay(1000);
          lcd.clear();
          lcd.setCursor(0,0);
          lcd.print("Scanning I2C (cont)...");
          nDevices = 0;
      }
    }
  }

  if (foundAddresses.empty()) {
    Serial.println("\nTidak ada perangkat I2C ditemukan.");
    lcd.setCursor(0, 1);
    lcd.print("No I2C Devices Found!");
  } else {
    Serial.println("\nScan selesai. " + String(foundAddresses.size()) + " perangkat I2C ditemukan.");
    lcd.setCursor(0, 0);
    lcd.print("I2C Found: ");
    lcd.print(foundAddresses.size());
    lcd.setCursor(0, 1);
    lcd.print("                    "); // Clear the line
    delay(2000);
  }
  Serial.println("\n--- I2C Scanner Selesai ---");
  delay(1000);

  return foundAddresses;
}


// --- Bagian 11: Fungsi Setup (Inisialisasi) ---
void setup() {
    Serial.begin(115200);

    Wire.begin(); // Inisialisasi komunikasi I2C
    lcd.init();
    lcd.backlight();

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Mesin Kopi Smart");
    lcd.setCursor(0, 1);
    lcd.print("Booting...");
    delay(1000);

    // Panggil fungsi I2C Scanner di awal startup
    std::vector<String> addresses = i2cScanner();

    // Buat string JSON dari hasil scan I2C untuk WebSocket
    i2cScanResultsJson = "{\"type\":\"i2cScan\",\"addresses\":[";
    for (size_t i = 0; i < addresses.size(); ++i) {
        i2cScanResultsJson += "\"" + addresses[i] + "\"";
        if (i < addresses.size() - 1) {
            i2cScanResultsJson += ",";
        }
    }
    i2cScanResultsJson += "]}";
    Serial.println("I2C Scan JSON untuk Web: " + i2cScanResultsJson);


    Serial.println("Melanjutkan setup setelah I2C scan...");
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Setup Devices...");

    // Inisialisasi PCF8574 pertama (jika digunakan)
    if (!pcf1.begin(PCF8574_ADDRESS_1, &Wire)) {
      Serial.println("PCF8574 (0x20) tidak ditemukan atau gagal inisialisasi.");
      lcd.setCursor(0, 3);
      lcd.print("PCF1 init FAILED!");
    } else {
      Serial.println("PCF8574 (0x20) OK!");
    }

    // --- Inisialisasi Komponen ---
    setupFrontPanel(PCF8574_ADDRESS_2);
    Serial.println("Front Panel (PCF8574 0x21) OK!");

    setupRfidCardReader();
    Serial.println("RFID Reader dari komponen OK!");

    storage_detector_init_all_sensors();

    pinMode(motorPin, OUTPUT);
    digitalWrite(motorPin, HIGH);

    if(!SPIFFS.begin(true)){
        Serial.println("Gagal mount SPIFFS");
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("ERROR: SPIFFS Fail!");
        while(true);
    }

    WiFi.begin(ssid, password);
    Serial.print("Menghubungkan ke WiFi...");
    lcd.setCursor(0, 0);
    lcd.print("Connecting to WiFi...");
    lcd.setCursor(0, 1);

    int wifi_dot_count = 0;
    while(WiFi.status() != WL_CONNECTED){
        delay(500);
        Serial.print(".");
        lcd.print(".");
        wifi_dot_count++;
        if (wifi_dot_count > 10) {
            lcd.setCursor(0, 1);
            lcd.print("          ");
            lcd.setCursor(0, 1);
            wifi_dot_count = 0;
        }
    }
    Serial.println();
    Serial.print("Terhubung ke WiFi, IP: ");
    Serial.println(WiFi.localIP());

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("WiFi Connected!");
    lcd.setCursor(0, 1);
    lcd.print("IP: ");
    lcd.print(WiFi.localIP());

    ws.onEvent(onWsEvent);
    server.addHandler(&ws);

    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(SPIFFS, "/index.html", "text/html");
    });

    server.begin();
    Serial.println("Server web dimulai.");
    lcd.setCursor(0, 2);
    lcd.print("Server Started!");
    // Status motor awal (dan RFID) akan diupdate di loop
    lcd.setCursor(0, 3);
    lcd.print("Motor: "); // Default display for line 3
    lcd.print(motorState ? "ON " : "OFF");
}

// --- Bagian 12: Fungsi Loop (Eksekusi Berulang) ---
void loop() {
    ws.cleanupClients();

    unsigned long currentMillis = millis(); // Ambil waktu saat ini sekali per loop

    // --- Panggil Fungsi Handle dari Komponen Front Panel ---
    handleFrontPanel();

    // --- Panggil Fungsi Handle dari Komponen RFID Card Reader ---
    handleRfidCardReader(currentMillis);


    // --- Pembacaan Sensor Jarak dan Pengiriman Data ke Web ---
    if(currentMillis - lastSensorReadMillis >= sensorReadInterval){
        lastSensorReadMillis = currentMillis;

        long distance1 = storage_detector_get_distance(SD_TRIG_PIN_1, SD_ECHO_PIN_1);
        long distance2 = storage_detector_get_distance(SD_TRIG_PIN_2, SD_ECHO_PIN_2);
        long distance3 = storage_detector_get_distance(SD_TRIG_PIN_3, SD_ECHO_PIN_3);

        if(distance1 == -1 || distance1 <= 0) distance1 = 0;
        if(distance2 == -1 || distance2 <= 0) distance2 = 0;
        if(distance3 == -1 || distance3 <= 0) distance3 = 0;

        // --- Update LCD dengan Data Sensor ---
        lcd.setCursor(0, 1); // Baris 1
        lcd.print("D1:"); lcd.print(distance1); lcd.print("cm ");
        lcd.setCursor(10, 1); // Pindah ke tengah baris 1
        lcd.print("D2:"); lcd.print(distance2); lcd.print("cm ");

        lcd.setCursor(0, 2); // Baris 2
        lcd.print("D3:"); lcd.print(distance3); lcd.print("cm ");

        // --- Update LCD untuk Baris 3 (RFID vs Motor) ---
        lcd.setCursor(0, 3);
        lcd.print("                    "); // Clear line 3 first

        // Prioritaskan tampilan RFID jika ada kartu yang terbaca
        // Mengakses currentRfidUid langsung
        if (currentRfidUid != "Belum Terbaca") {
            lcd.setCursor(0, 3);
            lcd.print("RFID: ");
            lcd.print(currentRfidUid);
        } else {
            // Jika tidak ada RFID, tampilkan status motor
            lcd.setCursor(0, 3);
            lcd.print("Motor: ");
            lcd.print(motorState ? "ON " : "OFF");
        }


        // --- Mengirim Data ke Klien WebSocket ---
        String json = "{\"distance1\":";
        json += distance1;
        json += ",\"distance2\":";
        json += distance2;
        json += ",\"distance3\":";
        json += distance3;
        json += ",\"relayState\":";
        json += motorState ? "true" : "false";

        // --- Tambahkan status push button ---
        // currentButtonState[0] adalah status stabil dari PB1
        // currentButtonState[1] adalah status stabil dari PB2
        // currentButtonState[2] adalah status stabil dari PB3
        // currentButtonState[3] adalah status stabil dari PB4
        // Mengakses currentButtonState langsung
        json += ",\"pb1State\":\""; json += (currentButtonState[0] == HIGH ? "HIGH" : "LOW"); json += "\"";
        json += ",\"pb2State\":\""; json += (currentButtonState[1] == HIGH ? "HIGH" : "LOW"); json += "\"";
        json += ",\"pb3State\":\""; json += (currentButtonState[2] == HIGH ? "HIGH" : "LOW"); json += "\"";
        json += ",\"pb4State\":\""; json += (currentButtonState[3] == HIGH ? "HIGH" : "LOW"); json += "\"";

        // --- Tambahkan status RFID UID dari komponen ---
        // Mengakses currentRfidUid langsung
        json += ",\"rfidUid\":\"";
        json += currentRfidUid;
        json += "\"";

        json += "}";

        ws.textAll(json);
        Serial.println("Data terkirim: " + json); // <<<--- DEBUGGING: Ini akan menampilkan semua JSON yang terkirim
    }
}
