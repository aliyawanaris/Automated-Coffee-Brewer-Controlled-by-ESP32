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

Asumsi:
- Ada file 'index.html' yang tersimpan di SPIFFS.
- Library 'storage_detector' berfungsi untuk membaca sensor jarak.
- 'motorPin' (GPIO 4) terhubung ke relay yang mengontrol motor,
dengan logika aktif LOW (HIGH = OFF, LOW = ON).
- Pin-pin untuk sensor jarak (SD_TRIG_PIN_1, SD_ECHO_PIN_1, dst.)
didefinisikan di tempat lain, kemungkinan di dalam 'storage_detector.h' atau file serupa.
- LCD I2C 20x4 terhubung ke GPIO 21 (SDA) dan GPIO 22 (SCL).
*/

// --- Bagian 1: Inklusi Library ---
// Library standar Arduino untuk fungsionalitas dasar.
#include <Arduino.h>
// Library untuk konektivitas Wi-Fi pada ESP32.
#include <WiFi.h>
// Library untuk sistem file SPIFFS (untuk menyimpan file seperti index.html).
#include <SPIFFS.h>
// Library untuk server web asinkron (lebih efisien untuk menangani banyak koneksi).
#include <ESPAsyncWebServer.h>
// Library untuk komunikasi TCP asinkron (dependensi ESPAsyncWebServer).
#include <AsyncTCP.h>
// Library kustom untuk deteksi penyimpanan/sensor (asumsi sensor jarak).
// Defenisi pin seperti SD_TRIG_PIN_1 kemungkinan ada di sini.
#include "components/storage_detector/storage_detector.h"

// --- Library untuk LCD I2C ---
// Diperlukan untuk komunikasi I2C.
#include <Wire.h>
// Diperlukan untuk mengontrol LCD dengan modul I2C.
#include <LiquidCrystal_I2C.h>


// --- Bagian 2: Konfigurasi Wi-Fi ---
// SSID (nama jaringan) Wi-Fi yang akan dihubungkan.
const char* ssid = "Pikan Miku";
// Kata sandi (password) untuk jaringan Wi-Fi.
const char* password = "jasamaru123";

// --- Bagian 3: Konfigurasi Perangkat Keras & Variabel Status ---
// Pin GPIO pada ESP32 yang terhubung ke relay (untuk mengontrol motor).
// Asumsi: Relay aktif LOW (motorPin HIGH = relay OFF, motorPin LOW = relay ON).
const int motorPin = 4;
// Variabel boolean untuk menyimpan status motor (true = ON, false = OFF).
// Awalnya diatur ke OFF.
bool motorState = LOW;

// --- Bagian 4: Objek Server Web & WebSocket ---
// Membuat objek server web pada port 80 (port default untuk HTTP).
AsyncWebServer server(80);
// Membuat objek WebSocket pada jalur "/ws" (untuk komunikasi real-time).
AsyncWebSocket ws("/ws");

// --- Bagian 5: Pengaturan Interval Pembacaan Sensor ---
// Variabel untuk menyimpan waktu terakhir sensor dibaca.
unsigned long lastSensorReadMillis = 0;
// Interval (dalam milidetik) antara setiap pembacaan sensor (2000 ms = 2 detik).
const long sensorReadInterval = 2000;

// --- Bagian 6: Objek LCD I2C ---
// Alamat I2C LCD Anda. Umumnya 0x27 atau 0x3F.
// Jika tidak yakin, Anda perlu menjalankan I2C Scanner untuk menemukannya.
#define LCD_ADDRESS 0x27
// Jumlah kolom pada LCD (20 untuk 20x4).
#define LCD_COLUMNS 20
// Jumlah baris pada LCD (4 untuk 20x4).
#define LCD_ROWS    4
// Membuat objek LCD, dengan parameter alamat I2C, jumlah kolom, dan jumlah baris.
LiquidCrystal_I2C lcd(LCD_ADDRESS, LCD_COLUMNS, LCD_ROWS);


