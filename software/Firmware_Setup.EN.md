<div align="center">
  <a href="./Firmware_Setup.md"><img src="https://img.shields.io/badge/简体中文-4285F4.svg?style=flat&logo=googletranslate&logoColor=white" alt="简体中文"></a>
  <a href="./Firmware_Setup.EN.md"><img src="https://img.shields.io/badge/English-4285F4.svg?style=flat&logo=googletranslate&logoColor=white" alt="English"></a>
</div>

The harmonic equatorial mount controller uses an **ESP32-WROOM-32E16N (16MB flash version)**. This high-capacity module can reliably run the complex [OnStep / OnStepX](https://onstep.groups.io/g/main/wiki) firmware.

## ⚙️ Compilation Environment and Board Settings (Critical)

Because this project uses the 16MB version of the ESP32, **do not use the default 4MB compilation settings**, otherwise firmware compilation may fail, or the available flash space may not be fully utilized. The following procedure is based on the Arduino IDE and also applies to PlatformIO users who need to modify `platformio.ini`.

1. **Prepare the environment**: Download the latest Arduino IDE, add the ESP32 board manager URL in “Preferences”, and install the ESP32 support package (version 2.0.0 is recommended for better compatibility)
2. **Select the board**: Select **ESP32 Dev Module** under `Tools` -> `Board`
3. **Configure the key parameters** (strictly follow the settings below):
   * **Flash Size**: Select `16MB (128Mb)`
   * **Partition Scheme**: Select a scheme suitable for OnStep, such as `16M Flash (3MB APP/9.9MB FATFS)` or `Huge APP`, to ensure there is enough space for the OnStep core code
   * **PSRAM**: If the module includes PSRAM, set it to `Enabled`; keep it `Disabled` for the standard 32E module
   * **Upload Speed**: Select `921600`; if uploading is unstable, reduce it to `460800`

## 📝 Config.h Core Parameter Configuration

Before flashing, modify `Config.h` in the OnStep source code so that it precisely matches the current hardware. Refer to the template files `Config-5160.h` and `Config-2209.h`.

**💡 Tip: If the mechanical structure you reproduced is identical to this project, including the motor and harmonic reducer specifications, the relevant parameters can be copied directly.**

* **Pinmap**: Find and enable the pin definition scheme that matches this custom PCB, ensuring that the communication wiring, step pulse interfaces (STEP/DIR), limit switches, and other interfaces are mapped correctly
* **Steps per Degree Configuration**:
  * Core calculation formula: `(Physical motor steps per revolution × Microsteps × Total reduction ratio) / 360`
  * Using a standard 1.8° stepper motor (200 steps per revolution) and a size 25 harmonic reducer with a 1:100 ratio as an example, the exact value depends on the driver mode used

* **Mode A: TMC5160 SPI Intelligent Mode (Recommended 🚀)**:
  * **Driver Model**: Define the right ascension axis (RA) and declination axis (Dec) strictly as `TMC5160_SPI`
  * **Microstep Setting**: **64** or **128** is recommended. The TMC automatically interpolates to 256 microsteps internally for smoother sidereal tracking
  * **Parameter Example**: With 64 microsteps, the calculation is `(200 × 64 × 100) / 360 ≈ 3555.556`. Enter `3555.556` for `AXIS1_STEPS_PER_DEGREE` and `AXIS2_STEPS_PER_DEGREE`

* **Mode B: Traditional STEP/DIR Mode (Backward Compatible 🔌)**:
  * **Driver Model**: If SPI is disconnected with a jumper and an A4988, DRV8825, or standalone-mode TMC driver is used, change the driver model to the corresponding type, such as `A4988` or `GENERIC`
  * **Strict Microstep Matching**: ⚠️ `AXIS_MICROSTEPS` in the firmware **must exactly match** the physical microstep value set by the motherboard jumpers. In traditional STEP/DIR mode, the firmware cannot dynamically change the microstep setting
  * **Parameter Example**: If the physical jumpers are set to 32 microsteps, the calculation is `(200 × 32 × 100) / 360 ≈ 1777.778`. Enter this value for the corresponding steps parameter
  * **⚡ Current Setting (Manual Adjustment)**: In traditional mode, OnStep cannot control the current through software. With the motherboard powered on, use a multimeter to take measurements, and use a ceramic Phillips screwdriver to turn the potentiometer on the driver module extremely slowly to set the VREF reference voltage. **Be extremely careful during operation to avoid a short circuit!**

### 🚀 Flashing the Firmware and Uploading the Web UI

A complete OnStep installation on the ESP32 platform contains **two independent steps**: uploading the core firmware and uploading the file system. Both are required. The following procedure uses a TMC5160 driver as an example.

## Step 1: Upload the OnStep Core Firmware

1. Connect the controller board with a high-quality data cable. System communication relies on the onboard CP2102GMR chip, so make sure the official Silicon Labs driver is installed
2. Select the corresponding COM port in the IDE and click “Upload”
3. First disable SPI communication mode in `Config.h` to avoid errors caused by the shared I/O pins on this PCB and to prepare for flashing the ESP07S
   * *Note: If the terminal reports a `Connecting...` timeout, hold down the `BOOT` button on the PCB until the upload progress bar appears*

## Step 2: Upload the Smart Web Server

Controlling OnStep through Wi-Fi depends on web files stored in the ESP07S flash memory. Because the PCB has a compact design and does not provide a separate flashing channel for the ESP07S, the ESP32 must be used as a forwarding bridge to flash the ESP07S firmware.

* *Note: When flashing the ESP32, enable the `#define SERIAL_B_ESP_FLASHING` function in `Config.h` and use a jumper cap to connect the corresponding RST pins on the PCB*

---

## 📡 Guide to Flashing the ESP07S (SmartWebServer) Through ESP32 Passthrough

Strictly follow the sequence below. Any incorrect step may cause flashing to fail. If the reproduced structure is identical to this project, the relevant parameters can be copied directly.

**🛠️ Phase 1: Preparation**

To use a command-line tool for flashing, the source code cannot be uploaded directly, so a firmware file in `.bin` format must first be generated and the corresponding flashing tool must be prepared.

**1. Prepare the WiFi Firmware File (`.bin`)**

1. Open the ESP8266 WiFi (SmartWebServer) source code in the Arduino IDE
2. Select **`LOLIN (WeMos) D1 R2 & mini`** under **Tools -> Board**
3. Click the menu bar: **Sketch -> Export Compiled Binary**
4. Locate the newly generated `.bin` file in the source code folder and select the version with the shortest filename, such as `SmartWebServer.bin`
5. Copy the file to the root directory of the `D drive` and rename it to a simple filename, such as `wifi.bin`

**2. Prepare the Flashing Tool (`esptool`)**

Prepare `esptool.py` in a Python environment or the compiled standalone program `esptool.exe`.

* **If Python is already installed**: Open the CMD command prompt and enter `pip install esptool` to complete the installation
* **If Python is not installed**: Search for `esptool.exe` in the Arduino installation directory and copy it to the root directory of the `D drive` for later use

**🚀 Phase 2: Operating Procedure**

**Step 1: Establish the Passthrough Channel**

1. Connect the controller board containing the ESP32 to the computer through USB
2. Open the Arduino **Serial Monitor**
3. Enter `:ESPFLASH#` in the send box, then click send
4. **Critical Check**: At this point, OnStep automatically pulls IO26 (WiFi IO0) low according to the predefined pins, then pulls IO2 (WiFi RST) low and high. If this step succeeds, the ESP07S should have entered the “waiting for flashing” state
5. 🛑 **Close the Serial Monitor!** This step is extremely important. The COM port must be released for `esptool` in the next step

**Step 2: Execute the “No Reset” Flashing Command**

Open CMD (Command Prompt) on the computer and enter the following command according to the prepared tool. The example assumes that the file is located on the D drive and the port is `COM7`, so modify them according to the actual situation.

**Option A: Use `esptool.exe` (Windows Standalone Program)**

First enter `D:` and press Enter to switch to the D drive, then execute:

```bash
esptool.exe -p COM7 -b 115200 --before no_reset write_flash 0x00000 wifi.bin
```

**Option B: Use Python `esptool.py`**

Execute directly in CMD:

```bash
python -m esptool -p COM7 -b 115200 --before no_reset write_flash 0x00000 D:\wifi.bin
```

* *Note: The jumper cap can be removed after flashing is complete*

## Step 3: Re-upload the OnStep Core Firmware

1. Re-enable SPI communication mode in `Config.h` and disable `#define SERIAL_B_ESP_FLASHING`
2. Select the corresponding COM port in the IDE and click “Upload” to flash it again
3. After completing the steps above, flashing is complete and the system is ready for use

---

### 🔍 Detailed Parameter Description

* **`-p COM7`**: Specifies the port connected to the hardware
* **`-b 115200`**: **A low speed must be used!** Software passthrough cannot handle high baud rates such as 921600, and packet loss can easily occur when the controller forwards data through the software serial port
* **`--before no_reset`**: **Core parameter.** It tells the computer to “send data directly without attempting to restart the ESP32”, allowing the controller ESP32 to remain in passthrough mode and forward the data to the ESP07S
* *(If using PlatformIO, click the ant icon on the left and run the `Upload File System image` task.)*

> ⚠️
> **Initial Startup and Safety Verification:** After flashing is complete, the controller board creates a Wi-Fi hotspot named `OnStep` on its first startup (the default password is usually `password`). After connecting, enter `192.168.0.1` in the browser to access the control panel. **Strongly Recommended:** Before mounting an expensive telescope, perform a no-load test first and carefully verify the TMC5160 driver current setting, motor rotation direction, and limit switch trigger status to ensure that the high-torque harmonic reducer does not damage the mechanical structure
