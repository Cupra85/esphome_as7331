ESPHome AS7331 Spectral UV Sensor

Dieses Projekt integriert den ams OSRAM AS7331 Spektral-UV-Sensor als Custom ESPHome Component.
Der Sensor misst UVA, UVB und UVC kontinuierlich über I²C und berechnet daraus zusätzlich
einen biologisch normgerechten UV-Index gemäß WHO / ISO 17166.

Das Projekt basiert auf:

ams OSRAM AS7331 Datasheet

SparkFun AS7331 Arduino Library

ESPHome Custom Component API

✨ Features

✔ Kontinuierliche Messung (CONT-Mode)

✔ UVA / UVB / UVC Rohwerte (Counts)

✔ UVA / UVB / UVC Bestrahlungsstärke (W/m²)

✔ UV-Index nach WHO / CIE-Erythem-Modell

✔ Automatische Gain- & Integration-Time-Regelung

✔ I²C-Adresse & Bus frei konfigurierbar

✔ ESPHome- & Home-Assistant-kompatibel

🧪 Messprinzip & UV-Index-Berechnung

Der AS7331 besitzt drei breitbandige UV-Kanäle:

Kanal	Spektralbereich	Bedeutung
UVA	ca. 320–400 nm	Geringe erythemische Wirkung
UVB	ca. 280–320 nm	Dominant für Hautrötung
UVC	ca. 230–280 nm	Technisch, nicht UV-Index-relevant
Biologische Gewichtung (Erythem)

Da die menschliche Haut auf verschiedene UV-Wellenlängen unterschiedlich reagiert, wird für den
UV-Index eine erythemische Gewichtung verwendet:

UVB dominiert die Hautrötung

UVA trägt nur minimal bei

UVC wird nicht berücksichtigt (ISO-konform)

Dieses Verfahren entspricht der SparkFun-Referenzimplementierung und einer
praxisgerechten Approximation der CIE-Erythem-Wirkungsfunktion.

Berechnungsformel
E_ery = UVB + (UVA × 0.002)
UV-Index = E_ery / 0.025

🔌 Hardware-Anschluss
AS7331	ESP32
VCC	3.3V
GND	GND
SDA	GPIO21 (Standard)
SCL	GPIO22 (Standard)

Andere Pins sind in der YAML frei wählbar.
