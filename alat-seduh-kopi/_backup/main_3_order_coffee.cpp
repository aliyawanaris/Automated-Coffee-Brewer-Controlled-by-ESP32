/*
Judul Proyek: Sistem Monitoring & Kontrol Mesin Kopi Berbasis Web & LCD I2C
Versi: Terintegrasi dengan RFID RC522 (RST Pin 2), Logika Menu Kopi, dan DHT22

Deskripsi:
Kode ini mengimplementasikan server web asinkron pada ESP32 untuk:
1. Menghubungkan ke jaringan Wi-Fi (dengan debugging status koneksi).
2. Melayani file web (index.html) dari SPIFFS.
3. Menggunakan WebSocket untuk komunikasi real-time dua arah.
4. Mengontrol status sebuah relay (yang terhubung ke motor atau perangkat lain)
    melalui pesan WebSocket dari klien web.
5. Membaca data dari sensor jarak (asumsi HC-SR04 atau sejenisnya)
    melalui library 'storage_detector' dan mengirimkannya ke klien web secara periodik.
6. Menampilkan status sistem, IP Address, dan data sensor pada LCD I2C.
7. Melakukan pemindaian I2C saat startup untuk mendeteksi perangkat I2C yang terhubung
    dan mengirimkan hasilnya ke klien web.
8. Mengelola input dari 4 push button dan mengontrol 2 LED pada PCF8574 kedua (0x21)
    melalui modul 'order_coffee' yang terpisah, dan menampilkan status tombol di web. <<< DIUBAH
9. Membaca UID dari modul RFID RC522 melalui modul 'rfid_card_reader' yang terpisah,
    dan menampilkannya pada Serial Monitor, LCD, dan halaman web.
10. Membaca data dari sensor DHT22 (suhu & kelembaban) melalui modul
    'temperature_humidity' dan mengirimkannya ke klien web.

Asumsi:
- Ada file 'index.html' yang tersimpan di SPIFFS.
- Library 'storage_detector' berfungsi untuk membaca sensor jarak (kode implementasi di file terpisah).
- 'motorPin' (GPIO 4) terhubung ke relay yang mengontrol motor,
dengan logika aktif LOW (HIGH = OFF, LOW = ON).
- Pin-pin untuk sensor jarak (SD_TRIG_PIN_1, SD_ECHO_PIN_1, dst.)
didefinisikan di 'components/storage_detector/storage_detector.h'.
- LCD I2C 20x4 terhubung ke GPIO 21 (SDA) dan GPIO 22 (SCL), dengan alamat 0x27.
- LED pada PCF8574 menggunakan konfigurasi Common Anode (LOW = ON, HIGH = OFF).
- Push button pada PCF8574 menggunakan koneksi pull-down eksternal (HIGH saat ditekan, LOW saat dilepas).
- RFID RC522 terhubung (SS_PIN: GPIO5, RST_PIN: GPIO2).
- DHT22 terhubung ke GPIO 17 (Data Pin).
*/

// --- Bagian 1: Inklusi Library & Komponen ---
#include <Arduino.h>
#include <WiFi.h>
#include <SPIFFS.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include <vector> // Diperlukan untuk std::vector
#include <Wire.h> // Diperlukan untuk komunikasi I2C
#include <SPI.h> // Diperlukan untuk komunikasi SPI (digunakan oleh RFID)
#include <Adafruit_PCF8574.h> // Diperlukan untuk PCF8574

// Inklusi Komponen Lokal Anda
#include "components/lcd_display/lcd_display.h"
#include "components/storage_detector/storage_detector.h"
#include "components/order_coffee/order_coffee.h" // <<< UBAH INKLUSI INI
#include "components/rfid_card_reader/rfid_card_reader.h"
#include "components/temperature_humidity/temperature_humidity.h"

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
const long sensorReadInterval = 2000; // Interval 2 detik

// --- Bagian 7: Definisi Alamat PCF8574 ---
#define PCF8574_ADDRESS_1 0x20 // Alamat I2C PCF8574 pertama (jika digunakan)
#define PCF8574_ADDRESS_2 0x21 // Alamat I2C PCF8574 kedua (untuk Front Panel/Order Coffee)
Adafruit_PCF8574 pcf1; // Deklarasi objek PCF8574 (jika digunakan)

// --- Bagian 8: GLOBAL VARIABLE UNTUK MENYIMPAN HASIL I2C SCAN ---
String i2cScanResultsJson = "{\"type\":\"i2cScan\",\"addresses\":[]}";

