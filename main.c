#include <Arduino.h>
#include <TinyGPS++.h>
#include <HardwareSerial.h>

// 1. Inicializace objektu pro parsování GPS dat
TinyGPSPlus gps;

// 2. Nastavení hardwarového UART2 pro GPS (piny podle návrhu)
HardwareSerial GPS_Serial(2);
const int RX_PIN = 16;
const int TX_PIN = 17;
const uint32_t GPS_BAUD = 9600; // Výchozí rychlost modulu ATGM336H

// 3. Stavy a proměnné měřicího automatu
enum StavMereni {
  CEKA_NA_START,
  PROBIHA_MERENI,
  DOKONCENO
};

StavMereni stav = CEKA_NA_START;
unsigned long cas_startu = 0;
float vysledny_cas = 0.0;

// Logika měření akcelerace
void zmer_zrychleni(float aktualni_rychlost) {
  switch (stav) {
    
    case CEKA_NA_START:
      // Jakmile stojící auto překročí 1 km/h, spustí se stopky
      if (aktualni_rychlost > 1.0) {
        cas_startu = millis();
        stav = PROBIHA_MERENI;
        Serial.println("\n==========================================");
        Serial.println(">>> START! Měření 0–100 km/h zahájeno <<<");
        Serial.println("==========================================");
      }
      break;

    case PROBIHA_MERENI:
      // Průběžný výpis stavu během akcelerace
      Serial.print("Měřím... Rychlost: ");
      Serial.print(aktualni_rychlost, 1);
      Serial.print(" km/h | Čas: ");
      Serial.print((millis() - cas_startu) / 1000.0, 2);
      Serial.println(" s");

      // Detekce protnutí cílové rychlosti 100 km/h
      if (aktualni_rychlost >= 100.0) {
        unsigned long cas_cile = millis();
        vysledny_cas = (cas_cile - cas_startu) / 1000.0;
        stav = DOKONCENO;

        Serial.println("\n******************************************");
        Serial.print(">>> CÍL! 0–100 km/h za: ");
        Serial.print(vysledny_cas, 2);
        Serial.println(" s <<<");
        Serial.println("******************************************\n");
      } 
      // Ochrana: pokud auto zastaví dřív, než dosáhne 100 km/h
      else if (aktualni_rychlost < 0.5 && (millis() - cas_startu > 2000)) {
        Serial.println("\n>>> Měření přerušeno: vozidlo zastavilo před 100 km/h. <<<");
        stav = CEKA_NA_START;
      }
      break;

    case DOKONCENO:
      // Reset automatu poté, co auto po měření opět zastaví
      if (aktualni_rychlost < 1.0) {
        Serial.println("Vozidlo zastavilo. Systém je připraven na další pokus.");
        stav = CEKA_NA_START;
      }
      break;
  }
}

// Vyčítání dat ze sériové linky GPS
void cti_data_z_gps() {
  while (GPS_Serial.available() > 0) {
    char c = GPS_Serial.read();
    
    if (gps.encode(c)) {
      // Když máme nový balík dat s rychlostí, pošleme ji do měřicího automatu
      if (gps.speed.isUpdated()) {
        float aktualni_v = gps.speed.kmph();
        zmer_zrychleni(aktualni_v);
      }

      // Volitelný výpis polohy a satelitů pro kontrolu fixu
      if (gps.location.isUpdated() && stav == CEKA_NA_START) {
        Serial.print("Satelity: ");
        Serial.print(gps.satellites.value());
        Serial.print(" | Aktuální rychlost: ");
        Serial.print(gps.speed.kmph(), 1);
        Serial.println(" km/h");
      }
    }
  }
}

void setup() {
  Serial.begin(115200);
  GPS_Serial.begin(GPS_BAUD, SERIAL_8N1, RX_PIN, TX_PIN);

  Serial.println("RaceBox - Inicializace systému...");
  Serial.println("Čekám na GPS fix (pro testování je nutné být venku)...");
}

void loop() {
  cti_data_z_gps();
}