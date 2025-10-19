### 🏠 OpenTherm Gateway (ESPHome + ESP32)

This project is still a work in progress and will implements a **fully native OpenTherm Gateway** using an **ESP32-S3** and ESPHome.  
It acts as a modern, reliable, and flexible interface between your boiler and Home Assistant — featuring full OpenTherm communication, diagnostics, and web-accessible emergency controls.


### 🚀 Features

- Full OpenTherm Protocol Support
- Integrated Home Assistant Support
- Emergency Mode (Offline Control)
- Weather Compensation (Equitherm)  
- Diagnostic & Monitoring
– Real-time logging (boiler state, flame, fault codes)  
– Flow detection and DHW water rate measurement  

### ⚙️ Hardware Overview

| Component | Purpose |
|------------|----------|
| **ESP32-S3 DevKitC N16R8** | Main controller running ESPHome (could be any ESP32-S3) |
| **OpenTherm Interface Circuit** | Connects boiler OpenTherm bus to ESP (isolation required!) |


### 🖥️ Web Interface

- Access via http://otgateway.local (or the device IP)
- Toggle Emergency Mode to enable offline control
- Manually control:
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

```T_flow = n * (T_set + k - T_out) + t + (T_set - T_in) * fb```

where:

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


<details>
<summary><strong>🔍 Explanation</strong></summary>

The Equithermic control curve automatically increases boiler flow temperature when it’s colder outside, keeping the indoor temperature stable with minimal cycling and optimal condensing efficiency.

- **n** and **k** shape how steeply the flow temperature reacts to outdoor changes.  
- **t** vertically shifts the entire curve, roughly aligning it with the desired comfort level.  
- **fb** applies an indoor feedback correction:  
  - If the indoor temperature is *below* the target, the flow temperature increases slightly.  
  - If it’s *above* the target, it decreases slightly.  
- The final calculated temperature is **clamped** to the `Maximum CH Temperature` number you define in Home Assistant.

</details>