// --- Bagian 7: Fungsi Callback WebSocket ---
// Fungsi ini akan dipanggil setiap kali ada event pada WebSocket (koneksi, diskoneksi, data diterima).
void onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client,
               AwsEventType type, void *arg, uint8_t *data, size_t len) {
    // Jika klien baru terhubung
    if(type == WS_EVT_CONNECT){
        Serial.printf("WebSocket client #%u connected\n", client->id());
        // Kirim status relay saat ini ke klien yang baru terhubung.
        String json = "{\"relayState\":";
        json += motorState; // Tambahkan nilai boolean (true/false)
        json += "}";
        client->text(json); // Kirim sebagai teks JSON
    }
    // Jika klien terputus
    else if(type == WS_EVT_DISCONNECT){
        Serial.printf("WebSocket client #%u disconnected\n", client->id());
    }
    // Jika data diterima dari klien
    else if(type == WS_EVT_DATA){
        String msg = (char*)data; // Konversi data yang diterima menjadi String
        Serial.println("Pesan dari client: " + msg);

        // Jika pesan yang diterima adalah "toggleRelay"
        if(msg == "toggleRelay"){
            motorState = !motorState; // Ubah status motor (toggle ON/OFF)

            // Kontrol pin motor/relay
            // Jika motorState true (ON), kirim LOW ke motorPin (aktifkan relay)
            // Jika motorState false (OFF), kirim HIGH ke motorPin (non-aktifkan relay)
            digitalWrite(motorPin, motorState ? LOW : HIGH);

            Serial.printf("Motor diubah ke: %s\n", motorState ? "ON" : "OFF");

            // Kirim status relay yang baru ke semua klien WebSocket yang terhubung
            String json = "{\"relayState\":";
            json += motorState;
            json += "}";
            ws.textAll(json); // Kirim ke semua klien

            // --- Update LCD setelah status motor berubah dari Web ---
            lcd.setCursor(0, 3); // Baris 3, kolom 0
            lcd.print("Motor: ");
            lcd.print(motorState ? "ON " : "OFF"); // Tambah spasi untuk hapus sisa karakter jika "OFF" jadi "ON"
        }
    }
}

// --- Bagian 8: Fungsi Setup (Inisialisasi) ---
void setup() {
    // Inisialisasi komunikasi serial untuk debugging.
    Serial.begin(115200);

    // --- Inisialisasi LCD I2C ---
    Wire.begin(); // Inisialisasi komunikasi I2C
    lcd.init();   // Inisialisasi LCD
    lcd.backlight(); // Nyalakan backlight LCD

    // Tampilkan pesan awal pada LCD
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Mesin Kopi Smart");
    lcd.setCursor(0, 1);
    lcd.print("Booting...");

    // Inisialisasi semua sensor di dalam 'storage_detector'.
    // Asumsi: Ini akan menginisialisasi pin-pin sensor jarak.
    storage_detector_init_all_sensors();

    // Mengatur pin motor sebagai OUTPUT.
    pinMode(motorPin, OUTPUT);
    // Memastikan relay non-aktif di awal (aktif LOW).
    digitalWrite(motorPin, HIGH);

    // Inisialisasi SPIFFS (Sistem File SPI Flash).
    // Parameter 'true' akan memformat SPIFFS jika gagal di-mount.
    if(!SPIFFS.begin(true)){
        Serial.println("Gagal mount SPIFFS");
        // Tampilkan error di LCD juga
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("ERROR: SPIFFS Fail!");
        while(true); // Loop tak terbatas jika SPIFFS gagal
    }

    // Menghubungkan ESP32 ke jaringan Wi-Fi.
    WiFi.begin(ssid, password);
    Serial.print("Menghubungkan ke WiFi...");
    // Tunggu hingga Wi-Fi terhubung.
    while(WiFi.status() != WL_CONNECTED){
        delay(500); // Jeda sebentar sebelum mencoba lagi
        Serial.print(".");
        // Update LCD saat mencoba koneksi
        lcd.setCursor(0, 2);
        lcd.print("Connecting to WiFi...");
    }
    Serial.println(); // Baris baru
    Serial.print("Terhubung ke WiFi, IP: ");
    Serial.println(WiFi.localIP()); // Cetak alamat IP yang didapat.

    // Setelah terhubung, tampilkan IP di LCD
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("WiFi Connected!");
    lcd.setCursor(0, 1);
    lcd.print("IP: ");
    lcd.print(WiFi.localIP());


    // Menetapkan fungsi 'onWsEvent' sebagai handler untuk semua event WebSocket.
    ws.onEvent(onWsEvent);
    // Menambahkan handler WebSocket ke server web.
    server.addHandler(&ws);

    // Menangani permintaan HTTP GET untuk halaman root ("/")
    // Akan mengirim file "index.html" dari SPIFFS sebagai halaman web.
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(SPIFFS, "/index.html", "text/html");
    });

    // Memulai server web.
    server.begin();
    Serial.println("Server web dimulai.");
    lcd.setCursor(0, 2);
    lcd.print("Server Started!");
    lcd.setCursor(0, 3);
    lcd.print("Motor: ");
    lcd.print(motorState ? "ON " : "OFF"); // Tampilkan status motor awal
}

