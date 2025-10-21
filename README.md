Tested OK on ESPHome software version 2025.9.3 - 2025.10.1

### 🏠 OpenTherm Gateway (ESPHome + ESP32)
This project is still a work in progress and will implements a **fully native OpenTherm Gateway** using an **ESP32-S3** and ESPHome.  
It acts as a modern, reliable, and flexible interface between your boiler and Home Assistant — featuring full OpenTherm communication, diagnostics, and web-accessible emergency controls.

### ⬇️ Installation
1) Download latest release
2) Use vscode or flash tool to create firmware 
3) Flash to your ESP device
4) Connect ESP device to Opentherm Interface Circuit
5) Connect Opentherm Interface Circuit to boiler OT+/- 

<details>
<summary><strong>Connection schedule</strong></summary>
| ESP32-S3 DevKitC-1   | DIYless Thermostat Shield | Function           | Notes                                                              |
| -------------------- | ------------------------- | ------------------ | ------------------------------------------------------------------ |
| **3V3 (pin 1 or 2)** | **3V3**                   | Power              | powers the shield logic (it runs at 3.3 V). **Do NOT** use 5 V.    |
| **GND (pin 22)**     | **GND**                   | Ground             | common reference for logic side.                                   |
| **GPIO 17**          | **D5**                    | OpenTherm TX (OUT) | from ESP32 to shield (MCU drives line driver).                     |
| **GPIO 18**          | **D6**                    | OpenTherm RX (IN)  | from shield to ESP32 (optocoupler output).                         |


</details>
### 🚀 Features
- Full OpenTherm Protocol Support
- Integrated Home Assistant Support
- Emergency Mode (Offline Control)
- Optional weather Compensation (Equitherm or boilers own)  
- Diagnostic & Monitoring (boiler state, flame, fault codes)  

### ⚙️ My Hardware Overview
| Component | Purpose |
|------------|----------|
| **ESP32-S3 DevKitC N16R8** | Main controller running ESPHome (could be any ESP32-S3) |
| **OpenTherm Interface Circuit** | Connects boiler OpenTherm bus to ESP |


### 🖥️ Web Interface
- Access via http://otgateway.local (or the device IP)
- Toggle Emergency Mode to enable offline control
- Manually control
- Heating (CH) On/Off
- Domestic Hot Water (DHW) On/Off
- Target temperatures
- All controls are processed locally — no HA required.

### 🔒 OTA & Maintenance
- Updates can be pushed via Home Assistant or directly through ESPHome
- OTA encrypted and password protected
- Automatic reboot after successful updates
- Safe Mode available if configuration fails to boot

### 🧮 Equitherm Heating Curve
This project implements a custom **Equithermic control algorithm** that dynamically adjusts the **boiler flow temperature** based on outdoor and indoor conditions.  
It uses the same principle as weather-compensated control, but allows you to fully tune the curve parameters via Home Assistant.

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

### ⚫ Status LED
| Condition                 | Color     | Behavior                       |
| ------------------------- | --------- | ------------------------------ |
| **Booting (<10 s)**       | 🟢 Green  | Device initialization phase    |
| **Wi-Fi disconnected**    | 🔵 Blue   | No Wi-Fi connection            |
| **OpenTherm comm error**  | 🟣 Purple | No or invalid boiler response  |
| **Emergency mode active** | 🟠 Orange | Manual override or safety mode |
| **Boiler fault**          | 🔴 Red    | Boiler fault detected          |
| **System OK / standby**   | ⚫ Off    | Everything normal              |