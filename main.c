#include <Arduino.h>
#include <TinyGPS++.h>
#include <HardwareSerial.h>

// 1. Inicializace objektu pro parsování GPS dat
TinyGPSPlus gps;

// 2. Nastavení hardwarového UART2 pro GPS
// ESP32 má 3 hardwarové UARTy. UART0 je pro USB, UART2 použijeme pro GPS.
HardwareSerial GPS_Serial(2);

// Piny podle našeho README
const int RX_PIN = 16;
const int TX_PIN = 17;

// Většina modulů ATGM336H má z výroby nastavenou rychlost 9600 baudů
const uint32_t GPS_BAUD = 9600; 

void setup() {
  // Komunikace s počítačem (Sériový monitor) pro výpis výsledků
  Serial.begin(115200);

  // Spuštění komunikace s GPS modulem
  // SERIAL_8N1 znamená 8 datových bitů, bez parity, 1 stop bit (standard)
  GPS_Serial.begin(GPS_BAUD, SERIAL_8N1, RX_PIN, TX_PIN);

  Serial.println("RaceBox - Test GPS inicializován...");
  Serial.println("Čekám na satelity (venku to trvá cca 1-2 minuty)...");
}

// Zde je tvá jedna funkce čistě pro obsluhu GPS
void cti_data_z_gps() {
  // Pokud GPS modul posílá nějaká data na piny RX/TX, přečteme je
  while (GPS_Serial.available() > 0) {
    char c = GPS_Serial.read();
    
    // Znak po znaku "krmíme" knihovnu TinyGPSPlus
    if (gps.encode(c)) {
      
      // Pokud se poskládala celá validní věta a máme fix (vidíme satelity)
      if (gps.location.isUpdated()) {
        
        Serial.print("Satelity: ");
        Serial.print(gps.satellites.value());
        
        Serial.print(" | Rychlost: ");
        Serial.print(gps.speed.kmph());
        Serial.print(" km/h");
        
        Serial.print(" | Poloha: ");
        Serial.print(gps.location.lat(), 6);
        Serial.print(", ");
        Serial.println(gps.location.lng(), 6);
      }
    }
  }
}

void loop() {
  // Neustále točíme čtecí funkci. 
  // UART2 má vlastní hardwarový buffer, takže se data neztratí.
  cti_data_z_gps();
}