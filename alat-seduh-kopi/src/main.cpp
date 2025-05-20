#include <Arduino.h>
#include <WiFi.h>

// Ganti dengan SSID dan Password WiFi kamu
const char* ssid = "Pikan Miku";
const char* password = "jasamaru123";

// Pin HC-SR04
const int trigPin = 5;
const int echoPin = 18;

WiFiServer server(80);

void setup() {
  Serial.begin(115200);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  // Koneksi ke WiFi
  WiFi.begin(ssid, password);
  Serial.print("Menghubungkan ke WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Terhubung! IP address: ");
  Serial.println(WiFi.localIP());

  server.begin();
}

long getDistance() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH);
  long distance = duration * 0.034 / 2;
  return distance;
}

void loop() {
  WiFiClient client = server.available();
  if (client) {
    Serial.println("Client terhubung");
    while (client.connected()) {
      if (client.available()) {
        client.read(); // baca request, abaikan
        long distance = getDistance();

        String html = "<!DOCTYPE html><html><head><meta http-equiv='refresh' content='1'/>"
                      "<title>Sensor Jarak</title></head><body><h1>Jarak: " + String(distance) + " cm</h1></body></html>";
        client.println("HTTP/1.1 200 OK");
        client.println("Content-type:text/html");
        client.println();
        client.println(html);
        break;
      }
    }
    delay(1);
    client.stop();
    Serial.println("Client disconnected");
  }
}
