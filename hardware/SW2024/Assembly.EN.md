<div align="center">
  <a href="./README.md"><img src="https://img.shields.io/badge/简体中文-4285F4.svg?style=flat&logo=googletranslate&logoColor=white" alt="简体中文"></a>
  <a href="./README.EN.md"><img src="https://img.shields.io/badge/English-4285F4.svg?style=flat&logo=googletranslate&logoColor=white" alt="English"></a>
</div>

# Dual 25-100 Harmonic Equatorial Mount Assembly Instructions

This manual, using real-life photos from the `pictures/Assembly process` directory, explains the complete assembly process step-by-step, from the mechanical structure to the electrical control system.

## 🛠️ Preparation and Bill of Materials (BOM)

Before starting assembly, please ensure you have the following core components and tools ready (refer to the BOM for specific hardware requirements):

* **Transmission Core**: 25-100 type harmonic reducer (1:100 reduction ratio)

* **Idler Gears**: 2GT 10mm wide 80-tooth idler gear and 2GT 10mm wide 19mm high 20-tooth idler gear

* **Motor and Protection**: Stepper motor, power-off brake (electromagnetic damping/holding brake) module

* **Main Control and Drive**: ESP32 + ESP07S combination, TMC5160 stepper motor drive module

* **Circuit Board**: Custom-designed main control PCB (supports 12V/24V) Power plane)

* **Wires and accessories**: **Power switch lead wire**, various types of hex socket screws, lubricant, screw glue

*Note: For the idler gear, you need to have certain machining capabilities to modify the finished idler gear according to the specifications in the drawing. If you cannot modify it, please modify the model yourself to adapt to standard parts on the market.*

---

## ⚙️ Stage 1: Mechanical Structure Assembly

### 1. Installation of Harmonic Reducer (RA/DEC Axis)
In this step, we will install the core component of the equatorial mount. Considering the stress requirements of the radial slow rotation load, please ensure that the flange contact surface is flat and fits snugly during installation.

1. Accurately align the reducer flange face with the screw holes of the aluminum plate, and try to make the center round hole of the aluminum plate coaxial with the hollow shaft of the reducer. Use hex socket screws and tighten them evenly in a diagonal sequence to ensure that the reducer housing is subjected to uniform force and avoid deformation caused by excessive tightness or vibration caused by excessive looseness.

2. Two of the screw holes require longer screws and do not need to be fitted with lock nuts for subsequent motor mount installation.

3. Four of the screw holes do not require screws for subsequent installation of the 3D-printed reducer protective cover.

4. Tighten the excess parts of the remaining core screw assembly (excluding those from step 2) into lock nuts to ensure a secure connection between the reducer and the aluminum plate that will not loosen due to vibration.

> 📷 *Reference Image:*

> ![Installing Harmonic Reducer](https://github.com/Leo-John233/DIY-Harmonic-Equatorial-Mount/blob/main/pictures/Assembly%20process/DSC%20(1).JPG "Installing Harmonic Reducer")

> ![Installing Harmonic Reducer](https://github.com/Leo-John233/DIY-Harmonic-Equatorial-Mount/blob/main/pictures/Assembly%20process/DSC%20(2).JPG) "Installing the Harmonic Reducer")

> ![Installing the Harmonic Reducer](https://github.com/Leo-John233/DIY-Harmonic-Equatorial-Mount/blob/main/pictures/Assembly%20process/DSC%20(3).JPG "Installing the Harmonic Reducer")

### 2. Assembly of the Transmission Part

1. Install the machined 80-gear 2GT idler wheel on the input shaft of the reducer, ensuring that the idler wheel is coaxial with the input shaft. Use hex socket screws and tighten them evenly in a diagonal sequence to ensure a firm connection that will not loosen due to vibration.

2. Install the 20-gear 2GT idler wheel on the output shaft of the stepper motor.

3. Install the stepper motor onto the motor mount using copper pillars.

4. **Power-off Brake Installation**: Replace the original motor screws at the tail end of the dual-output stepper motor used for the RA axis and install an electromagnetic brake module. This effectively prevents damage to the telescope's "head" due to the unbalanced weight when the equipment is unexpectedly powered off.

5. Beforehand, slip a 2GT 10mm wide synchronous belt onto the motor idler pulley to facilitate subsequent installation.

> 📷 *Reference Image:*

> ![RA Axis Motor and Brake Module Installation](https://github.com/Leo-John233/DIY-Harmonic-Equatorial-Mount/blob/main/pictures/Assembly%20process/DSC(15).jpg "RA Axis Motor and Brake Module Installation")

> ![RA Axis Motor and Brake Module Installation](https://github.com/Leo-John233/DIY-Harmonic-Equatorial-Mount/blob/main/pictures/Assembly%20process/DSC(16).jpg "RA Axis Motor and Brake Module Installation") ![DEC Axis Motor Installation](https://github.com/Leo-John233/DIY-Harmonic-Equatorial-Mount/blob/main/pictures/Assembly%20process/DSC(17).jpg "DEC Axis Motor Installation")

### 3. Assembly of RA Axis Latitude Seat Flange

1. Connect the RA axis flange to connecting shaft A and connecting shaft B using hex socket head cap screws.

2. During step 1, simultaneously assemble the push rod and push block structure, ensuring the push rod can slide smoothly within the push block, and that connecting shaft A and connecting shaft B can achieve relative movement through the push rod.

3. Install the assembled structure from steps 1 and 2 onto the flange of the harmonic reducer using hex socket head cap screws. Simultaneously install the 3D-printed reducer protective cover during the installation process.

> 📷 *Reference Image:*

> ![Assembly of Latitude Constellation](https://github.com/Leo-John233/DIY-Harmonic-Equatorial-Mount/blob/main/pictures/Assembly%20process/DSC%20(7).JPG "Assembly of Latitude Constellation")

> ![Assembly of Latitude Constellation](https://github.com/Leo-John233/DIY-Harmonic-Equatorial-Mount/blob/main/pictures/Assembly%20process/DSC%20(8).JPG "Assembly of Latitude Constellation")

> ![Assembly of Latitude Constellation](https://github.com/Leo-John233/DIY-Harmonic-Equatorial-Mount/blob/main/pictures/Assembly%20process/DSC%20(10).JPG "Assembly of Latitude Constellation")

### 4. Connection of motor, synchronous belt, and reducer, as well as installation of sensors and flanges:

1. Install the synchronous belt onto the pre-drilled holes on the motor mount using the longer screws pre-drilled on the aluminum plate in step 1. Tighten the synchronous belt and tighten the lock nuts on the screws to ensure that the synchronous belt does not slip or become excessively loose during transmission.

2. Adjust the height of the idler pulley on the motor output shaft appropriately so that it is level with the idler pulley on the reducer input shaft. This ensures even tension distribution of the synchronous belt and avoids excessive wear or reduced transmission efficiency due to non-parallelism.

3. After completing the above steps, manually rotate the large idler pulley to check if the synchronous belt runs smoothly and ensure there are no abnormal friction sounds or vibrations. This will help confirm the correct installation and adjustment of the transmission system.

4. After installation, it is recommended to spray an appropriate amount of silicone lubricant on the synchronous belt to reduce friction and abnormal noise and extend the service life of the synchronous belt.

5. Install the photoelectric sensor limit switches and origin switches on the RA/DEC shaft, ensuring that their positions can accurately detect the extreme and origin positions of the shaft for safety protection and precise positioning.

6. Install the 3D-printed reducer protective cover on the outside of the reducer, and fix it with the reserved screw holes and hex screws to ensure that the protective cover can effectively prevent dust and debris from entering the reducer, while not interfering with its normal operation. 7. Install the flange of the RA/DEC shaft, ensuring that it is coaxial with the shaft, and tighten it evenly with hex screws to ensure that the flange will not shift or loosen during operation. 📷 *Reference image:* ![Installation of RA shaft motor and brake module, sensor, flange](https://github.com/Leo-John233/DIY-Harmonic-Equatorial-Mount/blob/main/pictures/Assembly%20process/DSC%20(5).JPG "Installation of RA shaft motor and brake module, sensor, flange") ![DEC Axis Motor, Synchronous Belt, and Sensor Installation](https://github.com/Leo-John233/DIY-Harmonic-Equatorial-Mount/blob/main/pictures/Assembly%20process/DSC%20(9).JPG "DEC Axis Motor, Synchronous Belt, and Sensor Installation")

> ![DEC Axis Flange Installation](https://github.com/Leo-John233/DIY-Harmonic-Equatorial-Mount/blob/main/pictures/Assembly%20process/DSC%20(6).JPG "DEC Axis Flange Installation")

### 5. Housing Assembly

1. Connect the assembled RA and DEC axes mechanical structures using aluminum blocks, ensuring all aluminum plates are vertically and orthogonally installed without interference using a marble horizontal surface.

2. Apply an appropriate amount of threadlocker to the screw connections to ensure the stability and reliability of the mechanical connections during equipment operation.

> 📷 *Reference Image:*

> ![Shell Assembly](https://github.com/Leo-John233/DIY-Harmonic-Equatorial-Mount/blob/main/pictures/Assembly%20process/DSC%20(11).JPG "Shell Assembly")

---

## ⚡ Phase Two: Electrical Control Assembly and Wiring

### 1. PCB Baseboard and Driver Installation

1. Fix the completed PCB board inside the electrical control compartment using copper pillars. During installation, carefully check the 12V/24V high-current power supply plane on the back to ensure sufficient insulation distance between it and the metal casing.

2. Connect the TMC5160 driver module and ESP32 main control board to the corresponding female connectors in sequence. Pay attention to the pin orientation and do not reverse them.

> 📷 *Reference Images:*

> ![PCB and Driver Installation](https://github.com/Leo-John233/DIY-Harmonic-Equatorial-Mount/blob/main/pictures/Assembly%20process/DSC(18).jpg "PCB and Driver Installation")

> ![PCB and Driver Installation](https://github.com/Leo-John233/DIY-Harmonic-Equatorial-Mount/blob/main/pictures/Assembly%20process/DSC(19).jpg "PCB and Driver Installation")

### 2. Standard Wiring and Cable Management To ensure the absolute stability of the equipment in outdoor environments and under high current driving conditions, the wiring in this project has strict requirements:

1. The main power supply line must use standard **power switch leads** to connect to the PCB. **The use of any arbitrary "flying wires"** as a substitute for the power path is strictly prohibited.

2. Connect the control wires of the RA and DEC axis stepper motors to the corresponding terminals on the TMC5160.

3. Connect the control wires of the power-off protection system and tidy up all wiring harnesses to ensure that the cables do not become tangled or excessively stretched during the extreme rotation of the equatorial mount.

> 📷 *Reference Image:*

> ![Standard Power Leads and Internal Routing](https://github.com/Leo-John233/DIY-Harmonic-Equatorial-Mount/blob/main/pictures/Assembly%20process/DSC%20(12).JPG "Standard Power Leads and Internal Routing")

---

## 💻 Phase Three: Firmware Configuration and Initial Debugging

The hardware logic of the Dual 25 is designed for a standard OnStep environment.

1. **Firmware Selection:** The system runs **standard OnStep firmware** (⚠️Please note: OnStepX is **not applicable to this project; do not flash the wrong firmware version**).

2. **Communication Confirmation:** Ensure the ESP32 pin mapping matches your modified OnStep firmware, and confirm the ESP07S's network communication connection is normal.

3. **Power-On Self-Test:** Connect to power and test whether the electromagnetic brake can engage/release normally. Monitor the TMC5160's operating status and microstepping settings via the host computer.

> 📷 *Reference Image:*

> ![Electronic Control System Illumination and Online Testing](https://github.com/Leo-John233/DIY-Harmonic-Equatorial-Mount/blob/main/pictures/Assembly%20process/DSC%20(13).JPG "Electronic Control System Lighting and Online Testing")

> ![Electronic Control System Lighting and Online Testing](https://github.com/Leo-John233/DIY-Harmonic-Equatorial-Mount/blob/main/pictures/Assembly%20process/DSC%20(14).JPG "Electronic Control System Lighting and Online Testing")

---

## 🤝 Open Source License and Contributions

If you find any optimizable mechanical tolerances or better wiring solutions during the assembly process, please submit an Issue or Pull Request. This project uses the `GPLv3 + CERN-OHL-S` dual open source license. Thank you for your contribution to the open source astronomical hardware community!

