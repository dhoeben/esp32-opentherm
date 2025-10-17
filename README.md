### 🏠 OpenTherm Gateway (ESPHome + ESP32)

This project is still a work in progress and will implements a **fully native OpenTherm Gateway** using an **ESP32-S3** and ESPHome.  
It acts as a modern, reliable, and flexible interface between your boiler and Home Assistant — featuring full OpenTherm communication, diagnostics, and web-accessible emergency controls.


### 🚀 Features

✅ **Full OpenTherm Protocol Support**  
– Native communication with OpenTherm-compatible boilers  
– Continuous polling with configurable intervals  
– Automatic modulation and setpoint control  

✅ **Integrated Home Assistant Support**  
– Exposes boiler temperatures, DHW, modulation, and status  
– Adjustable temperature limits and domestic hot water (DHW) setpoints  
– Binary sensors for flame, faults, diagnostics, and communication health  

✅ **Emergency Mode (Offline Control)**  
– Local fallback via web interface  
– Manual switches for heating or DHW, even when Home Assistant is offline  

✅ **Weather Compensation (Equitherm)**  
– Calculates target flow temperature dynamically  
– Adjustable slope and offset values through Home Assistant  

✅ **Diagnostic & Monitoring**  
– Real-time logging (boiler state, flame, fault codes)  
– Flow detection and DHW water rate measurement  

✅ **Optional Add-ons**  
– mDNS: access gateway via `http://otgateway.local`  
– Web UI for emergency mode and manual control  
– (Optional) DHW water usage statistics in HA’s Energy dashboard  
– (Optional) Power monitoring via ADC or smart plug  


### ⚙️ Hardware Overview

| Component | Purpose |
|------------|----------|
| **ESP32-S3 DevKitC N16R8** | Main controller running ESPHome (could be any ESP32-S3) |
| **OpenTherm Interface Circuit** | Connects boiler OpenTherm bus to ESP (isolation required!) |


### 🖥️ Web Interface (Emergency Mode)

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