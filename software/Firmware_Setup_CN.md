<div align="center">
  <a href="./Firmware_Setup_CN.md"><img src="https://img.shields.io/badge/简体中文-4285F4.svg?style=flat&logo=googletranslate&logoColor=white" alt="简体中文"></a>
  <a href="./Firmware_Setup_EN.md"><img src="https://img.shields.io/badge/English-4285F4.svg?style=flat&logo=googletranslate&logoColor=white" alt="English"></a>
</div>

谐波赤道仪主控采用 **ESP32-WROOM-32E16N（16MB 闪存版）** 这款大容量核心能够稳定运行复杂的 [OnStep / OnStepX](https://onstep.groups.io/g/main/wiki) 固件

## ⚙️ 编译环境与主板设置（至关重要）

由于本项目使用的是 16MB 版本 ESP32，**请不要使用默认的 4MB 编译设置**，否则可能导致固件编译失败，或无法充分利用可用闪存空间以下流程基于 Arduino IDE，同样适用于需要修改 `platformio.ini` 的 PlatformIO 用户

1. **准备环境**：下载最新版 Arduino IDE，在“首选项”中添加 ESP32 开发板管理器网址，并安装 ESP32 支持包（建议使用 2.0.0 版本以获得较好的兼容性）
2. **选择主板**：在 `工具 (Tools)` -> `开发板 (Board)` 中选择 **ESP32 Dev Module**
3. **关键参数配置**（请严格按照以下设置）：
   * **Flash Size（闪存大小）**：选择 `16MB (128Mb)`
   * **Partition Scheme（分区方案）**：选择适合 OnStep 的方案，例如 `16M Flash (3MB APP/9.9MB FATFS)` 或 `Huge APP`，以确保有足够空间存放 OnStep 的核心代码
   * **PSRAM**：如果模块带有 PSRAM，请设为 `Enabled`；标准 32E 模块请保持 `Disabled`
   * **Upload Speed（上传速度）**：选择 `921600`；如遇上传不稳定，可降至 `460800`

## 📝 Config.h 核心参数适配

正式刷写前，需要在 OnStep 源码中修改 `Config.h`，使其精确适配当前硬件可参考模板文件 `Config-5160.h` 和 `Config-2209.h`

**💡 提示：如果您复刻的机械结构与本项目完全一致，包括电机和谐波减速器规格，相关参数可以直接复制使用**

* **引脚映射 (Pinmap)**：找到并启用与本定制 PCB 匹配的引脚定义方案，确保通讯走线、步进脉冲接口（STEP/DIR）以及限位开关等接口映射正确
* **每度步数配置 (Steps per Degree)**：
  * 核心计算公式：`(电机每圈物理步数 × 细分数 × 总减速比) / 360`
  * 以标准 1.8° 步进电机（每圈 200 步）和 1:100 的 25 规格谐波减速器为例，具体数值取决于所使用的驱动器模式

* **模式 A：TMC5160 SPI 智能模式（推荐 🚀）**：
  * **驱动器型号**：将赤经轴（RA）和赤纬轴（Dec）严格定义为 `TMC5160_SPI`
  * **细分设置**：建议设为 **64** 或 **128**TMC 会在底层自动插值到 256 微步，以获得更平滑的恒星跟踪效果
  * **参数示例**：若使用 64 细分，计算结果为 `(200 × 64 × 100) / 360 ≈ 3555.556`请将 `3555.556` 填入 `AXIS1_STEPS_PER_DEGREE` 和 `AXIS2_STEPS_PER_DEGREE`

* **模式 B：传统 STEP/DIR 模式（向下兼容 🔌）**：
  * **驱动器型号**：如果通过跳线断开 SPI，并使用 A4988、DRV8825 或独立模式的 TMC 驱动器，请将驱动型号修改为对应类型，例如 `A4988` 或 `GENERIC`
  * **细分严格匹配**：⚠️ 固件中的 `AXIS_MICROSTEPS` **必须严格等于**主板跳线帽设定的物理细分值传统 STEP/DIR 模式下，固件无法动态修改细分
  * **参数示例**：若物理跳线设为 32 细分，计算结果为 `(200 × 32 × 100) / 360 ≈ 1777.778`请将该数值填入对应的步数参数
  * **⚡ 电流设置（手动调节）**：传统模式下，OnStep 无法通过软件控制电流请在主板通电状态下使用万用表测量，并使用陶瓷十字螺丝刀极其缓慢地旋转驱动器模块上的电位器，以设定 VREF 参考电压**操作时务必小心，避免短路！**

### 🚀 刷写固件与上传 Web UI

OnStep 在 ESP32 平台上的完整安装包含**两个独立步骤**：上传核心固件和上传文件系统两者缺一不可以下流程以使用 TMC5160 驱动为例

## 第一步：上传 OnStep 核心固件

1. 使用优质数据线连接主控板系统通讯依赖板载 CP2102GMR 芯片，请确保已安装 Silicon Labs 官方驱动
2. 在 IDE 中选择对应的 COM 端口，点击“上传 (Upload)”
3. 先在 `Config.h` 中关闭 SPI 通讯模式，以避免因本 PCB 共用 I/O 口而导致报错，并为后续刷写 ESP07S 做准备
   * *注：如果终端出现 `Connecting...` 超时，请按住 PCB 上的 `BOOT` 按钮，直到上传进度条出现*

## 第二步：上传 Smart Web Server

通过 Wi-Fi 控制 OnStep 依赖存储在 ESP07S 闪存中的网页文件由于 PCB 设计紧凑，没有为 ESP07S 预留独立刷写通道，因此需要将 ESP32 作为转发桥梁来完成 ESP07S 固件刷写

* *注：刷写 ESP32 时，需要在 `Config.h` 中打开 `#define SERIAL_B_ESP_FLASHING` 功能，关闭 SPI 通讯模式（共用io若不先关闭会导致编译报错），并使用跳线帽连接 PCB 上对应的 RST 针脚*

---

## 📡 ESP32 透传刷写 ESP07S（SmartWebServer）指南

请严格按照以下顺序操作任何步骤错误都可能导致刷写失败如果复刻结构与本项目完全一致，相关参数可以直接复制使用

**🛠️ 第一阶段：准备工作**

为了使用命令行工具进行刷写，不能直接上传源码，需要先生成 `.bin` 格式的固件文件，并准备对应的刷写工具

**1. 准备 WiFi 固件文件 (`.bin`)**

1. 在 Arduino IDE 中打开 ESP8266 WiFi（SmartWebServer）源码
2. 在 **工具 (Tools) -> 开发板 (Board)** 中选择 **`LOLIN (WeMos) D1 R2 & mini`**
3. 点击菜单栏：**项目 (Sketch) -> 导出已编译的二进制文件 (Export Compiled Binary)**
4. 在源码文件夹中找到刚刚生成的 `.bin` 文件，选择文件名最短的版本，例如 `SmartWebServer.bin`
5. 将该文件复制到 `D 盘` 根目录，并改成简单文件名，例如 `wifi.bin`

**2. 准备刷写工具 (`esptool`)**

需要准备 Python 环境下的 `esptool.py`，或编译好的独立程序 `esptool.exe`

* **如果已经安装 Python**：打开 CMD 命令提示符，输入 `pip install esptool` 即可完成安装
* **如果没有安装 Python**：在 Arduino 安装目录中搜索 `esptool.exe`，并将其复制到 `D 盘` 根目录备用

**🚀 第二阶段：操作流程**

**第一步：建立透传通道**

1. 将包含 ESP32 的主控板通过 USB 连接至电脑
2. 打开 Arduino 的 **串口监视器 (Serial Monitor)**
3. 在发送框输入 `:ESPFLASH#`，然后点击发送
4. **关键检查**：此时 OnStep 会根据预先定义的引脚自动拉低 IO26（WiFi IO0），随后拉低再拉高 IO2（WiFi RST）如果这一步成功，ESP07S 应已进入“等待刷写”状态
5. 🛑 **关闭串口监视器！** 这一步极其重要，必须释放 COM 口供下一步 `esptool` 使用

**第二步：执行“无复位”刷写指令**

打开电脑的 CMD（命令提示符），根据准备的工具输入以下命令示例假设文件位于 D 盘，端口为 `COM7`，请按实际情况修改

**选项 A：使用 `esptool.exe`（Windows 独立程序）**

先输入 `D:` 并回车，切换到 D 盘，然后执行：

```bash
esptool.exe -p COM7 -b 115200 --before no_reset write_flash 0x00000 wifi.bin
```

**选项 B：使用 Python 的 `esptool.py`**

直接在 CMD 中执行：

```bash
python -m esptool -p COM7 -b 115200 --before no_reset write_flash 0x00000 D:\wifi.bin
```

* *注：刷写完成后即可取下跳线帽*

## 第三步：重新上传 OnStep 核心固件

1. 在 `Config.h` 中重新打开 SPI 通讯模式，并关闭 `#define SERIAL_B_ESP_FLASHING`
2. 在 IDE 中选择对应的 COM 端口，点击“上传 (Upload)”重新刷写
3. 完成以上步骤后，即可完成刷写并投入使用

---

### 🔍 参数详解

* **`-p COM7`**：指定连接硬件的端口
* **`-b 115200`**：**必须使用低速！** 软件透传无法承受 921600 等高速波特率，主控软串口转发时容易丢包
* **`--before no_reset`**：**核心参数** 它告诉电脑“直接发送数据，不要尝试重启 ESP32”，这样主控 ESP32 才能保持在透传模式，并将数据转发给 ESP07S
* *(如果使用 PlatformIO，请点击左侧蚂蚁图标，运行 `Upload File System image` 任务)*

> ⚠️
> **初始启动与安全校验：** 刷写完成后，主控板首次启动会建立一个名为 `OnStep` 的 Wi-Fi 热点（默认密码通常为 `password`）连接后，在浏览器中输入 `192.168.0.1` 即可进入控制面板**强烈建议：** 在挂载昂贵的望远镜之前，务必先进行空载测试，仔细验证 TMC5160 驱动器电流设置、电机运转方向，以及限位开关 (Limit Switches) 的触发状态，确保高扭矩谐波减速器不会对机械结构造成损坏
