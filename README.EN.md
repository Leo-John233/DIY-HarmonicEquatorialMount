<div align="right">
  <a href="./README.md"><img src="https://img.shields.io/badge/🌐_Language-简体中文-0052D4.svg?style=flat" alt="简体中文"></a>
  <a href="./README.EN.md"><img src="https://img.shields.io/badge/🌐_Language-English-C8102E.svg?style=flat" alt="English"></a>
</div>

## 📄 License / Open Source Agreement

This project employs a combination of open-source licenses, with separate licensing applied to the software and hardware components:

*   **Software** (e.g., firmware code): Licensed under the [GNU General Public License v3.0 (GPL-3.0)](LICENSE-GPL). For detailed information, please refer to the `LICENSE-GPL` file.
*   **Hardware** (e.g., PCB designs, schematics, mechanical 3D models): Licensed under the [CERN Open Hardware Licence Version 2 - Strongly Reciprocal (CERN-OHL-S v2)](LICENSE-CERN). For detailed information, please refer to the `LICENSE-CERN` file.

---
# DIY Harmonic Equatorial Mount (Power-on Verified; Awaiting Field Testing)

Having grown weary of the sheer weight of my EQ3D equatorial mount, I planned to replace it with a lighter, more portable harmonic mount. However, commercially available harmonic mounts are prohibitively expensive (especially when compared to the specifications achievable via DIY). Thus, this project was born: a harmonic equatorial mount capable of delivering high precision, portability, and cost-effectiveness—all without compromising on performance.

# Project Description

