Tested OK on ESPHome software version 2025.9.3 - 2025.10.1

### 🏠 OpenTherm Gateway (ESPHome + ESP32)
This project is still a work in progress and will implements a **fully native OpenTherm Gateway** using an **ESP32-S3** chip and ESPHome.  
It acts as a modern, reliable, and flexible interface between your boiler and Home Assistant — featuring full OpenTherm communication, diagnostics, and web-accessible emergency controls. To create the PCB just download the gerber files from .pcb folder, then order at your convenience.

Feel free to also use my thermostat I created, which uses a E-ink display and is also fully customizable. Also has his own PCB you can just order! https://github.com/dhoeben/esp32-thermostat. 

### ⬇️ Installation
1) Download latest release
2) Use vscode or flash tool to create firmware 
3) Flash to your ESP device
4) Connect ESP device to Opentherm Interface Circuit
5) Connect Opentherm Interface Circuit to boiler OT+/- 

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
| **Boiler fault**          | 🔴 Red    | Boiler fault detected          |
| **Emergency mode active** | 🟠 Orange | Manual override or safety mode |
| **Backup Temp used**      | 🟡 Yellow | Onboard BME680 is used         |
| **Connected to power**    | 🟢 Green  | Power is connected             |
| **Wi-Fi disconnected**    | 🔵 Blue   | No Wi-Fi connection            |
| **OpenTherm comm error**  | 🟣 Purple | No or invalid boiler response  |
| **Home Assistant error**  | ⚪ White  | Connect to Home Assistant      |
| **Board has no power**    | ⚫ Off    | Connect to power               |