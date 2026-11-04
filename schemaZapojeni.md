#Schéma zapojení 

**Vlevo nahoře:** 
  *  **I2C OLED Displej (SSD1306)**
      * VCC: červená linka **3V3**
      * GND: černá linka **GND**
      * SDA: zelený vodič -> **GPIO 8**
      * SCL: žlutý vodič -> **GPIO 9**
      
**Vlevo dole:** 
  * **I2C Akcelerometr (GY-521 MPU-6050)**
      * VCC: červená linka **3V3**
      * GND: černá linka **GND**
      * SDA: zelený vodič -> **GPIO 8** (spojeno paralelně s displejem)
      * SCL: žlutý vodič -> **GPIO 9** (spojeno paralelně s displejem)
      * AD0: propojit na **GND** (fixuje I2C adresu na `0x68`)
            
**Vpravo nahoře:** 
  * **UART GPS Modul (NEO-6M)**
      * VCC: červená linka **3V3** nebo **5V** (modul má vlastní regulátor)
      * GND: černá linka **GND**
      * TX: modrý vodič -> **GPIO 18** (RX na ESP32)
      * RX: bílý vodič -> **GPIO 17** (TX na ESP32)

**Vpravo dole:**
  * **SPI Čtečka MicroSD**
      * VCC: červená linka **3V3** nebo **5V**
      * GND: černá linka **GND**
      * CS: fialový vodič -> **GPIO 10**
      * DI(MOSI): hnědý vodič -> **GPIO 11**
      * SCK: šedý vodič -> **GPIO 12**
      * DO(MISO): oranžový vodič -> **GPIO 13**   

![Schéma]([https://github.com/AdamBohm/rocnikovy-projekt/new/main](https://github.com/AdamBohm/rocnikovy-projekt/blob/main/Schema%20zapojeni,%20rocnikovy%20projekt.png?raw=true))