// --- Bagian 9: Fungsi Callback WebSocket ---
void onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client,
               AwsEventType type, void *arg, uint8_t *data, size_t len) {
    if(type == WS_EVT_CONNECT){
        Serial.printf("WebSocket client #%u connected.\n", client->id());
        String jsonRelay = "{\"relayState\":";
        jsonRelay += (motorState ? "true" : "false");
        jsonRelay += "}";
        client->text(jsonRelay);
        client->text(i2cScanResultsJson); // Kirim hasil I2C scan
    }
    else if(type == WS_EVT_DISCONNECT){
        Serial.printf("WebSocket client #%u disconnected.\n", client->id());
    }
    else if(type == WS_EVT_DATA){
        String msg = (char*)data;
        Serial.println("Pesan dari client: " + msg);
        if(msg == "toggleRelay"){
            motorState = !motorState;
            digitalWrite(motorPin, motorState ? LOW : HIGH);
            Serial.printf("Motor diubah ke: %s.\n", motorState ? "ON" : "OFF");
            String json = "{\"relayState\":";
            json += (motorState ? "true" : "false");
            json += "}";
            ws.textAll(json);
        }
    }
}

// --- Bagian 10: Fungsi I2C Scanner ---
std::vector<String> i2cScanner() {
    Serial.println("\n--- Memulai I2C Scanner ---");
    lcd.clear(); // Gunakan objek lcd yang sudah extern dari lcd_display.h
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

            Serial.print("I2C device found at address: ");
            Serial.println(addrStr);

            // Tampilkan 2 alamat per baris, 2 baris total
            lcd.setCursor(nDevices % 2 == 0 ? 0 : 10, (nDevices / 2) + 1);
            lcd.print(addrStr);
            nDevices++;
            if (nDevices >= 4) { // Pindah layar LCD setelah 4 device terdeteksi
                delay(1000);
                lcd.clear();
                lcd.setCursor(0,0);
                lcd.print("Scanning I2C (cont)...");
                nDevices = 0; // Reset counter untuk baris LCD
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
        lcd.print("                    "); // Clear baris yang mungkin ada sisa alamat
        delay(2000);
    }

    Serial.println("\n--- I2C Scanner Selesai ---");
    delay(1000);

    return foundAddresses;
}