// --- Bagian 9: Fungsi Loop (Eksekusi Berulang) ---
void loop() {
    // Membersihkan klien WebSocket yang tidak aktif/terputus.
    ws.cleanupClients();

    // Dapatkan waktu saat ini.
    unsigned long currentMillis = millis();

    // Periksa apakah sudah waktunya untuk membaca sensor lagi.
    if(currentMillis - lastSensorReadMillis >= sensorReadInterval){
        lastSensorReadMillis = currentMillis; // Perbarui waktu pembacaan terakhir

        // --- Membaca Data Sensor Jarak ---
        // Mendapatkan jarak dari sensor pertama.
        long distance1 = storage_detector_get_distance(SD_TRIG_PIN_1, SD_ECHO_PIN_1);
        // Mendapatkan jarak dari sensor kedua.
        long distance2 = storage_detector_get_distance(SD_TRIG_PIN_2, SD_ECHO_PIN_2);
        // Mendapatkan jarak dari sensor ketiga.
        long distance3 = storage_detector_get_distance(SD_TRIG_PIN_3, SD_ECHO_PIN_3);

        // Menangani nilai yang tidak valid atau nol dari sensor (misal -1 atau <=0 dianggap 0).
        // Ini membantu memastikan data yang dikirim valid.
        if(distance1 == -1 || distance1 <= 0) distance1 = 0;
        if(distance2 == -1 || distance2 <= 0) distance2 = 0;
        if(distance3 == -1 || distance3 <= 0) distance3 = 0;

        // --- Update LCD dengan Data Sensor ---
        lcd.setCursor(0, 1); // Baris 1
        lcd.print("D1:"); lcd.print(distance1); lcd.print("cm "); // Jarak sensor 1
        lcd.setCursor(10, 1); // Pindah ke tengah baris 1
        lcd.print("D2:"); lcd.print(distance2); lcd.print("cm "); // Jarak sensor 2

        lcd.setCursor(0, 2); // Baris 2
        lcd.print("D3:"); lcd.print(distance3); lcd.print("cm "); // Jarak sensor 3
        // Jika ada sensor suhu, bisa ditambahkan di sini, contoh:
        // lcd.print("T:"); lcd.print(temperature); lcd.print("C");


        // --- Mengirim Data ke Klien WebSocket ---
        // Membangun string JSON yang berisi semua data sensor dan status relay.
        String json = "{\"distance1\":";
        json += distance1;
        json += ",\"distance2\":";
        json += distance2;
        json += ",\"distance3\":";
        json += distance3;
        json += ",\"relayState\":";
        json += motorState; // Tambahkan status relay saat ini
        // Jika ada sensor suhu, bisa ditambahkan di sini, contoh:
        // json += ",\"temperature\":"; json += temperature;
        json += "}";

        // Kirim string JSON ini ke semua klien WebSocket yang terhubung.
        ws.textAll(json);

        // Cetak JSON yang dikirim ke Serial Monitor untuk debugging.
        Serial.println("Data terkirim: " + json);
    }
}