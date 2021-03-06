#TODO: to be updated for LPA* branches
# MultiplexMC2020Retrofit
Multiplex MC2020 firmware replacement with Arduino Mega2560

Firmware/MCU replacement to retrofit a Multiplex MC2020 radiotransmitter.
The HF module is kept.

Use a SSD1327 OLED 128x128 OLED display in place of the liquid crystal display.

<div style="page-break-after: always"></div>

# Wiring pour NANO sur �metteur simple

![NANO pinout](doc/Arduino-Nano-pinout.jpg)

<div style="page-break-after: always"></div>

![OLED pinout](doc/Diagram-OLED.jpg)

<div style="page-break-after: always"></div>

*Pr�voir une r�sistance de pull-up de 4.7kOhm � 5V sur SDA et SCL !*

| Pin   | Nom          |  Type de Signal  |    A relier sur      |
|:------|:-------------|:-----------------|:--------------------:|
| D13   | Led interne  | Di.Out 0/5V      |          -           |
| 3.3V  | 3.3V R�gul.  | Sortie 3.3V      |      OLED VCC        |
| AREF  | Ref.interne  | 0..5V            |          -           |
| A0    | Profondeur   | An.In 0..5V      | Potar Prof           |
| A1    | Direction    | An.In 0..5V      | Potar Direc          |
| A2    | Gaz          | An.In 0..5V      | Potar Gaz            |
| A3    | Aileron      | An.In 0..5V      | Potar Aileron        |
| A4    | SDA (I2C)    | I2C Data 5V      | OLED SDA et pull-up 4.7kOhm |
| A5    | SCL (I2C)    | I2C Clock 5V     | OLED SCL et pull-up 4.7kOhm |
| A6    | Trim1        | An.In 0..5V      |          -           |
| A7    | Trim2        | An.In 0..5V      |          -           |
| +5V   | 5V R�gul.    | Sortie 5V        | commun des pull-up 4.7kOhm   |
| RST   | Reset �C     | Entr�e GND       |          -           |
| GND   | Masse        | GND              | commun GND           |
| VIN   | Alim r�gul.  | 7-12V            | (+) batteries        |


<div style="page-break-after: always"></div>


| Pin   | Nom          |  Type de Signal  |    A relier sur      |
|:------|:-------------|:-----------------|:--------------------:|
| D12   | Btn Reset Config | Entr�e bouton | GND � ramener via bouton |
| D11   |              |                  |           -          |
| D10   | PPM          | Pulses 0/5V      | Entr�e module HF     |
| D9    | Btn OPtion 2 | Entr�e bouton    | GND � ramener via bouton |
| D8    | Btn Option 1 | Entr�e bouton    | GND � ramener via bouton |
| D7    | Btn Coupling | Entr�e bouton    | GND � ramener via bouton |
| D6    | Btn Dual Rate | Entr�e bouton   | GND � ramener via bouton |
| D5    | Btn Page     | Entr�e bouton    | GND � ramener via bouton |
| D4    | Btn Next     | Entr�e bouton    | GND � ramener via bouton |
| D3    | Btn Moins    | Entr�e bouton    | GND � ramener via bouton |
| D2    | Btn Plus     | Entr�e bouton    | GND � ramener via bouton |
| GND   | Masse        | GND              | commun GND           |
| RST   | Reset �C     | Entr�e GND       |           -          |
| RX0   |              |                  |           -          |
| TX1   |              |                  |           -          |


<div style="page-break-after: always"></div>


```
[SDA NANO] -----o----- [SDA OLED]
                |
                o
           [4.7 kOhm]
                o
                |
                o
              [+5V]
                o
                |
                o
            [4.7 kOhm]
                o
                |
[SCL NANO] -----o----- [SCL OLED]

```

# Wiring pour MEGA2560 sur �metteur MC2020

![Mega2560 pinout](doc/mega2560pinout.png)

<div style="page-break-after: always"></div>

![OLED pinout](doc/Diagram-OLED.jpg)

<div style="page-break-after: always"></div>

*Pr�voir une r�sistance de pull-up de 4.7kOhm � 5V sur SDA et SCL !*

| Pin   | Nom          |  Type de Signal  |    A relier sur      |
|:------|:-------------|:-----------------|:--------------------:|
| D13   | Led interne  | Di.Out 0/5V      |          -           |
| 3.3V  | 3.3V R�gul.  | Sortie 3.3V      |      OLED VCC        |
| AREF  | Ref.interne  | 0..5V            |          -           |
| A0    | Profondeur   | An.In 0..5V      | Potar Profondeur     |
| A1    | Direction    | An.In 0..5V      | Potar Direction      |
| A2    | Gaz          | An.In 0..5V      | Potar Gaz            |
| A3    | Aileron      | An.In 0..5V      | Potar Aileron        |
| A4    | Flap         | An.In 0..5V      | Potar volets         |
| A5    | Train        | An.In 0..5V      | Potar Train          |
| A6    | Trim1        | An.In 0..5V      | Trim Profondeur      |
| A7    | Trim2        | An.In 0..5V      | Trim Direction       |
| A8    | Trim3        | An.In 0..5V      | Trim Gaz             |
| A9    | Trim4        | An.In 0..5V      | Trim Aileron         |
| +5V   | 5V R�gul.    | Sortie 5V        | commun des pull-up 4.7kOhm   |
| RST   | Reset �C     | Entr�e GND       |          -           |
| GND   | Masse        | GND              | commun GND           |
| VIN   | Alim r�gul.  | 7-12V            | (+) batteries        |


<div style="page-break-after: always"></div>


| Pin   | Nom          |  Type de Signal  |    A relier sur      |
|:------|:-------------|:-----------------|:--------------------:|
| SDA   | SDA (I2C)    | I2C Data 5V      | OLED SDA et pull-up 4.7kOhm |
| SCL   | SCL (I2C)    | I2C Clock 5V     | OLED SCL et pull-up 4.7kOhm |
| D12   | Btn Reset Config | Entr�e bouton | GND � ramener via bouton |
| D11   |              |                  |           -          |
| D10   | PPM          | Pulses 0/5V      | Entr�e module HF     |
| D9    | Btn OPtion 2 | Entr�e bouton    | GND � ramener via bouton |
| D8    | Btn Option 1 | Entr�e bouton    | GND � ramener via bouton |
| D7    | Btn Coupling | Entr�e bouton    | GND � ramener via bouton |
| D6    | Btn Dual Rate | Entr�e bouton   | GND � ramener via bouton |
| D5    | Btn Page     | Entr�e bouton    | GND � ramener via bouton |
| D4    | Btn Next     | Entr�e bouton    | GND � ramener via bouton |
| D3    | Btn Moins    | Entr�e bouton    | GND � ramener via bouton |
| D2    | Btn Plus     | Entr�e bouton    | GND � ramener via bouton |
| GND   | Masse        | GND              | commun GND           |
| RST   | Reset �C     | Entr�e GND       |           -          |
| RX0   |              |                  |           -          |
| TX1   |              |                  |           -          |


<div style="page-break-after: always"></div>


