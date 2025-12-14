# OpenTherm Gateway 
[![GitHub release](https://img.shields.io/github/v/release/dhoeben/esp32-opentherm.svg)](https://GitHub.com/dhoeben/esp32-opentherm/releases/) [![ESPHome version](https://img.shields.io/badge/Based%20on%20ESPHome-v2025.12.1-blue)](https://GitHub.com/esphome/esphome/releases/)



This project implements a **fully native OpenTherm Gateway** using an **ESP32-S3** chip and ESPHome.  
It acts as a modern, reliable, and flexible interface between your boiler and Home Assistant — featuring full OpenTherm communication, diagnostics, and web-accessible emergency controls.
It is designed to work with the PCB attached in the .pcb folder, to create the PCB just download the gerber files and order at your convenience.

*This is a hobby project, use at your own risk. It might damage your boiler when used incorrectly.*
*Do not sell!*


Feel free to also checkout my [battery powered thermostat](https://github.com/dhoeben/esp32-thermostat), which features an e-ink display, fully customizable and has it's own PCB you can just order! 


### ⬇️ Installation
1) Order PCB using gerber_files and 3D print / assemble case.
2) Download latest release
3) Update secrets.yaml
4) Use vscode or flash tool to create firmware 
5) Flash to PCB using USB.
6) Connect to boiler's OpenTherm connectors.
7) Connect to Home Assistant


### 🚀 Featuress
- Full OpenTherm Protocol Support
- Integrated Home Assistant Support
- Emergency Mode (Offline Control)
- Optional weather Compensation (Equitherm or boilers own)  
- Diagnostic & Monitoring (boiler state, flame, fault codes)  

### ⚙️ PCB Overview
- 4 layer PCB build from scratch
- Used the best part and foodprint for the task.
- Created Galvanic isolation, so no false signals or inteferance.
- Low power usage and "light off principle"
- 7 multicolor LED's for status of the gateway
- Used an overpowered ESP32-S3, so there will be no issues in the future
- Used BME680 on PCB for a back-up temperature. Also gives CO2 equivalent, for monitoring of air quality in boiler room. 
**Warning: Air quality sensing is just for fun, always use proper safety equipment for air quality monitoring.**

This also works with *any* ESP32-S3, however it is not fully supported and tested. Make sure to update secrets.yaml according to your config.

### 🖥️ Web Interface
- Access via http://otgateway.local (or the device IP)
- Option to toggle Emergency Mode and enable offline control
- Option to control even without Home Assistant or during (temporary) failure.
- Heating (CH) control
- Domestic Hot Water (DHW) control
- All controls are processed locally on the ESP chip — not on Home Assistant.

### 🔒 OTA & Maintenance
- Updates can be pushed via Home Assistant or directly through ESPHome
- OTA encrypted and password protected
- Automatic reboot after successful updates
- Safe Mode available if configuration fails to boot
- External component for OpenTherm: stable releases and adapt config as you please without issues

### 🧮 Equitherm Heating Curve
This project implements a custom **Equithermic control algorithm** that dynamically adjusts the **boiler flow temperature** based on outdoor and indoor conditions.  
It uses the same principle as weather-compensated control, but allows you to fully tune the curve parameters via Home Assistant. 
This takes time, but eventually will save you a lot of money (and it is envoirmental friendly).

You can also switch it off.

<details>
<summary><strong>More information</strong></summary>

```T_flow = n * (T_set + k - T_out) + t + (T_set - T_in) * fb```

| Symbol | Definition | Source |
|:-------|:------------|:-------|
| **T₍flow₎** | Calculated target flow temperature (°C) | Sent to boiler via OpenTherm (DID 0x11) |
| **T₍out₎** | Outdoor temperature (°C) | From Home Assistant weather sensor |
| **T₍in₎** | Current indoor temperature (°C) | From indoor temperature sensor |
| **T₍set₎** | Desired indoor setpoint (°C) | From ESPHome `climate` target |
| **n** | Curve exponent / slope multiplier | Tunable number (default ≈ 1.2–1.3) |
| **k** | Base slope factor | Tunable number (default ≈ 0.8–1.0) |
| **t** | Curve offset (°C) | Shifts the curve up/down; usually near indoor target |
| **fb** | Feedback gain | Correction factor based on indoor deviation |
| **max_ch_temp** | Maximum boiler flow limit (°C) | Adjustable from Home Assistant |


The Equithermic control curve automatically increases boiler flow temperature when it’s colder outside, keeping the indoor temperature stable with minimal cycling and optimal condensing efficiency.

- **n** and **k** shape how steeply the flow temperature reacts to outdoor changes.  
- **t** vertically shifts the entire curve, roughly aligning it with the desired comfort level.  
- **fb** applies an indoor feedback correction:  
  - If the indoor temperature is *below* the target, the flow temperature increases slightly.  
  - If it’s *above* the target, it decreases slightly.  
- The final calculated temperature is **clamped** to the `Maximum CH Temperature` number you define in Home Assistant.

</details>
