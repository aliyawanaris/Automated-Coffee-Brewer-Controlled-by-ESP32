#ifndef RFID_CARD_READER_H
#define RFID_CARD_READER_H

#include <Arduino.h>
#include <SPI.h>       // Diperlukan untuk komunikasi SPI
#include <MFRC522.h>   // Library untuk MFRC522

// Definisi pin RFID RC522
#define SS_PIN 5  // SDA (Slave Select) pin for RC522
#define RST_PIN 26 // RST (Reset) pin for RC522

// Deklarasi global variable untuk menyimpan UID kartu yang terbaca
extern String currentRfidUid;
extern bool rfidErrorActive;
extern unsigned long lastRfidReadMillis;
// extern const long RFID_DISPLAY_DURATION_MS; // <-- Ini TIDAK perlu extern karena dia adalah const

// Deklarasi fungsi
void setupRfidCardReader();
void handleRfidCardReader(unsigned long currentMillis);
void processRfidMenuSelection(String rfidUid);

#endif // RFID_CARD_READER_H

//////