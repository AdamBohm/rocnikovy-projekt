#include <Arduino.h>
#include <TinyGPS++.h>
#include <HardwareSerial.h>
#include <WiFi.h>
#include <WebServer.h>

// ==========================================
// 1. NASTAVENÍ WI-FI A WEBSERVERU
// ==========================================
const char* AP_SSID = "RaceBox_AP";
const char* AP_PASS = "12345678"; // Min. 8 znaků
WebServer server(80);

// ==========================================
// 2. NASTAVENÍ HARDWARU A GPS
// ==========================================
TinyGPSPlus gps;
HardwareSerial GPS_Serial(2);

const int RX_PIN = 16; // Pin 16 na ESP32 propojit s TX na GPS
const int TX_PIN = 17; // Pin 17 na ESP32 propojit s RX na GPS

// ==========================================
// 3. STAVOVÝ AUTOMAT PRO MĚŘENÍ 0-100 KM/H
// ==========================================
enum StavMereni {
  CEKA_NA_START,
  PROBIHA_MERENI,
  DOKONCENO
};

StavMereni stav = CEKA_NA_START;
unsigned long cas_startu = 0;
float vysledny_cas = 0.0;
float aktualni_rychlost = 0.0;
int pocet_satelitu = 0;

void zmer_zrychleni(float v) {
  aktualni_rychlost = v;

  switch (stav) {
    case CEKA_NA_START:
      // Při překročení 1 km/h se spustí stopky
      if (aktualni_rychlost > 1.0) {
        cas_startu = millis();
        stav = PROBIHA_MERENI;
        Serial.println(">>> START: Měření zahájeno! <<<");
      }
      break;

    case PROBIHA_MERENI:
      // Detekce protnutí cílové rychlosti 100 km/h
      if (aktualni_rychlost >= 100.0) {
        vysledny_cas = (millis() - cas_startu) / 1000.0;
        stav = DOKONCENO;
        Serial.print(">>> CÍL: 0-100 km/h za ");
        Serial.print(vysledny_cas, 2);
        Serial.println(" s <<<");
      } 
      // Ochrana před falešným startem: pokud auto zastaví dříve
      else if (aktualni_rychlost < 0.5 && (millis() - cas_startu > 2000)) {
        stav = CEKA_NA_START;
        Serial.println(">>> Měření stornováno (vozidlo zastavilo). <<<");
      }
      break;

    case DOKONCENO:
      // Automatický reset po zastavení vozidla
      if (aktualni_rychlost < 1.0) {
        stav = CEKA_NA_START;
        Serial.println(">>> Připraven k dalšímu pokusu. <<<");
      }
      break;
  }
}

// ==========================================
// 4. REST API & WEBOVÉ ROZHRANÍ
// ==========================================

