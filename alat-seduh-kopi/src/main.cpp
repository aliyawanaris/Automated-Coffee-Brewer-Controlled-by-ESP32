// --- Bagian 1: Inklusi Library & Komponen ---
#include <Arduino.h>
#include <WiFi.h>
#include <SPIFFS.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include <vector>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_PCF8574.h>

// --- Inklusi Komponen Lokal ---
#include "components/lcd_display/lcd_display.h"
#include "components/storage_detector/storage_detector.h"
#include "components/order_coffee/order_coffee.h"
#include "components/rfid_card_reader/rfid_card_reader.h"
#include "components/temperature_humidity/temperature_humidity.h"
#include "components/motor_control/motor_control.h"

// --- Bagian 2: Konfigurasi Wi-Fi ---
const char* ssid = "Coffee WD";
const char* password = "coffeewd123";

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
#define PCF8574_MOTOR_CONTROL_ADDRESS 0x20// Alamat I2C PCF8574 pertama
#define PCF8574_FRONT_PANEL_ADDRESS 0x21 // Alamat I2C PCF8574 kedua
Adafruit_PCF8574 pcf1; // Deklarasi objek PCF8574

// --- Bagian 8: Global Variable Untuk Menyimpan Hasil I2C Scan ---
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

            Serial.print("I2C device found at address: ");
            Serial.println(addrStr);

            // Tampilkan 2 alamat per baris, 2 baris total
            lcd.setCursor(nDevices % 2 == 0 ? 0 : 10, (nDevices / 2) + 1);
            lcd.print(addrStr);
            nDevices++;
            if (nDevices >= 4) {
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
        delay(2000);
    }

    Serial.println("\n--- I2C Scanner Selesai ---");
    delay(1000);

    return foundAddresses;
}

