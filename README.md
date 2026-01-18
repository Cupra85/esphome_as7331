# ESPHome AS7331 Spectral UV Sensor

Dieses Projekt integriert den **ams OSRAM AS7331** Spektral-UV-Sensor als **Custom ESPHome Component**.
Der Sensor misst **UVA, UVB und UVC** kontinuierlich über I²C und berechnet daraus zusätzlich
einen **biologisch normgerechten UV-Index** (WHO / ISO 17166).

Das Projekt basiert auf:
- ams OSRAM AS7331 Datasheet
- SparkFun AS7331 Arduino Library
- ESPHome Custom Component API

---

## ✨ Features

- ✔ Kontinuierliche Messung (CONT-Mode)
- ✔ UVA / UVB / UVC Rohwerte (Counts)
- ✔ UVA / UVB / UVC Bestrahlungsstärke (W/m²)
- ✔ UV-Index nach WHO / CIE-Erythem-Modell
- ✔ Automatische Gain- & Integration-Time-Regelung
- ✔ I²C-Adresse & Bus frei konfigurierbar
- ✔ Keine externen Libraries notwendig
- ✔ ESPHome- & Home-Assistant-kompatibel

---

## 🧪 Messprinzip

Der AS7331 besitzt drei breitbandige Kanäle:

| Kanal | Spektralbereich | Verwendung |
|-----|----------------|------------|
| UVA | ca. 320–400 nm | UV-Leistung |
| UVB | ca. 280–320 nm | UV-Index (dominant) |
| UVC | ca. 230–280 nm | technische Messung |

### UV-Index-Berechnung

Der UV-Index wird **biologisch gewichtet** berechnet:

