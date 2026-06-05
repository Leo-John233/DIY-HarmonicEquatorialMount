<div align="center">
  <a href="./Firmware_Setup.md"><img src="https://img.shields.io/badge/简体中文-4285F4.svg?style=flat&logo=googletranslate&logoColor=white" alt="简体中文"></a>
  <a href="./Firmware_Setup.EN.md"><img src="https://img.shields.io/badge/English-4285F4.svg?style=flat&logo=googletranslate&logoColor=white" alt="English"></a>
</div>

The harmonic equatorial mount uses the **ESP32-WROOM-32E16N (16MB flash version)** as its main controller. This high-capacity core can easily run complex [OnStep / OnStepX](https://onstep.groups.io/g/main/wiki) firmware.

## ⚙️ Compilation Environment and Motherboard Settings (Critical)

Since we are using the 16MB version of the ESP32, **do not use the default 4MB compilation settings**, otherwise it will cause firmware compilation failure or waste a lot of storage space. The following is the official recommended configuration process based on the Arduino IDE (also applicable to PlatformIO users modifying `platformio.ini`):

1. **Prepare the environment:** Download the latest version of the Arduino IDE and add the ESP32 board manager URL to "Preferences". Install the latest version of the ESP32 support package (version 2.0.0 is recommended for best compatibility).

2. **Select the board:** In `Tools` -> `Board`, select **ESP32 Dev Module**.

3. **Configure key parameters:** (Strictly follow these settings):

* **Flash Size:** Select `16MB (128Mb)`

* **Partition Scheme:** Select a scheme optimized for OnStep, such as `16M Flash (3MB APP/9.9MB FATFS)` or `Huge APP`. This ensures sufficient space to store OnStep's large core codebase.

* **PSRAM**: If your module has PSRAM, please set it to `Enabled`; for standard 32E modules, please keep it `Disabled`.

* **Upload Speed**: `921600` (can be reduced to `460800` if unstable).

## 📝 Config.h Core Parameter Adaptation

Before the actual flashing, you need to modify `Config.h` in the OnStep source code to accurately adapt to the hardware. Refer to `Config-5160.h` and `Config-2209.h` templates.

**💡: If your replicated mechanical and structural design (including motor and harmonic reducer specifications) is exactly the same as this project, the relevant parameters can be directly copied and flashed.**

* **Pin Mapping**: Find and enable the pin definition scheme that matches this custom PCB, ensuring correct mapping of communication traces, stepper pulses (STEP/DIR), and limit switches.

* **Steps per Degree Configuration**:

* Core calculation formula: `(Physical steps per motor revolution × Subdivision × Total reduction ratio) / 360`, to match a standard 1.8° stepper motor (200 steps per revolution) and a 1:100 reduction ratio of 25... Taking a harmonic reducer as an example, the specific values ​​depend on the driver mode you are using:

* **Mode A: TMC5160 SPI Smart Mode (Recommended 🚀)**:

* **Driver Model**: Strictly define the right ascension (RA) and declination (Dec) axes as `TMC5160_SPI`.

* **Microstepping Setting**: It is recommended to set it to **64** or **128** (TMC will automatically interpolate to 256 microsteps at the underlying level to ensure extremely smooth star stream tracing).

* **Parameter Copies**: If using 64 microsteps, the calculation is `(200 × 64 × 100) / 360 ≈ 3555.556`. Please fill in `3555.556` in `AXIS1_STEPS_PER_DEGREE` and `AXIS2_STEPS_PER_DEGREE`

* **Mode B: Traditional STEP/DIR Mode (Backward Compatible 🔌)**:

* **Driver Model**: If SPI is disconnected via jumper and using A4988, DRV8825, or standalone TMC, please change the driver model to the corresponding model (e.g., `A4988` or `GENERIC`).

* **Strict Microstepping Match**: ⚠️ The `AXIS_MICROSTEPS` in the firmware **must be strictly equal to** the physical microstepping value set by the motherboard jumper cap. The firmware cannot dynamically change the microstepping in this mode.

* **Parameter Copying**: If the physical jumper is set to 32 microsteps, the calculation is `(200 × 32 × 100) / 360 ≈ 1777.778`. Please fill in this in the corresponding step parameter.

* **⚡ Current Setting (Manual Adjustment)**: In traditional mode, OnStep cannot control current via software. You must use a multimeter to measure the current while the motherboard is powered on, and extremely slowly rotate the potentiometer on the driver module with a ceramic Phillips screwdriver to set the VREF reference voltage. **Please be extremely careful to prevent short circuits during operation!**

### 🚀 Flashing Firmware and Uploading Web UI

A complete installation of OnStep on the ESP32 platform consists of **two separate steps** (firmware + file system), both of which are essential. The following flashing process uses the TMC5160 driver as an example:

## Step 1: Uploading OnStep Core Firmware
1. Connect the main control board using a high-quality data cable. System communication relies on the onboard CP2102GMR chip. Please ensure that the official Silicon Labs driver is installed.

2. In the IDE, select the corresponding COM port and click "Upload".

3. First, disable the SPI communication mode in `Config.h` to prevent errors (this PCB design uses a shared I/O port) to prepare for flashing the ESP07S.

* *Note: If the terminal displays a `Connecting...` timeout, please press and hold the `BOOT` button on the PCB until the progress bar appears. *

## Step Two: Uploading the Smart Web Server
Controlling OnStep's behavior via Wi-Fi relies on web page files stored in the ESP07S flash memory. Due to the compact PCB design, there is no dedicated flashing channel for the ESP07S; therefore, the ESP32 must be used as a forwarding bridge to flash the firmware.

* *Note: When flashing the ESP32, the `#define SERIAL_B_ESP_FLASHING` function in `Config.h` must be enabled, and the corresponding RST pin on the PCB must be connected with a jumper cap.

---

## 📡 ESP32 Pass-through Flashing Guide for ESP07s (SmartWebServer)

Please strictly follow the following order; any mistake will result in flashing failure. If the replica's structural design is exactly the same, the parameters can be directly copied for flashing.

**🛠️ Phase One: Preparation**

To use command-line tools for flashing, we cannot directly upload the source code. We must first generate a firmware file in `.bin` format and prepare the corresponding flashing tool.

**1. Prepare the WiFi Firmware File (.bin)**

1. Open the ESP8266 WiFi (SmartWebServer) source code in the Arduino IDE.

2. In **Tools -> Board**, select **LOLIN (WeMos) D1 R2 & mini**.

3. Click the menu bar: **Sketch -> Export Compiled Binary**.

4. Locate the generated `.bin` file in the source code folder (choose the shortest filename, such as `SmartWebServer.bin`). 5. Copy it to the root directory of drive D and rename it to something simple, such as `wifi.bin`

**2. Prepare the flashing tool (`esptool`)**

You need `esptool.py` in your Python environment or the compiled standalone program `esptool.exe`

* **If you have Python installed**: Open the CMD command prompt and type `pip install esptool` to complete the installation.

* **If you don't have Python installed**: Search for `esptool.exe` in the Arduino installation directory and copy it to the root directory of drive D for later use.

**🚀 Second Stage: Operation Process**

**Step 1: Establish a Pass-through Channel**

1. Connect the main control board containing the ESP32 to the computer via USB.

2. Open the Arduino's **Serial Monitor**.

3. Enter `:ESPFLASH#` in the send box and click send.

4. **Critical Check**: At this point, OnStep will automatically pull IO26 (WiFi) low according to the predefined pins. IO0), then pull IO2 low and then high (WiFi RST). If this step is successful, the ESP07s should now be in the "waiting to flash" state.

5. 🛑 **Turn off the serial monitor! **(This step is extremely important; the COM port must be released for esptool to use in the next step)**

**Step 2: Execute the "No Reset" Flash Command**

Open your computer's CMD (Command Prompt). Using the tools you have prepared, enter the following commands (assuming the file is on drive D and the port is `COM7`, please modify according to your actual situation):

**Option A: Use `esptool.exe` (Windows standalone program)**
First, type `D:` and press Enter (switch to drive D), then execute:
```bash
esptool.exe -p COM7 -b 115200 --before no_reset write_flash 0x00000 wifi.bin
```

**Option B: Use Python's `esptool.py`**
Execute directly in CMD:
```bash
python -m esptool -p COM7 -b 115200 --before no_reset write_flash 0x00000 D:\wifi.bin
``` 
* *Note: Remove the jumper cap after flashing.

## Step 3: Re-upload OnStep Core Firmware

1. Enable SPI communication mode in `Config.h` and disable `#define SERIAL_B_ESP_FLASHING`.

2. Select the corresponding COM port in the IDE and click "Upload" to re-flash.

3. After completing the above steps, the flashing is successful and the firmware is ready for use.

---

### 🔍 Parameter Details

* **`-p COM7`**: Specifies the hardware port to connect to.

* **`-b 115200`**: **Must use low speed!** **Software pass-through cannot handle high baud rates such as 921600; the main controller's software serial port forwarding is prone to packet loss.**

**`--before no_reset`:** This is a core parameter that tells the computer to "send data directly and do not attempt to restart the ESP32." This ensures the ESP32 remains in pass-through mode and obediently forwards data to the ESP07s.

* *(If using PlatformIO, please click the ant icon on the left and run the `Upload File System image` task.)*

> ⚠️

**Initial Startup and Security Verification:** After flashing, the main controller will create a Wi-Fi hotspot named `OnStep` (default password is usually `password`) upon first boot. After connecting, enter `192.168.0.1` in your browser to access the control panel. **Strongly recommended:** Perform an no-load test before mounting an expensive telescope! Carefully verify the current settings of the TMC5160 driver, the direction of motor rotation, and the trigger status of the limit switches to ensure that the high-torque harmonic reducer will not damage the mechanical structure.