// --- Bagian 11: Fungsi Setup (Inisialisasi) ---
void setup() {
    // --- Inisialisasi Serial Monitor
        Serial.begin(115200); // Mengatur baud rate Serial Monitor

    // --- Inisialisasi I2C Bus dan LCD ---
        Wire.begin(21, 22); // SDA di GPIO 21, SCL di GPIO 22
        Serial.println("Inisialisasi I2C Bus...");
        setupLCD(); // Panggil fungsi setupLCD dari komponen terpisah

        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Mesin Kopi Smart");
        lcd.setCursor(0, 1);
        lcd.print("Booting...");
        delay(1000);

    // --- Panggil fungsi I2C Scanner di awal startup ---
        Serial.println("Memulai I2C Scanner...");
        std::vector<String> addresses = i2cScanner();

    // --- Buat string JSON dari hasil scan I2C untuk WebSocket ---
        if (addresses.empty()) {
            i2cScanResultsJson = "{\"type\":\"i2cScan\",\"addresses\":[]}";
        } else {
            Serial.println("I2C Scan berhasil, alamat ditemukan: " + String(addresses.size()));
        }
        i2cScanResultsJson = "{\"type\":\"i2cScan\",\"addresses\":[";
        for (size_t i = 0; i < addresses.size(); ++i) {
            i2cScanResultsJson += "\"" + addresses[i] + "\"";
            if (i < addresses.size() - 1) {
                i2cScanResultsJson += ",";
            }
        }
        i2cScanResultsJson += "]}";
        Serial.println("I2C Scan JSON untuk Web: " + i2cScanResultsJson);

        // Tampilkan hasil I2C scan di LCD
        Serial.println("Melanjutkan setup setelah I2C scan...");
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Setup Devices...");

    // --- Inisialisasi PCF8574 pertama ---
        Serial.println("Inisialisasi PCF8574 (0x20)..."); // Tambahkan log untuk inisialisasi
        // (PENTING: Hanya panggil sekali di setup() utama (jika digunakan)
        if (!pcf1.begin(PCF8574_ADDRESS_1, &Wire)) {
        Serial.println("PCF8574 (0x20) tidak ditemukan atau gagal inisialisasi.");
        lcd.setCursor(0, 3);
        lcd.print("PCF1 init FAILED!");
        } else {
        Serial.println("PCF8574 (0x20) OK!");
        }

    // --- Inisialisasi Komponen Lain ---
        // Inisialisasi Order Coffee (dulunya Front Panel)
        setupOrderCoffee(PCF8574_ADDRESS_2); // <<< UBAH PANGGILAN FUNGSI INI
        Serial.println("Order Coffee (PCF8574 0x21) OK!"); // <<< UBAH LOG PESAN INI

    // --- Inisialisasi Bus SPI ---
        Serial.println("Inisialisasi SPI Bus...");
        // (PENTING: Hanya panggil sekali di setup() utama
        // untuk menghindari konflik dengan SPI lainnya seperti RFID RC522)
        SPI.begin();

    // --- Inisialisasi Modul RFID RC522 ---
        setupRfidCardReader();
        Serial.println("RFID Reader terintegrasi OK dari main.cpp!");

    // --- Inisialisasi Sensor DHT22 ---
        setupTemperatureHumidity();
        Serial.println("DHT22 Sensor terintegrasi OK dari main.cpp!");

    // --- Inisialisasi semua sensor jarak ---
        storage_detector_init_all_sensors();

    // --- Konfigurasi pin motor ---
        pinMode(motorPin, OUTPUT);
        digitalWrite(motorPin, HIGH); // Motor OFF secara default

    // --- Inisialisasi SPIFFS untuk melayani file web ---
        if(!SPIFFS.begin(true)){
            Serial.println("Gagal mount SPIFFS");
            lcd.setCursor(0, 0);
            lcd.print("ERROR: SPIFFS Fail!");
            while(true);
        }

        Serial.println("Menghubungkan ke WiFi...");
        lcd.setCursor(0, 0);
        lcd.print("Connecting to WiFi...");
        lcd.setCursor(0, 1);

    // --- Bagian Koneksi WiFi dengan Timeout ---
        WiFi.mode(WIFI_STA); // Pastikan ESP32 dalam mode Station
        WiFi.begin(ssid, password); // Mulai koneksi WiFi

        unsigned long connectionAttemptStart = millis();
        const long CONNECTION_TIMEOUT_MS = 30000; // Timeout 30 detik

        int dot_count = 0;
        while(WiFi.status() != WL_CONNECTED){
            delay(500); // Tunggu sebentar
            Serial.print(".");
            lcd.print(".");
            dot_count++;
            if (dot_count >= 10) { // Setelah 10 titik, bersihkan baris dan mulai lagi
                lcd.setCursor(0, 1);
                lcd.print("          "); // Clear 10 spaces
                lcd.setCursor(0, 1);
                dot_count = 0;
            }

            if (millis() - connectionAttemptStart > CONNECTION_TIMEOUT_MS) {
                Serial.println("\nKoneksi WiFi Timeout!");
                lcd.clear();
                lcd.setCursor(0,0);
                lcd.print("WiFi FAILED!");
                lcd.setCursor(0,1);
                lcd.print("Check SSID/Pass!");
                while(true); // Hentikan eksekusi jika WiFi gagal setelah timeout
            }
        }
        Serial.println(); // Baris baru setelah titik-titik koneksi

    // --- Bagian Koneksi WiFi Berhasil ---
        // Jika sampai sini, berarti WiFi sudah terhubung
        Serial.println("WiFi Berhasil Terhubung!");
        Serial.print("IP Address: ");
        Serial.println(WiFi.localIP());
        Serial.print("Kekuatan Sinyal (RSSI): ");
        Serial.print(WiFi.RSSI());
        Serial.println(" dBm");

        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("WiFi Connected!");
        lcd.setCursor(0, 1);
        lcd.print("IP: ");
        lcd.print(WiFi.localIP());

    // --- Konfigurasi WebSocket dan Server Web ---
    ws.onEvent(onWsEvent); // Atur fungsi callback untuk event WebSocket
    server.addHandler(&ws); // Tambahkan handler WebSocket ke server

    // --- Handler untuk melayani file index.html dari SPIFFS saat root URL diakses ---
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(SPIFFS, "/index.html", "text/html");
    });

    // --- Mulai server web dan WebSocket ---
        server.begin(); // Mulai server web
        Serial.println("Server web dimulai.");
        lcd.setCursor(0, 2);
        lcd.print("Server Started!");

    // --- Pesan LCD setelah semua setup selesai ---
        delay(2000); // Beri sedikit waktu untuk membaca status awal
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Coffee Wd");
        lcd.setCursor(0, 2);
        lcd.print("Silahkan pilih menu");
        lcd.setCursor(0, 3);
        lcd.print("kopi pilihan anda!");
        // Akhir pesan LCD
        Serial.println("Setup selesai, sistem siap digunakan.");
}

