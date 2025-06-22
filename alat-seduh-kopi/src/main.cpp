#include <WiFi.h>
#include <SPIFFS.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>

// --- Include untuk modul RFID Anda ---
#include "components/card_reader/card_reader.h"

// --- Baris untuk Storage Detector dan Temperature & Humidity dihapus sementara ---
// #include "components/storage_detector/storage_detector.h"
// #include "components/temperature_humidity/temperature_humidity.h"

const char* ssid = "Pikan Miku";
const char* password = "jasamaru123";

const int motorPin = 4;
bool motorState = LOW;

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

unsigned long lastSensorReadMillis = 0;
const long sensorReadInterval = 2000;

void onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client,
                AwsEventType type, void *arg, uint8_t *data, size_t len) {
    if(type == WS_EVT_CONNECT){
        Serial.printf("WebSocket client #%u connected\n", client->id());
        String json = "{\"relayState\":";
        json += (motorState ? "true" : "false"); // Kirim boolean sebagai string "true"/"false"
        json += "}";
        client->text(json);
    } else if(type == WS_EVT_DISCONNECT){
        Serial.printf("WebSocket client #%u disconnected\n", client->id());
    } else if(type == WS_EVT_DATA){
        String msg = (char*)data;
        Serial.println("Pesan dari client: " + msg);
        if(msg == "toggleRelay"){
            motorState = !motorState;

            // Pastikan ini sesuai dengan logika relay Anda (HIGH/LOW untuk ON/OFF)
            // Jika relay Anda aktif LOW, maka HIGH akan mematikannya
            digitalWrite(motorPin, motorState ? LOW : HIGH); // Contoh: LOW = ON, HIGH = OFF

            Serial.printf("Motor diubah ke: %s\n", motorState ? "ON" : "OFF");

            String json = "{\"relayState\":";
            json += (motorState ? "true" : "false"); // Kirim boolean sebagai string "true"/"false"
            json += "}";
            ws.textAll(json);
        }
    }
}

void setup() {
    Serial.begin(115200);
    Serial.println("ESP32 booting...");

    // --- Inisialisasi komponen yang ingin tetap aktif ---
    // storage_detector_init_all_sensors(); // Dihapus sementara
    // temperature_humidity_init();       // Dihapus sementara
    card_reader_init(); // Inisialisasi modul RFID

    pinMode(motorPin, OUTPUT);
    digitalWrite(motorPin, HIGH); // Pastikan kondisi awal relay OFF

    if(!SPIFFS.begin(true)){
        Serial.println("Gagal mount SPIFFS");
        return;
    }

    WiFi.begin(ssid, password);
    Serial.print("Menghubungkan ke WiFi...");
    while(WiFi.status() != WL_CONNECTED){
        delay(500);
        Serial.print(".");
    }
    Serial.println();
    Serial.print("Terhubung ke WiFi, IP: ");
    Serial.println(WiFi.localIP());

    ws.onEvent(onWsEvent);
    server.addHandler(&ws);
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(SPIFFS, "/index.html", "text/html");
    });
    server.begin();
    Serial.println("Web Server dimulai.");
}

void loop() {
    ws.cleanupClients();

    unsigned long currentMillis = millis();

    if(currentMillis - lastSensorReadMillis >= sensorReadInterval){
        lastSensorReadMillis = currentMillis;

        // --- Baris untuk pembacaan Ultrasonik dan DHT22 dihapus sementara ---
        // long distance1 = storage_detector_get_distance(SD_TRIG_PIN_1, SD_ECHO_PIN_1);
        // long distance2 = storage_detector_get_distance(SD_TRIG_PIN_2, SD_ECHO_PIN_2);
        // long distance3 = storage_detector_get_distance(SD_TRIG_PIN_3, SD_ECHO_PIN_3);
        // float temperature = temperature_humidity_read_temperature();
        // float humidity = temperature_humidity_read_humidity();

        // --- Pembacaan RFID ---
        String rfid_uid = "N/A";
        if (card_reader_is_new_card_present()) {
            rfid_uid = card_reader_read_card_uid();
            Serial.print("Kartu RFID Terdeteksi! UID: ");
            Serial.println(rfid_uid);
            card_reader_halt();
        }

        // --- Penanganan nilai error Ultrasonik dan DHT22 dihapus sementara ---
        // if(distance1 == -1 || distance1 <= 0) distance1 = 0;
        // if(distance2 == -1 || distance2 <= 0) distance2 = 0;
        // if(distance3 == -1 || distance3 <= 0) distance3 = 0;
        // if(temperature == -999.0) temperature = 0;
        // if(humidity == -999.0) humidity = 0;

        String json = "{";
        // json += "\"distance1\":" + String(distance1) + ","; // Dihapus sementara
        // json += "\"distance2\":" + String(distance2) + ","; // Dihapus sementara
        // json += "\"distance3\":" + String(distance3) + ","; // Dihapus sementara
        // json += "\"temperature\":" + String(temperature) + ","; // Dihapus sementara
        // json += "\"humidity\":" + String(humidity) + ",";       // Dihapus sementara

        json += "\"rfid_uid\":\"" + rfid_uid + "\","; // UID RFID tetap ada
        json += "\"relayState\":" + String(motorState ? "true" : "false"); // Relay State tetap ada
        json += "}";

        ws.textAll(json);

        Serial.println("Data terkirim: " + json);
    }
}