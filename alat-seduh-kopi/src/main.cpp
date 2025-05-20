#include <WiFi.h>
#include <SPIFFS.h>
#include <DHT.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>

// Wifi Credentials
const char* ssid = "Pikan Miku";
const char* password = "jasamaru123";

// Pins Ultrasonic
const int trigPin = 5;
const int echoPin = 18;

// DHT22
#define DHTPIN 15
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// Async Web Server & WebSocket on port 80
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

long getDistance() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  long duration = pulseIn(echoPin, HIGH, 30000); // 30ms timeout
  long distance = duration * 0.034 / 2;
  return distance;
}

// WebSocket event handler
void onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client,
               AwsEventType type, void *arg, uint8_t *data, size_t len) {
  if(type == WS_EVT_CONNECT){
    Serial.printf("WebSocket client #%u connected\n", client->id());
  } else if(type == WS_EVT_DISCONNECT){
    Serial.printf("WebSocket client #%u disconnected\n", client->id());
  }
  // Tidak perlu handle pesan dari client saat ini
}

void setup() {
  Serial.begin(115200);
  dht.begin();

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

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

  // Setup WebSocket
  ws.onEvent(onWsEvent);
  server.addHandler(&ws);

  // Serve index.html dari SPIFFS
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.html", "text/html");
  });

  server.begin();
}

unsigned long lastSend = 0;

void loop() {
  unsigned long now = millis();
  if(now - lastSend > 2000){
    lastSend = now;

    long distance = getDistance();
    float temperature = dht.readTemperature();

    if(isnan(temperature)) temperature = 0;
    if(distance <= 0) distance = 0;

    // Kirim data JSON ke semua client WebSocket
    String json = "{\"distance\":";
    json += distance;
    json += ",\"temperature\":";
    json += temperature;
    json += "}";

    ws.textAll(json);

    Serial.println("Data terkirim: " + json);
  }
}