// Hlavní HTML dashboard (Frontend)
void handleRoot() {
  String html = R"rawliteral(
    <!DOCTYPE html>
    <html lang="cs">
    <head>
      <meta charset="utf-8">
      <meta name="viewport" content="width=device-width, initial-scale=1">
      <title>RaceBox Telemetrie</title>
      <style>
        body { font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif; background: #0f1115; color: #fff; text-align: center; margin: 0; padding: 20px; }
        .card { background: #1a1d24; border-radius: 16px; padding: 25px; max-width: 340px; margin: 20px auto; box-shadow: 0 8px 24px rgba(0,0,0,0.5); }
        .label { color: #8a909d; font-size: 13px; text-transform: uppercase; letter-spacing: 1px; }
        .speed { font-size: 72px; font-weight: 800; color: #00e676; margin: 5px 0; font-variant-numeric: tabular-nums; }
        .status-badge { display: inline-block; padding: 6px 14px; border-radius: 20px; font-size: 14px; font-weight: 600; background: #262b35; color: #ffb300; margin-top: 15px; }
        .time-box { margin-top: 20px; padding: 15px; background: #13151b; border-radius: 10px; }
        .time-val { font-size: 32px; font-weight: 700; color: #29b6f6; font-variant-numeric: tabular-nums; }
        .sat { font-size: 12px; color: #5a606d; margin-top: 15px; }
      </style>
    </head>
    <body>
      <div class="card">
        <div class="label">Rychlost</div>
        <div id="spd" class="speed">0.0</div>
        <div class="label">km/h</div>
        
        <div id="stat" class="status-badge">Čekám na rozjezd</div>
        
        <div class="time-box">
          <div class="label">0 – 100 km/h</div>
          <div id="timer" class="time-val">-- s</div>
        </div>
        
        <div id="sat" class="sat">Satelity: 0</div>
      </div>

      <script>
        // Pravidelné dotazování REST API každých 100 ms (10 Hz)
        setInterval(function() {
          fetch('/api/telemetry')
            .then(response => response.json())
            .then(data => {
              document.getElementById('spd').innerText = data.speed.toFixed(1);
              document.getElementById('stat').innerText = data.status;
              document.getElementById('timer').innerText = data.time.toFixed(2) + " s";
              document.getElementById('sat').innerText = "Satelity: " + data.satellites;
            })
            .catch(err => console.error("Chyba čtení API:", err));
        }, 100);
      </script>
    </body>
    </html>
  )rawliteral";

  server.send(200, "text/html", html);
}

// REST API endpoint vracející JSON data
void handleTelemetryApi() {
  String statusText = "Čekám na start";
  if (stav == PROBIHA_MERENI) {
    statusText = "Měřím akceleraci!";
  } else if (stav == DOKONCENO) {
    statusText = "Cíl dokončen!";
  }

  float beziciCas = (stav == PROBIHA_MERENI) ? (millis() - cas_startu) / 1000.0 : vysledny_cas;

  String json = "{";
  json += "\"speed\":" + String(aktualni_rychlost, 1) + ",";
  json += "\"status\":\"" + statusText + "\",";
  json += "\"time\":" + String(beziciCas, 2) + ",";
  json += "\"satellites\":" + String(pocet_satelitu);
  json += "}";

  // Odeslání JSON odpovědi s CORS hlavičkou
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "application/json", json);
}

// ==========================================
// 5. OBSLUHA GPS DAT
// ==========================================
void cti_data_z_gps() {
  while (GPS_Serial.available() > 0) {
    char c = GPS_Serial.read();
    if (gps.encode(c)) {
      if (gps.satellites.isUpdated()) {
        pocet_satelitu = gps.satellites.value();
      }
      if (gps.speed.isUpdated()) {
        zmer_zrychleni(gps.speed.kmph());
      }
    }
  }
}

// ==========================================
// 6. SETUP & LOOP
// ==========================================
void setup() {
  Serial.begin(115200);

  // Krok 1: Inicializace GPS a konfigurace na 115200 baud / 10 Hz
  GPS_Serial.begin(9600, SERIAL_8N1, RX_PIN, TX_PIN);
  delay(150);

  // Příkaz modulu pro přepnutí rychlosti na 115200 baud
  GPS_Serial.println("$PCAS01,5*19");
  GPS_Serial.flush();
  delay(100);

  // Přepnutí sběrnice UART2 na ESP32 na stejnou rychlost
  GPS_Serial.updateBaudRate(115200);
  delay(100);

  // Příkaz modulu pro zvýšení vzorkování na 10 Hz (interval 100 ms)
  GPS_Serial.println("$PCAS02,100*1E");
  GPS_Serial.flush();
  delay(50);

  Serial.println("GPS modul úspěšně nakonfigurován na 10 Hz.");

  // Krok 2: Spuštění Wi-Fi Access Pointu
  WiFi.softAP(AP_SSID, AP_PASS);
  Serial.println("Wi-Fi Access Point spuštěn.");
  Serial.print("SSID: ");
  Serial.println(AP_SSID);
  Serial.print("IP adresa: http://");
  Serial.println(WiFi.softAPIP());

  // Krok 3: Registrace webových cest a spuštění serveru
  server.on("/", HTTP_GET, handleRoot);
  server.on("/api/telemetry", HTTP_GET, handleTelemetryApi);
  server.begin();
  Serial.println("HTTP server a REST API běží.");
}

void loop() {
  // Zpracování síťových dotazů
  server.handleClient();

  // Čtení a parsování dat ze sériové linky GPS
  cti_data_z_gps();
}