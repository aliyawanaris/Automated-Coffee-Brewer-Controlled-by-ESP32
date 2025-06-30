#ifndef GLOBAL_LOGGER_H
#define GLOBAL_LOGGER_H

#include <Arduino.h> // Diperlukan untuk Serial.print/println/printf

// Mendeklarasikan counter log global
extern unsigned long logEntryCounter;

// Makro untuk mencetak log dengan penomoran otomatis (untuk String/const char*)
#define MY_SERIAL_LOG(message) \
    do { \
        Serial.print(logEntryCounter++); \
        Serial.print(". "); \
        Serial.println(message); \
    } while (0)

// Makro untuk mencetak log terformat dengan penomoran otomatis (mirip Serial.printf)
#define MY_SERIAL_LOG_FMT(fmt, ...) \
    do { \
        Serial.print(logEntryCounter++); \
        Serial.print(". "); \
        Serial.printf(fmt, ##__VA_ARGS__); \
    } while (0)

#endif // GLOBAL_LOGGER_H