The core component of this homemade harmonic equatorial mount—the harmonic reducer—is based on a used, industrial-grade unit salvaged from a Leaderdriver assembly. (Note: This design is also compatible with harmonic reducers of the same specifications purchased on Taobao—e.g., the "25-100" model.)
![image.png](https://github.com/Leo-John233/DIY-Harmonic-Equatorial-Mount/blob/main/pictures/3D-Views/1.png)
![image.jpg](https://github.com/Leo-John233/DIY-Harmonic-Equatorial-Mount/blob/main/pictures/3D-Views/3.jpg)
![image.jpg](https://github.com/Leo-John233/DIY-Harmonic-Equatorial-Mount/blob/main/pictures/3D-Views/5.JPG)
![image.jpg](https://github.com/Leo-John233/DIY-Harmonic-Equatorial-Mount/blob/main/pictures/3D-Views/4.jpg)
![image.jpg](https://github.com/Leo-John233/DIY-Harmonic-Equatorial-Mount/blob/main/pictures/3D-Views/2.jpg)

This project aims to address several critical issues commonly encountered in amateur astronomy equipment:
- The high cost of commercially manufactured mounts.
- A lack of openness and flexibility in design.
- Excessive complexity or incompatibility with individual equipment setups.

The key advantages of this DIY harmonic equatorial mount include:
- 🧰 **Cost-Effective** — The total build cost is under 3,000 RMB, with the cost of custom-machined parts amounting to less than 1,500 RMB.
- 🪶 **Relatively Portable** — Weighs 7.5 kg (excluding base)
- 🏗️ **Easy to Build** — Designed for makers with basic tools
- 💪 **High Performance** — Precise tracking (sub-arcsecond RMS), no counterweights required
- ⚠️ **Note on Current Project Status** ⚠️
This project has currently only reached the stage of power-on and simple "GoTo" testing; no actual astrophotography tests have been conducted yet.

# ✨ Key Features
| **Feature** | **Description** |
| :----- | :----- |
| Mount Type | German Equatorial Mount (GEM) |
| Drive Type | Harmonic Drive + Belt Drive + 42 Stepper Motor |
| Payload Capacity | Max. 30kg (counterweights recommended); 20–25kg without counterweights |
| Harmonic Drive Model | Leaderdrive LHS-25-100-C-III |
| Control System | OnStep / OnStepx |
| Connectivity | USB-B; Wi-Fi; Bluetooth; Integrated DS3231SN Real-Time Clock Chip |
| Hardware Features | RA Axis Brake; RA/DEC Axis Photoelectric Limit Sensors; DEC/RA Axis Photoelectric Home Sensors;<br>Simultaneous compatibility with both Step/Dir and SPI driver modes (i.e., compatible with TMC2209 and TMC5160 drivers); 12V/24V Input |
| Estimated Build Cost | 2200–2600 RMB |
| Estimated Weight | 9.5kg (including dovetail saddle and latitude base) |

## 🔗 Bill of Materials (BOM)

Please refer to [BOM.md](BOM.md)

**Estimated Total Cost: Approx. 2500 RMB (including latitude base)**

The main structural components of this project rely heavily on custom fabrication. Key components include two size 25-100 harmonic drives, which provide an exceptionally high gear ratio and torque output. The base interface can be adapted to fit most mainstream tripods, allowing for customization based on individual preference.

*Note: Prices are approximate and may vary depending on the motor supplier, CNC machining batch, and geographic location.*

---

## 🔌 Electronics Overview

The control system features a completely custom design, built around a compact architecture, and includes:

*   **An ESP32-WROOM-32UE16N-based main control core**, running the powerful [OnStep/OnStepX](https://onstep.groups.io/g/main/wiki) system.
*   **Integrated ESP07S wireless connectivity**, providing efficient wireless and wired communication capabilities.
*   **TMC-series stepper motor drivers**, enabling precise, silent microstepping control and pointing for both Right Ascension (RA) and Declination (Dec) axes.
*   **An efficient power management solution**, featuring an onboard TPS5450 buck converter module and AMS1117 LDO voltage regulator chips to ensure clean and stable system voltage, even under heavy loads.
*   Integrated interfaces for **home sensors and limit switches**, ensuring device safety during operation.
*   Support for remote control via Wi-Fi, Bluetooth, or USB routing, allowing for seamless integration with mainstream astrophotography software ecosystems (N.I.N.A., INDI, ASCOM, etc.).
*   The control board's PCB layout is designed with future expandability in mind; using jumpers located next to the main board's driver slots, you can easily physically switch between the advanced **SPI Smart Control Mode** and the traditional **STEP/DIR Independent Mode** (firmware reflashing is required after switching modes).

*   **TMC5160 SPI Mode (Recommended Configuration) 🚀**
*   **Hardware Setup:** Use jumper caps to bridge the dedicated SPI pin headers, directly connecting the ESP32's SPI bus to the TMC5160 drivers.
*   **System Advantages:** Completely eliminates the need for manual potentiometer adjustment, allowing the OnStep firmware to directly read from and write to the driver registers. You can configure run/hold currents and precisely set microstepping resolutions in real-time—either via the web interface or within `Config.h`—and intelligently switch between StealthChop (ultra-silent operation) and SpreadCycle (high dynamic torque) modes to maximize the performance of the harmonic drive reducer.
*   **Firmware Integration:** Within `Config.h`... The RA/Dec drive model is defined as `TMC5160_SPI`.

* **Traditional STEP/DIR Mode (Backward Compatible) 🔌**
* **Hardware Setup:** Disconnect the SPI communication jumpers.
* **Use Case:** Serves as a reliable low-level fallback solution. If you need to temporarily substitute standard A4988 or DRV8825 drivers, or if you choose to utilize the "Standalone" mode of the TMC drivers, this design offers exceptional hardware fault tolerance and ease of maintenance.
* **Firmware Integration:** You must precisely enter the microstep values—as configured via jumper caps—into the `Config.h` file. Furthermore, you **must manually use a multimeter and a ceramic screwdriver** to adjust the VREF potentiometer on the driver board to set the motor current.

> 💡 **Hardware Design Tip:** To ensure data integrity during high-speed SPI communication, we have implemented strict spacing controls for the relevant signal traces on our custom PCB.
**Warning: Before inserting or removing driver modules, or altering jumper settings, you must completely disconnect the system's main power supply (including USB power) to prevent transient voltage surges from damaging the TMC5160 drivers or the ESP32 microcontroller.**

All circuit schematics and PCB layouts—which feature strictly controlled safety clearances, via dimensions, and high-current trace widths—were designed using open-source tools. You can order the PCBs directly from [JLC PCB](https://member.jlc.com/?spm=PCB.Homepage) or modify the designs to suit your specific requirements.

## 💻 Firmware Configuration and Flashing

The main controller for the harmonic equatorial mount features an **ESP32-WROOM-32UE (16MB)** module. This high-capacity core not only effortlessly runs the complex [OnStep / OnStepX](https://onstep.groups.io/g/main/wiki) firmware but also provides ample storage space for built-in star catalogs, high-precision pointing models, and a feature-rich Smart Web UI.

Please refer to [Firmware_Setup.md](https://github.com/Leo-John233/DIY-Harmonic-Equatorial-Mount/blob/main/software/Firmware_Setup.md) for a complete list of `Config.h` parameters and detailed pin mappings.

---

## 🛠️ Mechanical Assembly Guide

Please refer to [Assembly.md](https://github.com/Leo-John233/DIY-Harmonic-Equatorial-Mount/blob/main/hardware/Assembly.md)

Every component has been meticulously selected or designed to ensure high mechanical precision and ease of assembly—from the robust, CNC-machined aluminum body and the 25:100 harmonic drive reducer, to the 3D-printed protective enclosures for the stepper motors and controllers. Each stage has undergone rigorous testing and pointing model calibration to achieve sub-arcsecond tracking accuracy.