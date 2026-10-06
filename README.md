# Ročníkový projekt IT4 2026/27

## Racebox 

### Návrh a realizace autonomní telemetrické jednotky pro měření dynamiky vozidla (Performance Meter)

### 1. Cíl práce
Cílem práce je navrhnout, zkonstruovat a naprogramovat plně autonomní telemetrickou jednotku schopnou vysoce přesného měření dynamických parametrů vozidla (akcelerace 0–100 km/h, pružné zrychlení, brzdná dráha, boční a podélné přetížení). Součástí projektu je návrh nízkoúrovňového firmware pro mikrokontrolér řešící matematickou fúzi dat v reálném čase a webový server pro ukládání, analýzu a vizualizaci telemetrických dat.

### 2. Bezpečnostní analýza a vztah k vozidlu
Zařízení je navrženo jako zcela autonomní a galvanicky oddělené od řídicích systémů vozidla:

 - Nenapojuje se na sběrnici vozu (OBD-II, CAN-bus): Fyzicky nemůže dojít k zahlcení sběrnice, zápisu chybových kódů ani poškození řídicích jednotek (ECU).

 - Nezávislé napájení: Jednotka je napájena buď z vlastního integrovaného Li-Ion akumulátoru, nebo ze standardní 12V autozásuvky (zapalovače) přes certifikovaný DC-DC měnič s integrovanými ochranami proti přepětí a zkratu.

 - Riziko poškození elektroinstalace vozu je z principu nulové.

### 3. Technologická architektura projektu
Projekt je rozdělen do čtyř provázaných technologických celků:

A. Hardware & Senzorika
 - Řídicí mikrokontrolér: ESP32 (dvoujádrový procesor 240 MHz, podpora FreeRTOS, Wi-Fi, BLE).

 - Vysokofrekvenční GNSS modul: u-blox NEO-M8N nebo NEO-M9N s nastavitelnou vzorkovací frekvencí 10 až 25 Hz (standardní GPS moduly pracují pouze na 1 Hz, což je pro dynamická měření nedostatečné).

 - IMU jednotka (Inerciální měření): 6osý akcelerometr a gyroskop **MPU-6050** vzorkovaný na frekvenci 100 Hz.

 - Lokální periferie: **0.96" OLED (SSD1306)** pro zobrazení naměřených hodnot řidiči v reálném čase, slot pro MicroSD kartu pro ukládání surových logů.

B. Nízkoúrovňový firmware (C/C++) & Fyzikální výpočty

 - Parsování binárního protokolu: Místo pomalého textového protokolu NMEA bude implementován přímo nízkoúrovňový binární protokol u-blox UBX (zprávy UBX-NAV-PVT) přes vysokorychlostní UART sběrnici (115 200–460 800 baudů).
 - G-Trigger (Hardwarová eliminace latence GPS): Detekce startu měření v přesném čase $t=0$ pomocí hardwarového přerušení (interrupt) z akcelerometru při překročení prahové hodnoty přetížení (např. 0,1 G).Sub-sample matematická interpolace: Výpočet přesného okamžiku protnutí cílové rychlosti (např. 100,00 km/h) pomocí lineární/polynomické interpolace mezi dvěma po sobě jdoucími vzorky GPS pro dosažení přesnosti na setiny sekundy.
 - Validace sklonu trati: Automatický výpočet výškového profilu trasy; detekce a penalizace sklonu vozovky převyšujícího 1 % (standard profesionální telemetrie).Využití FreeRTOS: Rozdělení úloh mezi jádra (Core 0: vysokorychlostní sběr a zápis na SD; Core 1: matematické výpočty, obsluha displeje a síťová vrstva).

C. Mechanická konstrukce & 3D modelování (CAD)
 - Návrh v CAD softwaru: Konstrukce kompaktního pouzdra s ohledem na ergonomii, chlazení a umístění GPS antény.

 - Výroba aditivní technologií (3D tisk): Použití teplotně odolných materiálů do interiéru automobilu (PETG, ASA nebo ABS – odolnost vůči teplotám nad 70 °C v letních měsících na slunci; vyřazení nevhodného PLA).

 - Mechanické uchycení: Integrované magnety a montážní body pro adaptér na sklo/palubní desku.

D. Softwarová platforma (Backend & Frontend)
- **Backend:** REST API server (Python / FastAPI nebo Django) s relační databází (PostgreSQL) nebo časovou databází pro telemetrii.

 - **Synchronizace dat:** Automatický export logů z SD karty přes Wi-Fi (připojení k domácí síti nebo mobilnímu hotspotu).

 - **Webový dashboard:**

    - Interaktivní vykreslení trasy jízdy na mapových podkladech (např. Leaflet.js).

    - Grafy závislosti rychlosti, akcelerace a podélného/bočního přetížení na čase.

    - Tabulkový přehled a porovnání jednotlivých měření (0–50 km/h, 0–100 km/h, 100–200 km/h, brzdná dráha 100–0 km/h).

### 4. Předpokládané výstupy práce
 - Funkční fyzické zařízení v 3D tištěném pouzdře připravené k provozu.

 - Zdrojový kód firmwaru pro ESP32 v C/C++ s implementovaným UBX parserem a G-triggerem.

 - Běžící webová aplikace pro nahrávání, ukládání a vizualizaci telemetrických dat.

 - Technická dokumentace obsahující elektrická schémata, CAD výkresy, popis algoritmů a naměřená reálná data s analýzou přesnosti.
