# ESPHome AS7331

ESPHome External Component für den AS7331 UV-Sensor.

## Features
- UVA / UVB / UVC Raw Counts
- Berechnete Leistung in W/m²
- UV Index (ELV-nah)
- YAML identisch nutzbar wie vitoconnect (filters, internal, accuracy_decimals)

## Beispiel

```yaml
sensor:
  - platform: as7331
    address: 0x77

    uva_raw:
      name: "AS7331 UVA Raw"
      accuracy_decimals: 0
      internal: true

    uva:
      name: "AS7331 UVA Leistung"
      accuracy_decimals: 2

    uv_index:
      name: "UV Index"
      accuracy_decimals: 2