// --- Bagian 12: Fungsi Loop (Eksekusi Berulang) ---
void loop() {
    // Membersihkan koneksi WebSocket yang terputus
        ws.cleanupClients();

    // Dapatkan waktu saat ini
        unsigned long currentMillis = millis();

    // --- Panggil Fungsi Handle dari Komponen order_coffee ---
        handleOrderCoffee();

    // --- Panggil Fungsi Handle dari Komponen rfid_card_reader ---
        handleRfidCardReader(currentMillis);

    // --- Panggil Fungsi Handle dari Komponen temperature_humidity ---
        handleTemperatureHumidity(currentMillis);

    // --- Pembacaan Sensor Jarak dan Pengiriman Data ke Web ---
        // Hanya update LCD dengan data sensor jika tidak ada menu aktif atau sedang diproses
        if (!menuActive && !menuConfirmed) { // <<< Variabel ini sekarang extern dari order_coffee.h
            if(currentMillis - lastSensorReadMillis >= sensorReadInterval){
                lastSensorReadMillis = currentMillis;

                long distance1 = storage_detector_get_distance(SD_TRIG_PIN_1, SD_ECHO_PIN_1);
                long distance2 = storage_detector_get_distance(SD_TRIG_PIN_2, SD_ECHO_PIN_2);
                long distance3 = storage_detector_get_distance(SD_TRIG_PIN_3, SD_ECHO_PIN_3);

                if(distance1 == -1) distance1 = 0;
                if(distance2 == -1) distance2 = 0;
                if(distance3 == -1) distance3 = 0;

                // Pesan LCD (sudah Anda pindahkan)
                lcd.setCursor(0, 0);
                lcd.print("Coffee Wd");
                lcd.setCursor(0, 2);
                lcd.print("Silahkan pilih menu");
                lcd.setCursor(0, 3);
                lcd.print("kopi pilihan anda!");

                // --- Update LCD untuk Baris 1 (Rotasi Data Sensor) ---
                static unsigned long lastSensorDisplayRotateMillis = 0;
                const long SENSOR_DISPLAY_ROTATE_INTERVAL = 3000; // Rotasi setiap 3 detik
                static int sensorDisplayMode = 0; // 0=Jarak, 1=DHT, 2=RFID/Motor

                if (currentMillis - lastSensorDisplayRotateMillis >= SENSOR_DISPLAY_ROTATE_INTERVAL) {
                    lastSensorDisplayRotateMillis = currentMillis;
                    sensorDisplayMode = (sensorDisplayMode + 1) % 3; // Rotasi antara 0, 1, 2
                }

                lcd.setCursor(0, 1); // Baris 1 untuk data sensor
                lcd.print("                    "); // Clear baris 1
                lcd.setCursor(0, 1);

                // --- Mengirim Data ke Klien WebSocket ---
                String json = "{\"distance1\":";
                json += distance1;
                json += ",\"distance2\":";
                json += distance2;
                json += ",\"distance3\":";
                json += distance3;
                json += ",\"relayState\":";
                json += motorState ? "true" : "false";

                // --- Tambahkan status push button (currentButtonState ada di order_coffee.cpp, tapi extern di order_coffee.h) ---
                json += ",\"pb1State\":\""; json += (currentButtonState[0] == HIGH ? "HIGH" : "LOW"); json += "\"";
                json += ",\"pb2State\":\""; json += (currentButtonState[1] == HIGH ? "HIGH" : "LOW"); json += "\"";
                json += ",\"pb3State\":\""; json += (currentButtonState[2] == HIGH ? "HIGH" : "LOW"); json += "\"";
                json += ",\"pb4State\":\""; json += (currentButtonState[3] == HIGH ? "HIGH" : "LOW"); json += "\"";

                // --- Tambahkan status RFID UID ---
                json += ",\"rfidUid\":\"";
                json += currentRfidUid;
                json += "\"";

                // --- Tambahkan status Suhu dan Kelembaban ---
                json += ",\"temperature\":";
                json += String(currentTemperature, 1); // Format ke 1 desimal
                json += ",\"humidity\":";
                json += String(currentHumidity, 0); // Format ke 0 desimal

                json += "}";

                ws.textAll(json); // Kirim JSON ke semua klien WebSocket yang terhubung
                // Serial.println("Data terkirim: " + json); // Aktifkan untuk debugging
            }
        } else {
            // Jika menu aktif atau sedang diproses, jangan tampilkan data sensor di baris 1 LCD
            // LCD sudah diupdate oleh handleOrderCoffee() saat menu dipilih/dikonfirmasi
        }
}