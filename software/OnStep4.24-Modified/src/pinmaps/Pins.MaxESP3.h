// -------------------------------------------------------------------------------------------------
// Pin map for OnStep MaxESP Version 3.x (ESP32S)
// 修改pinmap之后还需修改对应的Validate.MaxESP3.h文件中的对应引脚, 否者会提示引脚占用

#if defined(ESP32)

// The multi-purpose pins (Aux3..Aux8 can be analog pwm/dac if supported)
// 多用途引脚（如果支持，Aux3..Aux8可以是模拟pwm/dac）
#define Aux2                  2     // ESP8266 RST control, or MISO for Axis1&2, or Axis4 EN support
                                    // ESP8266 RST控制，或轴1和2的MISO或轴4的EN使能(原始值IO2)
#define Aux3                 21     // I2C SDA
#define Aux4                 22     // I2C SCL
#define Aux5                 25     // Home SW for Axis1(RA轴1原点开关)
#define Aux6                 15     // Home SW for Axis2(DEC轴2原点开关)
#define Aux7                 39     // Limit SW, PPS, etc.DEC(限位开关，PPS，等)
#define Aux8                 12     // OneWire, Status LED, Status2 LED, Reticle LED, Tone, etc.
                                    // OneWire、状态LED、状态2 LED、网格LED、音调等。

// Misc. pins
// 杂项引脚
#ifndef OneWirePin
  #define OneWirePin       Aux8     // Default Pin for OneWire bus(单线总线默认引脚)
#endif
#define AddonBootModePin     26     // ESP8266 GPIO0 (Dir2即IO26)(附加启动模式引脚)
#define AddonResetPin      Aux2     // ESP8266 RST(重置引脚)

// The PEC index sense is a logic level input, resets the PEC index on rising edge then waits for 60 seconds before allowing another reset
// PEC索引感测是一个逻辑电平输入，在上升沿重置PEC索引，然后等待60秒再允许另一次重置
#define PecPin              OFF     // 原始值IO36
#define AnalogPecPin        OFF     // PEC Sense, analog or digital (GPIO36)(PEC感测，模拟或数字(IO36)) 原始值A0

// The status LED is a two wire jumper with a 10k resistor in series to limit the current to the LED
// 状态指示灯是一个串联10k电阻器的双线跳线，用于限制流向指示灯的电流
#define LEDnegPin          Aux8     // Drain(LED负极引脚)
#define LEDneg2Pin         Aux8     // Drain(LED负极引脚)
#define ReticlePin         Aux8     // Drain(瞄准线引脚)

// For a piezo buzzer
// 用于压电蜂鸣器
#ifndef TonePin
  #define TonePin          Aux8     // Tone(蜂鸣器引脚)
#endif

// The PPS pin is a 3.3V logic input, OnStep measures time between rising edges and adjusts the internal sidereal clock frequency
// PPS引脚是一个3.3V逻辑输入，OnStep测量上升沿之间的时间并调整内部恒星时钟频率
#define PpsPin               36     // PPS time source, GPS for example
                                    // PPS时间源输入，例如GPS                                            
#define LimitPin           Aux7     // The limit switch sense is a logic level input normally pull high (2k resistor,) shorted to ground it stops gotos/tracking
                                    // 限位开关感测是一个逻辑电平输入，通常拉高（2k电阻器），对地短路，它会停止gotos/跟踪
// Axis1 RA/Azm step/dir driver
// 轴1 RA/步进/细分 驱动
#define Axis1_EN              4     // Enable(使能引脚)
#define Axis1_M0             13     // Microstep Mode 0 or SPI MOSI(微步模式0或SPI MOSI)
#define Axis1_M1             14     // Microstep Mode 1 or SPI SCK(微步模式1或SPI SCK)
#define Axis1_M2             23     // Microstep Mode 2 or SPI CS(微步模式2或SPI CS)
#define Axis1_M3           Aux2     // SPI MISO/Fault(SPI MISO/错误)
#define Axis1_STEP           18     // RA-Step(步进)
#define Axis1_DIR            19     // RA-Dir(细分)
#define Axis1_DECAY    Axis1_M2     // Decay mode(衰变模式)
#define Axis1_HOME         Aux5     // Sense home position(感知原点位置)

// Axis2 Dec/Alt step/dir driver
// 轴2 RA/步进/细分 驱动
#define Axis2_EN         SHARED     // Enable pin control shared with Axis1(和轴1共享使能引脚)
#define Axis2_M0             13     // Microstep Mode 0 or SPI MOSI(微步模式0或SPI MOSI)
#define Axis2_M1             14     // Microstep Mode 1 or SPI SCK(微步模式1或SPI SCk)
#define Axis2_M2              5     // Microstep Mode 2 or SPI CS(微步模式2或SPI CS)
#define Axis2_M3           Aux2     // SPI MISO/Fault(SPI MISO/错误)
#define Axis2_STEP           27     // DEC-Step(步进)
#define Axis2_DIR            26     // DEC-Dir(细分)
#define Axis2_DECAY    Axis2_M2     // Decay mode(衰变模式)
#define Axis2_HOME         Aux6     // Sense home position(感知原点位置)

// M0/M1 是共享的 MOSI/SCK 网络 SPI 驱动程序使用独立的 CS 引脚
// 使用TMC2209且在脉冲模式下必须将两个轴都保持在同一个微步模式
#define AXIS12_DRIVER_MODE_PINS_SHARED

// For rotator stepper driver
#define Axis3_EN            OFF     // No enable pin control (always enabled)
#define Axis3_STEP          OFF     // Step(原始值IO2)
#define Axis3_DIR           OFF     // Dir(原始值IO15)

// For focuser1 stepper driver
#define Axis4_EN            OFF     // Enable pin on Aux2 but can be turned OFF during validation
#define Axis4_STEP          OFF     // Step(原始值IO19)
#define Axis4_DIR           OFF     // Dir(原始值IO15)

// For focuser2 stepper driver
#define Axis5_EN            OFF     // No enable pin control (always enabled)
#define Axis5_STEP          OFF     // Step(原始值IO2)
#define Axis5_DIR           OFF     // Dir(原始值IO5)

// ST4 interface
#define ST4RAw               34     // ST4 RA- West
#define ST4DEs               32     // ST4 DE- South
#define ST4DEn               33     // ST4 DE+ North
#define ST4RAe               35     // ST4 RA+ East

#else
#error "Wrong processor for this configuration!"

#endif
