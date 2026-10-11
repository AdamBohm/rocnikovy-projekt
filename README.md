# Ročníkový projekt IT4 2026/27

## RaceBox / IoT Performance Meter
**Návrh a realizace autonomní telemetrické jednotky pro měření dynamiky vozidla**

---

### 1. Cíl práce
Cílem práce je navrhnout, zkonstruovat a naprogramovat plně autonomní telemetrickou jednotku schopnou vysoce přesného měření dynamických parametrů vozidla (akcelerace 0–100 km/h, pružné zrychlení, brzdná dráha, boční a podélné přetížení). Součástí projektu je návrh mechanického pouzdra v CADu pro 3D tisk, nízkoúrovňový firmware řešící fúzi senzorových dat a integrovaný asynchronní webserver poskytující vizualizaci dat na připojená mobilní zařízení bez nutnosti externího serveru.

---

### 2. Bezpečnostní analýza a vztah k vozidlu (Zero-Risk architektura)
Zařízení je navrženo jako zcela autonomní a galvanicky oddělené od řídicích systémů vozidla:
* **Nenapojuje se na sběrnici vozu (OBD-II, CAN-bus):** Fyzicky nemůže dojít k zahlcení sběrnice, zápisu chybových kódů ani poškození řídicích jednotek (ECU).
* **Nezávislé napájení:** Jednotka je napájena ze standardní 12V autozásuvky přes USB/DC-DC adaptér nebo z powerbanky.
* Riziko poškození elektroinstalace vozu je z principu **nulové**.

---

### 3. Technologická architektura projektu

#### A. Hardware & Senzorika
* **ESP32-S3 (nebo ESP32 DevKit):** Výkonný dvoujádrový mikrokontrolér s integrovanou Wi-Fi, obsluhující výpočty, senzory i lokální webserver.
* **ATGM336H GNSS modul:** Vícesystémový satelitní přijímač (GPS/BDS), konfigurovatelný až na 10 Hz. Tato frekvence v kombinaci s IMU plně postačuje pro přesnou fúzi dat a odstraňuje dlouhé zpoždění standardních (1 Hz) GPS přijímačů.
* **MPU-6050 IMU:** 6osý inerciální senzor (akcelerometr + gyroskop) vzorkovaný na 100 Hz pro okamžitou detekci přetížení.
* **0.96" OLED displej (SSD1306):** Lokální zobrazení rychlosti na zařízení (I2C sběrnice).
* **MicroSD SPI modul:** Samostatná hardwarová čtečka připojená k ESP32 přes SPI pro vysokorychlostní zápis surových logů z jízd.

#### B. Komunikační sběrnice a alokace pinů
Přestože ESP32 disponuje dostatkem volných GPIO pinů (přes 20 volných pro další rozšíření), návrh využívá standardní průmyslové sdílení sběrnic pro zjednodušení kódu a hardwarové stability:
* **I2C Sběrnice:** Společná pro MPU-6050 (adresa `0x68`) a OLED (adresa `0x3C`). Využívá 2 piny (SDA, SCL).
* **UART Sběrnice:** Vyhrazena pro komunikaci s GPS modulem.
* **SPI Sběrnice:** Vyhrazena pro MicroSD modul.

#### C. Nízkoúrovňový firmware (C/C++) & Fyzikální výpočty
* **Fúze dat (Dead Reckoning):** Softwarová kompenzace latence GPS pomocí 100Hz dat z akcelerometru. 
* **G-Trigger (Eliminace zpoždění startu):** Měření nezahajuje pomalá GPS, ale hardwarové přerušení z akcelerometru při zachycení pohybu vpřed.
* **Sub-sample interpolace:** Výpočet času na setiny sekundy interpolací bodů mezi 10 Hz vzorky rychlosti z GPS modulu.

#### D. Mechanická konstrukce & 3D modelování (CAD)
* **Návrh v CAD softwaru:** Modelování pouzdra pro umístění za čelní sklo (optimální signál).
* **Výroba 3D tiskem:** Tisk z materiálu PETG/ASA (odolnost vůči teplotám nad 70 °C, vyloučeno PLA).

#### E. Vestavěný Webserver a Frontend Vizualizace (Stand-alone řešení)
Zařízení nevyžaduje žádný cloud, externí aplikaci ani backend na PC.
* **ESP32 v režimu Access Point (AP):** Krabička vysílá vlastní Wi-Fi síť s dedikovanou IP adresou (např. `192.168.4.1`).
* **Asynchronní Webserver:** Uložený v lokální flash paměti mikrokontroléru (LittleFS). Obsluhuje požadavky bez blokování ostatních procesů (FreeRTOS).
* **Webový Dashboard (HTML/JS):** Po připojení libovolného mobilního telefonu si uživatel stáhne interaktivní stránku přímo ze zařízení.
* **Přenos přes WebSockets:** Data z měření (časy zrychlení, okamžité přetížení G-Force, aktuální rychlost) jsou streamována z ESP32 do prohlížeče v reálném čase.
* Pro vykreslování grafů a vizualizaci dat v prohlížeči mohou být využity standardní JS knihovny (např. Chart.js).
