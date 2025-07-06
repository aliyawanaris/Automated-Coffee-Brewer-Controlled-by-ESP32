#ifndef ORDER_COFFEE_H
#define ORDER_COFFEE_H

#include <Arduino.h>
#include <Adafruit_PCF8574.h>
#include <LiquidCrystal_I2C.h> // Untuk akses ke objek lcd

// --- Definisi Pin PCF8574 untuk Front Panel (0x21) ---
// PIN ANDA DARI SCRIPT YANG DIBERIKAN
const int FP_PB1_PIN = 0; // P0 = Push button 1 (INI SEKARANG AKAN JADI PIN DINAMO)
const int FP_PB2_PIN = 1; // P1 = Push button 2
const int FP_PB3_PIN = 2; // P2 = Push button 3
const int FP_PB4_PIN = 3; // P3 = Push button 4

const int FP_LED1_PIN = 4; // P4 = LED 1
const int FP_LED2_PIN = 5; // P5 = LED 2

// --- Definisi Pin Dinamo (baru) ---
#define DYNAMO_PIN FP_PB1_PIN // Dinamo terhubung ke pin 0 PCF8574 (P0), yang adalah FP_PB1_PIN

// --- Deklarasi Variabel Global untuk Logika Menu Kopi ---
extern int selectedMenu;
extern bool menuConfirmed; // True jika tombol konfirmasi ditekan
extern bool menuActive;    // True jika sedang dalam mode pemilihan/konfirmasi menu
extern unsigned long menuProcessStartTime;

// --- Deklarasi Variabel Global untuk Kontrol LED Blink ---
extern unsigned long lastBlinkMillis;
const long BLINK_INTERVAL = 250;
extern bool ledState;
extern bool blinkingStoppedMessagePrinted;

// --- Variabel untuk melacak scene LCD ---
enum CoffeeScene {
  SCENE_WELCOME,        // Pilihan kopi
  SCENE_CONFIRMATION,   // Konfirmasi "Seduh kopi"
  SCENE_PROCESSING      // Proses seduh sedang berlangsung
};
extern CoffeeScene currentScene; // Variabel global untuk state scene saat ini

// --- Deklarasi objek LCD dan PCF8574 sebagai extern ---
extern LiquidCrystal_I2C lcd;
extern Adafruit_PCF8574 pcf2; // Objek PCF8574 yang digunakan oleh modul ini

// --- DEKLARASI EKSTERN UNTUK currentButtonState ---
extern int currentButtonState[4];

// --- Variabel Global untuk Kontrol Dinamo (baru) ---
extern bool dynamoActive;           // Status dinamo (true=ON, false=OFF)
extern unsigned long dynamoStartTime; // Waktu dinamo mulai aktif
const unsigned long DYNAMO_DURATION = 5000; // Durasi dinamo aktif (5 detik)

// --- Prototipe Fungsi Order Coffee ---
void setupOrderCoffee(uint8_t pcf2_address);
void handleOrderCoffee();

// Fungsi helper yang tetap di dalam modul ini
bool readPushButton(int buttonPin, int buttonIndex);
void startBlinkingLEDs();
void stopBlinkingLEDs();
void updateBlinkingLEDs(unsigned long currentMillis);

#endif // ORDER_COFFEE_H