// --- Bagian 11: Fungsi Setup (Inisialisasi) ---
void setup() {
    Serial.begin(115200); // Mengatur baud rate Serial Monitor

    // --- Log Pembuka Setup ---
    Serial.println("====================================================");
    Serial.println("[SETUP START] Memulai Inisialisasi Sistem Kopi");
    Serial.println("====================================================");

    // --- [1] Inisialisasi I2C Bus & Perangkat ---
    Serial.println("\n--- [1] Inisialisasi I2C Bus & Perangkat ---");
    Wire.begin(21, 22);
    Serial.println("Inisialisasi I2C Bus...");

    // Setup LCD Display
    setupLCD();
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
        Serial.println("I2C Scan: Tidak ada perangkat I2C ditemukan.");
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
    Serial.println("Melanjutkan setup setelah I2C scan...");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Setup Devices...");

    // --- [2] Inisialisasi Komponen Hardware ---
    Serial.println("\n--- [2] Inisialisasi Komponen Hardware ---");

    Serial.println("\n--- [2.1] Modul PCF8574 ---");
    // Inisialisasi Motor Control (PCF8574 0x20)
    Serial.println("Menginisialisasi PCF8574 (0x20 - Motor Control)...");
    setupMotorControl(PCF8574_MOTOR_CONTROL_ADDRESS);
    Serial.println("PCF8574 (Motor Control) OK!");
    Serial.println("Motor Control pins configured (termasuk LM298N ENA/ENB & semua pompa).");
    Serial.println("Motor Control (PCF8574 0x20) siap digunakan.");

    // Inisialisasi Order Coffee Front Panel (PCF8574 0x21)
    Serial.println("\nMenginisialisasi PCF8574 (0x21 - Order Coffee Front Panel)...");
    setupOrderCoffee(PCF8574_FRONT_PANEL_ADDRESS);
    Serial.println("PCF8574 (Order Coffee Front Panel) OK!");
    Serial.println("Order Coffee Front Panel pins configured.");
    Serial.println("Order Coffee (PCF8574 0x21) siap digunakan.");

    Serial.println("\n--- [2.2] Modul Komunikasi & Sensor ---");
    // Inisialisasi Bus SPI (untuk RFID)
    Serial.println("Inisialisasi SPI Bus...");
    SPI.begin();

    // Inisialisasi Modul RFID RC522
    setupRfidCardReader();
    Serial.println("Inisialisasi RFID RC522 selesai.");
    Serial.println("RFID Reader terintegrasi OK!");

    // Inisialisasi Sensor DHT22
    setupTemperatureHumidity();
    Serial.println("DHT22 Sensor diinisialisasi.");
    Serial.println("DHT22 Sensor terintegrasi OK!");

    // Inisialisasi semua sensor jarak (Storage Detectors)
    storage_detector_init_all_sensors();
    Serial.println("Semua Sensor Detektor Penyimpanan berhasil diinisialisasi.");

    Serial.println("\n--- [2.3] Sistem File (SPIFFS) ---");
    // Inisialisasi SPIFFS untuk melayani file web
    if (!SPIFFS.begin(true)) {
        Serial.println("Gagal mount SPIFFS. Pastikan sudah di-upload!");
        lcd.setCursor(0, 0);
        lcd.print("ERROR: SPIFFS Fail!");
        while (true);
    }
    Serial.println("SPIFFS berhasil dimount.");

    // --- [3] Konektivitas Jaringan ---
    Serial.println("\n--- [3] Konektivitas Jaringan ---");
    Serial.println("Inisialisasi WiFi...");
    Serial.println("Menghubungkan ke WiFi.....");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Connecting to WiFi ");
    lcd.setCursor(0, 1);

    // Koneksi WiFi dengan Timeout
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);

    unsigned long connectionAttemptStart = millis();
    const long CONNECTION_TIMEOUT_MS = 30000; // Timeout 30 detik

    int dot_count = 0;
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
        lcd.print(".");
        dot_count++;
        if (dot_count >= 10) {
            lcd.setCursor(0, 1);
            lcd.print("          ");
            lcd.setCursor(0, 1);
            dot_count = 0;
        }

        if (millis() - connectionAttemptStart > CONNECTION_TIMEOUT_MS) {
            Serial.println("\nKoneksi WiFi Timeout!");
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("WiFi FAILED!");
            lcd.setCursor(0, 1);
            lcd.print("Check SSID/Pass!");
            while (true);
        }
    }
    Serial.println();

    // Koneksi WiFi Berhasil
    Serial.println("WiFi Berhasil Terhubung!");
    Serial.print("  IP Address: ");
    Serial.println(WiFi.localIP());
    Serial.print("  Kekuatan Sinyal (RSSI): ");
    Serial.print(WiFi.RSSI());
    Serial.println(" dBm");

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("WiFi Connected!");
    lcd.setCursor(0, 1);
    lcd.print("IP: ");
    lcd.print(WiFi.localIP());

    // --- Konfigurasi WebSocket dan Server Web ---
    ws.onEvent(onWsEvent);
    server.addHandler(&ws);

    // Handler untuk melayani file index.html dari SPIFFS
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(SPIFFS, "/index.html", "text/html");
    });

    // Mulai server web dan WebSocket
    server.begin();
    Serial.println("Server web dimulai.");
    lcd.setCursor(0, 2);
    lcd.print("Server Started!");

    // --- Log Penutup Setup ---
    delay(2000);
    Serial.println("\n====================================================");
    Serial.println("[SETUP SELESAI] Sistem siap digunakan.");
    Serial.println("Silakan akses server web di: " + WiFi.localIP().toString());
    Serial.println("====================================================");

    // Set tampilan awal LCD ke idle menu
    displayIdleMenu();
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
        if (!menuActive && !menuConfirmed) { // <<< Variabel ini sekarang extern dari order_coffee.h
            if(currentMillis - lastSensorReadMillis >= sensorReadInterval){
                lastSensorReadMillis = currentMillis;

                long distance1 = storage_detector_get_distance(SD_TRIG_PIN_1, SD_ECHO_PIN_1);
                long distance2 = storage_detector_get_distance(SD_TRIG_PIN_2, SD_ECHO_PIN_2);
                long distance3 = storage_detector_get_distance(SD_TRIG_PIN_3, SD_ECHO_PIN_3);

                if(distance1 == -1) distance1 = 0;
                if(distance2 == -1) distance2 = 0;
                if(distance3 == -1) distance3 = 0;

                 // --- Update LCD untuk Baris 1 (Rotasi Data Sensor) ---
                static unsigned long lastSensorDisplayRotateMillis = 0;
                const long SENSOR_DISPLAY_ROTATE_INTERVAL = 3000; // Rotasi setiap 3 detik
                static int sensorDisplayMode = 0; // 0=Jarak, 1=DHT, 2=RFID/Motor

                if (currentMillis - lastSensorDisplayRotateMillis >= SENSOR_DISPLAY_ROTATE_INTERVAL) {
                    lastSensorDisplayRotateMillis = currentMillis;
                    sensorDisplayMode = (sensorDisplayMode + 1) % 3; // Rotasi antara 0, 1, 2
                }

                // --- Mengirim Data ke Klien WebSocket ---
                String json = "{\"distance1\":";
                json += distance1;
                json += ",\"distance2\":";
                json += distance2;
                json += ",\"distance3\":";
                json += distance3;

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
            if(currentMillis - lastSensorReadMillis >= sensorReadInterval){
                lastSensorReadMillis = currentMillis;

                long distance1 = storage_detector_get_distance(SD_TRIG_PIN_1, SD_ECHO_PIN_1);
                long distance2 = storage_detector_get_distance(SD_TRIG_PIN_2, SD_ECHO_PIN_2);
                long distance3 = storage_detector_get_distance(SD_TRIG_PIN_3, SD_ECHO_PIN_3);

                if(distance1 == -1) distance1 = 0;
                if(distance2 == -1) distance2 = 0;
                if(distance3 == -1) distance3 = 0;

                String json = "{\"distance1\":";
                json += distance1;
                json += ",\"distance2\":";
                json += distance2;
                json += ",\"distance3\":";
                json += distance3;

                json += ",\"rfidUid\":\"";
                json += currentRfidUid;
                json += "\"";

                json += ",\"temperature\":";
                json += String(currentTemperature, 1);
                json += ",\"humidity\":";
                json += String(currentHumidity, 0);

                json += "}";
                ws.textAll(json);
            }
        }
}