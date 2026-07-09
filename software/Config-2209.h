// ---------------------------------------------------------------------------------------------------------------------------------
// Configuration for OnStep

/*
 *          For more information on setting OnStep up see http://www.stellarjourney.com/index.php?r=site/equipment_onstep 
 *                      and join the OnStep Groups.io at https://groups.io/g/onstep
 * 
 *           *** Read the compiler warnings and errors, they are there to help guard against invalid configurations ***
*/

// ---------------------------------------------------------------------------------------------------------------------------------
// ADJUST THE FOLLOWING TO CONFIGURE YOUR CONTROLLER FEATURES ----------------------------------------------------------------------
// <-Req'd = always must set, <-Often = usually must set, Option = optional, Adjust = adjust as req'd, Infreq = infrequently changed

// 引脚映射(PINMAP) ------------------------------------------------- see https://onstep.groups.io/g/main/wiki/Configuration-Controller#PINMAP
//   *** See the matching Pins.xxx.h file for your setup (found by looking in src/pinmaps/Models.h) with detailed information ***
//   *** to be sure it matches your wiring.            *** USE AT YOUR OWN RISK ***                                           ***

// 参数名(Parameter Name)              Value   Default  Notes                                                                      Hint
#define PINMAP                    MaxESP3 //    OFF, Choose from: MksGenL2, MiniPCB2, MaxPCB2, MaxESP3, CNC3, STM32Blue,     <-Req'd
                                          //    关闭，从以下选项中选择：MksGenL2、MiniPCB2、MaxPCB2、MaxESP3、CNC3、STM32蓝色，<-需要
                                          //    MaxSTM3, FYSETC_S6_2, etc.  其他主板和更多信息，在Constants.h

// 串口通信通道(SERIAL PORT COMMAND CHANNELS) --------------------------- see https://onstep.groups.io/g/main/wiki/Configuration-Controller#SERIAL
#define SERIAL_A_BAUD_DEFAULT        9600 //   9600, n. Where n=9600,19200,57600,115200 (common baud rates.)                  Infreq
                                          //   9600，n.其中n=9600，19200，57600，115200（常见波特率）
#define SERIAL_B_BAUD_DEFAULT        9600 //   9600, n. See (src/HAL/) for your MCU Serial port # etc.                        Option
                                          //   9600, n. 查看 (src/HAL/) 确认你的MCU串口号等信息
#define SERIAL_B_ESP_FLASHING         ON  //    OFF, ON Upload ESP8266 WiFi firmware through SERIAL_B with :ESPFLASH# cmd.    Option
                                          //    OFF, ON 通过串口B给ESP8266 WiFi模块刷固件（使用 :ESPFLASH# 命令）
#define SERIAL_C_BAUD_DEFAULT         ON  //    OFF, n, ON for ESP32 Bluetooth.                                               Option
                                          //    OFF, n, 如果是ESP32蓝牙则设为ON
#define SERIAL_C_BLUETOOTH_NAME  "OnStep" // "On..", Bluetooth device name for ESP32.                                         Option
                                          //  ESP32的蓝牙设备名称

// 赤道仪类型(MOUNT TYPE) ----------------------------------------- see https://onstep.groups.io/g/main/wiki/Configuration-Controller#MOUNT_TYPE
#define MOUNT_TYPE                    GEM //    GEM, GEM for German Equatorial, FORK for Equatorial Fork, or ALTAZM          <-Req'd
                                          //    GEM, GEM=德式赤道仪, FORK=叉式赤道仪, ALTAZM=经纬仪
                                          //    Dobsonian etc. mounts. GEM Eq mounts perform meridian flips.
                                          //    Dobsonian (道布森) 等也算经纬仪, 注意：只有 GEM 模式会执行中天翻转

// 用户反馈(USER FEEDBACK) ----------------------------------- see https://onstep.groups.io/g/main/wiki/Configuration-Controller#USER_FEEDBACK
#define LED_STATUS                     ON //     ON, Blinks w/sidereal tracking otherwise steady on indicates activity.       Option
                                          //     ON, 恒星速跟踪时闪烁，常亮表示有其他活动（如GOTO）
#define LED_STATUS2                   OFF //    OFF, ON Blinks 1s interval w/PPS sync, steady for gotos, off if in standby.   Option
                                          //    OFF, ON 收到PPS同步信号闪烁1秒，GOTO时常亮，待机时灭
#define LED_RETICLE                   OFF //    OFF, n. Where n=0..255 (0..100%) activates feature sets default brightness.   Option
                                          //    OFF, n. n=0..255 (0..100%) 开启极轴镜照明功能并设置默认亮度
#define BUZZER                        OFF //    OFF, ON, n. Where n=100..6000 (Hz freq.) for piezo speaker. ON for buzzer.    Option
                                          //    OFF, ON, n. n=100..6000 (Hz频率) 适用于压电喇叭。如果是普通有源蜂鸣器填ON
#define BUZZER_STATE_DEFAULT          OFF //    OFF, ON Start with piezo buzzer/speaker enabled.                              Option
                                          //    OFF, ON 启动时默认开启蜂鸣器/喇叭声音

// 时间和地点(TIME AND LOCATION) ----------------------------------------- see https://onstep.groups.io/g/main/wiki/Configuration-Controller#TLS
#define TIME_LOCATION_SOURCE       DS3231 //    OFF, DS3231 (I2c,) DS3234 (Spi,) TEENSY (T3.2 internal,) or GPS source.       Option
                                          //    OFF, DS3231 (I2c接口), DS3234 (Spi接口), TEENSY (T3.2内置), 或 GPS
                                          //    Provides Date/Time, and if available, PPS & Lat/Long also./时间位置源可以向 OnStep 提供日期/时间，并可选择性地提供经纬度

// SENSORS ----------------------------------------------- see https://onstep.groups.io/g/main/wiki/Configuration-Controller#SENSORS
// 传感器
// * = also supports ON_PULLUP or ON_PULLDOWN to activate MCU internal resistors if present.
// * = 支持 ON_PULLUP 或 ON_PULLDOWN 以启用单片机内部的上拉/下拉电阻
#define WEATHER                       OFF //    OFF, BME280 (I2C 0x77,) BME280_0x76, BME280_SPI (see pinmap for CS.)          Option
                                          //         BMP280 (I2C 0x77,) BMP280_0x76, BMP280_SPI (see pinmap for CS.)
                                          //         BME280 or BMP280 for temperature, pressure.  BME280 for humidity also.
                                          //         BME280 或 BMP280 用于测量温度和压力。BME280 也可用于测量湿度。
                                          
#define TELESCOPE_TEMPERATURE         OFF //    OFF, DS1820, n. Where n is the DS1820 s/n for focuser temperature.            Adjust
                                          //    OFF, DS1820, n. n是DS1820的序列号，用于检测调焦器处的温度（做温补）

#define HOME_SENSE              ON_PULLUP //    OFF, ON*, ON_PULLUP、ON_PULLUND Automatically detect and use home switches. For GEM mode only.      Option
                                          //    OFF, ON*. 自动检测并使用原点开关（霍尔/光电等）, 仅限 GEM (德式) 模式
#define HOME_SENSE_STATE_AXIS1       HIGH //   HIGH, State when clockwise of home position, as seen from front. Rev. w/LOW.   Adjust
                                          //   HIGH, 从前面看，当轴位于原点顺时针方向时的电平状态。用LOW反转
#define HOME_SENSE_STATE_AXIS2        LOW //   HIGH, State when clockwise of home position, as seen from above. Rev. w/LOW.   Adjust
                                          //   HIGH, 从上面看，当轴位于原点顺时针方向时的电平状态。用LOW反转
                                          //   Signal state reverses when travel moves ccw past the home position.
                                          //   当转动越过原点逆时针方向时，信号状态会反转       

// 偏置零位配置(由于传感器感应区域宽度或传感器安装位置不在原点，可能需要设置偏置角度)                            
#define HOME_OFFSET_AXIS1             0   //  n.n (度), 赤经/方位轴的偏置角度（支持正负号改变方向，0为不偏置）
#define HOME_OFFSET_AXIS2             0   //  n.n (度), 赤纬/俯仰轴的偏置角度（支持正负号改变方向，0为不偏置）
#define HOME_OFFSET_RATE              7   //  7, n. 偏置阶段使用的速度档位                            
                                          //  可选值通常为 1..9 档。
                                          //  提示：7 = 48x 恒星速（推荐，精找速度），8 = 半最大速，9 = 全速（Goto速度）

#define LIMIT_SENSE             ON_PULLUP //    OFF, ON*, ON_PULLUP、ON_PULLUND limit sense switch close to Gnd stops gotos and/or tracking.         Option
                                          //    OFF, ON* 限位开关。闭合接地时停止GOTO或跟踪
#define LIMIT_SENSE_STATE             LOW //    LOW, For NO (normally open) switches, HIGH for NC (normally closed.)          Adjust
                                          //    LOW, 对应常开(NO)开关请使用 LOW 端子。如果使用多个此类开关，请将它们并联连接
                                          //    HIGH 对应常闭(NC)开关请使用 高电平 (HIGH) 。如果使用多个此类开关，请将它们串联起来

#define PEC_SENSE                     OFF //    OFF, ON*, n, sense digital OR n=0 to 1023 (0 to 3.3V or 5V) analog threshold. Option
                                          //    OFF, ON*, n, 数字信号 OR n=0到1023 (0到3.3V/5V) 模拟阈值
#define PEC_SENSE_STATE              HIGH //   HIGH, Senses the PEC signal rising edge or use LOW for falling edge.           Adjust
                                          //   HIGH, 检测PEC信号的上升沿，LOW 检测下降沿
                                          //         Ignored in ALTAZM mode.
                                          //         在 ALTAZM (经纬仪) 模式下忽略

#define PPS_SENSE                      ON //    OFF, ON* enables PPS (pulse per second,) senses signal rising edge.           Option
                                          //    OFF, ON* 启用PPS (每秒脉冲) 同步，检测信号上升沿
                                          //         Better tracking accuracy especially for Mega2560's w/ceramic resonator.
                                          //         能提高跟踪精度，特别是对于使用陶瓷晶振的 Mega2560 主板

// ST4 导星接口(ST4 INTERFACE) --------------------------------------------- see https://onstep.groups.io/g/main/wiki/Configuration-Controller#ST4
// *** It is up to you to verify the interface meets the electrical specifications of any connected device, use at your own risk ***
// ***由您验证接口是否符合任何连接设备的电气规格，使用风险由您自行承担
#define ST4_INTERFACE                 ON  //    OFF, ON, ON_PULLUP enables interface. <= 1X guides unless hand control mode.  Option
                                          //    OFF, ON, ON_PULLUP 启用接口。<= 1X 倍速导星（除非在手柄模式）
                                          //         During goto btn press: aborts slew or continue meridian flip pause home
                                          //         在GOTO按钮按下期间：中止转向 或 继续中天翻转暂停回原点
#define ST4_HAND_CONTROL              ON  //    OFF, ON for hand controller special features and SHC support.                 Option
                                          //    OFF, ON 开启简易手柄模式（支持SHC手柄）
                                          //         Hold [E]+[W] btns >2s: Guide rate   [E]-  [W]+  [N] trk on/off [S] sync
                                          //         长按 [东]+[西] >2秒: 调速模式   [东]减速  [西]加速  [北]跟踪开关 [南]同步Sync
                                          //         Hold [N]+[S] btns >2s: Usr cat item [E]-  [W]+  [N] goto [S] snd on/off
                                          //         长按 [北]+[南] >2秒: 用户目录   [东]上条  [西]下条  [北]GOTO [南]声音开关
#define ST4_HAND_CONTROL_FOCUSER      OFF //    OFF, ON alternate to abovse: Focuser move [E]f1 [W]f2 [N]-     [S]+            Option
                                          //    OFF, ON 上述功能的替代：调焦控制 [东]焦距1 [西]焦距2 [北]收  [南]伸

// 导星行为(GUIDING BEHAVIOUR) ------------------------------------------ see https://onstep.groups.io/g/main/wiki/Configuration-Mount#GUIDING
#define GUIDE_TIME_LIMIT                0 //      0, No guide time limit. Or n. Where n=1..120 second time limit guard.       Adjust
                                          //      0, 无导星时间限制。或 n=1..120 秒，防止信号卡死导致一直转
#define GUIDE_DISABLE_BACKLASH        OFF //    OFF, Disable backlash takeup during guiding at <= 1X                          Option
                                          //    OFF, 在 <= 1X 导星修正时不进行反向间隙补偿（防止振荡）
// 跟踪行为(TRACKING BEHAVIOUR) ---------------------------------------- see https://onstep.groups.io/g/main/wiki/Configuration-Mount#TRACKING
#define TRACK_AUTOSTART                ON //    OFF, ON Start with tracking enabled.                                          Option
                                          //    OFF, ON 上电后自动开始跟踪(修改为开机电机使能)
#define TRACK_REFRACTION_RATE_DEFAULT OFF //    OFF, ON Start w/atmospheric refract. compensation (RA axis/Eq mounts only.)   Option
                                          //    OFF, ON 启动时开启大气折射率补偿 (仅限RA轴/赤道仪模式)
#define TRACK_BACKLASH_RATE            25 //     25, n. Where n=2..50 (x sidereal rate) during backlash takeup.               Option
                                          //     25, n. n=2..50 (倍恒星速) 反向间隙补偿时的速度
                                          //         Too fast motors stall/gears slam or too slow and sluggish in backlash.
                                          //         太快电机可能会堵转/齿轮撞击，太慢则补偿滞后

// 转向行为(SLEWING BEHAVIOUR) ------------------------------------------ see https://onstep.groups.io/g/main/wiki/Configuration-Mount#SLEWING
#define SLEW_RATE_BASE_DESIRED        6.0 //    1.0, n. Desired slew rate in deg/sec. Adjustable at run-time from            <-Req'd
                                          //    1.0, n. 期望的 GOTO 速度 (度/秒)。运行时可在
                                          //         1/2 to 2x this rate, and as MCU performace considerations require.
                                          //         0.5倍 到 2倍 之间调整，也受限于MCU性能
#define SLEW_RATE_MEMORY               ON //    OFF, ON Remembers rates set across power cycles.                              Option
                                          //    OFF, ON 掉电保存速度设置
#define SLEW_ACCELERATION_DIST        5.0 //    5.0, n, (degrees.) Approx. distance for acceleration (and deceleration.)      Adjust
                                          //    5.0, n, (度). 加速（和减速）所需的近似距离
#define SLEW_RAPID_STOP_DIST          2.0 //    2.0, n, (degrees.) Approx. distance required to stop when a slew              Adjust
                                          //    2.0, n, (度). 中止转向或触发限位时的急停距离
                                          //         is aborted or a limit is exceeded.
                                          //         停止距离

// 墩侧/翻转行为(PIER SIDE BEHAVIOUR) -------------------------------------- see https://onstep.groups.io/g/main/wiki/Configuration---Mount#SYNCING
#define MFLIP_SKIP_HOME                ON //    OFF, ON Goto directly to the destination without visiting home position.      Option
                                          //    OFF, ON goto中天翻转时不经过原点，直接去目标位置
#define MFLIP_PAUSE_HOME_MEMORY       OFF //    OFF, ON Remember meridian flip pause at home setting across power cycles.     Option
                                          //    OFF, ON 掉电保存“中天翻转在原点暂停”的设置
#define MFLIP_AUTOMATIC_MEMORY        OFF //    OFF, ON Remember automatic meridian flip setting across power cycles.         Option
                                          //    OFF, ON 掉电保存“自动中天翻转”的设置

#define PIER_SIDE_SYNC_CHANGE_SIDES   OFF //    OFF, ON Allows sync to change pier side, for GEM mounts.                      Option
                                          //    OFF, ON 允许通过 Sync (同步) 命令改变当前记录的墩侧状态 (针对GEM)
#define PIER_SIDE_PREFERRED_DEFAULT  BEST //   BEST, Stays on current side if possible. EAST or WEST switch if possible.      Option
                                          //   BEST, 尽可能保持当前侧。EAST 或 WEST 尽可能切换到指定侧

// 停机/泊车行为(PARKING BEHAVIOUR) ------------------------------------------ see https://onstep.groups.io/g/main/wiki/Configuration-Mount#PARKING
#define STRICT_PARKING                 OFF//    OFF, ON Un-parking is only allowed if successfully parked.                    Option
                                          //    OFF, ON 只有成功 Park (停机) 后才允许 Un-park (恢复)

// 运动控制(MOTION CONTROL) ---------------------------------------------- see https://onstep.groups.io/g/main/wiki/Configuration-Mount#MOTION
#define STEP_WAVE_FORM             SQUARE // SQUARE, PULSE Step signal wave form faster rates. SQUARE best signal integrity.  Adjust
                                          // SQUARE, PULSE 高速时的脉冲波形。SQUARE (方波) 信号最稳

// 开机电机保持
#define MOTOR_HOLD_ON_BOOT            OFF //    OFF, ON 只使能驱动器，不启动 tracking，不建立 home 坐标

// 步进驱动器型号说明 (也可以看 ~/OnStep/src/sd_drivers/Models.h 获取更多型号): 
// A4988, DRV8825, LV8729, S109, SSS TMC2209*, TMC2130* **, 和 TMC5160* ***
// * = 加上 _QUIET 后缀 (例如 "TMC2130_QUIET") 表示开启 stealthChop 静音跟踪模式
// ** = SSS TMC2130 如果你想软件设置电流(mA)，要把Vref电位器调到2.5V，而不是像通常那样调Vref来定电流。
// *** = SSS TMC5160 必须在下面定义 AXISn_TMC_IRUN (IHOLD 等) 来设置电流。

// 轴 1：赤经 / 方位(AXIS1 RA/AZM)
// see https://onstep.groups.io/g/main/wiki/Configuration-Mount#AXIS1
#define AXIS1_STEPS_PER_DEGREE  14222.222 //       12800, n. 每度步数:                                          <-Req'd
                                          //         n = (电机步数 * 细分 * 总减速比) / 360.0
#define AXIS1_STEPS_PER_WORMROT     51200 //      12800, n. 蜗杆旋转一圈的步数 (仅用于 PEC 赤道仪模式)             <-Req'd
                                          //         n = (AXIS1_STEPS_PER_DEGREE * 360) / 最后一级减速比

#define AXIS1_DRIVER_MODEL        TMC2209 //    OFF, (见上文). 驱动器型号.                                        <-Often
#define AXIS1_DRIVER_MICROSTEPS       64  //    OFF, n. 跟踪时的细分模式.                                         <-Often
#define AXIS1_DRIVER_MICROSTEPS_GOTO  32  //    OFF, n. GOTO (高速转向) 时的细分模式 (通常降低细分防丢步)            Option
#define AXIS1_DRIVER_IHOLD            OFF //    OFF, n, (mA.) 静止时的电流。OFF 代表使用 IRUN 的一半                Option
#define AXIS1_DRIVER_IRUN             OFF //    OFF, n, (mA.) 跟踪时的电流。根据电机/驱动调整                       Option
#define AXIS1_DRIVER_IGOTO            OFF //    OFF, n, (mA.) GOTO时的电流。OFF 代表和 IRUN 一样.                  Option
#define AXIS1_DRIVER_REVERSE           ON //    OFF, ON 反转运动方向。或者你也可以把电机线反着接.                   <-Often
#define AXIS1_DRIVER_STATUS           OFF //    OFF, TMC_SPI, HIGH, LOW.  轮询驱动器状态/故障.                     Option

#define AXIS1_LIMIT_MIN              -110 //  -180, n. n= -90..-270 (度). 赤道仪模式下的最小“时角”.                             Adjust
                                          //        n. n=-180..-360 (度). 经纬仪模式下的最小方位角.
#define AXIS1_LIMIT_MAX               110 //   180, n. n=  90.. 270 (度). 赤道仪模式下的最大“时角”.                             Adjust
                                          //        n. n= 180.. 360 (度). 经纬仪模式下的最大方位角.

// 轴 2：赤纬 / 俯仰(AXIS2 DEC/AL)T
// see https://onstep.groups.io/g/main/wiki/Configuration-Mount#AXIS2
#define AXIS2_STEPS_PER_DEGREE  14222.222 //  同上，每度步数                                                       <-Req'd
                                          //         n = (stepper_steps * micro_steps * overall_gear_reduction)/360.0

#define AXIS2_DRIVER_MODEL        TMC2209 //    OFF, 同上.                                                        <-Often
#define AXIS2_DRIVER_MICROSTEPS       64  //    OFF, 跟踪细分.                                                    <-Often
#define AXIS2_DRIVER_MICROSTEPS_GOTO  32  //    OFF, GOTO细分.                                                    Option
#define AXIS2_DRIVER_IHOLD            OFF //    OFF, 静止电流.                                                     Option
#define AXIS2_DRIVER_IRUN             OFF //    OFF, 跟踪电流.                                                     Option
#define AXIS2_DRIVER_IGOTO            OFF //    OFF, GOTO电流.                                                     Option
#define AXIS2_DRIVER_POWER_DOWN       OFF //    OFF, ON 停止移动10秒后，或最后一次<=1x导星10分钟后，自动断电.          Option
#define AXIS2_DRIVER_REVERSE          OFF //    OFF, ON 反转方向.                                                  <-Often
#define AXIS2_DRIVER_STATUS           OFF //    OFF, TMC_SPI, HIGH, or LOW, 状态检测.                               Option
#define AXIS2_TANGENT_ARM             OFF //    OFF, ON +limit range below. Set cntr w/[Reset Home] Return cntr w/[Find Home] Infreq

#define AXIS2_LIMIT_MIN               -90 //    -90, n. n=-90..0 (度). 最小允许赤纬.                                 Infreq
#define AXIS2_LIMIT_MAX                90 //     90, n. n=0..90 (度). 最大允许赤纬.                                  Infreq

// AXIS3 ROTATOR
// see https://onstep.groups.io/g/main/wiki/Configuration-Rotator-and-Focusers#AXIS3
#define ROTATOR                       OFF //    OFF, ON 启用旋转器 (或经纬仪的场旋消除器)              Option
#define AXIS3_STEPS_PER_DEGREE       64.0 //   64.0, n. 旋转每度的步数.                               Adjust
                                          //         经纬仪消旋: n = (圆周像素数 * 2)/360, 最小值
#define AXIS3_SLEW_RATE_DESIRED       1.0 //    1.0, n, (度/秒) 最大速度，取决于处理器性能.                  Adjust

#define AXIS3_DRIVER_MODEL            OFF //    OFF, TMC2130, TMC5160. 除这两款外保持OFF.                                     Option
#define AXIS3_DRIVER_MICROSTEPS       OFF //    OFF, n. 跟踪细分,              针对 TMC2130, TMC5160.                         Option
#define AXIS3_DRIVER_IHOLD            OFF //    OFF, n, (mA.) 静止电流.                                                       Option
#define AXIS3_DRIVER_IRUN             OFF //    OFF, n, (mA.) 跟踪电流.                                                       Option
#define AXIS3_DRIVER_POWER_DOWN       OFF //    OFF, ON 静止时电机断电.                                                        Option
#define AXIS3_DRIVER_REVERSE          OFF //    OFF, ON 反转方向.                                                              Option

#define AXIS3_LIMIT_MIN              -180 //   -180, n. n=-360..0 (度.) 最小允许旋转角.                                        Infreq
#define AXIS3_LIMIT_MAX               180 //    180, n. n=0..360 (度.) 最大允许旋转角.                                         Infreq

// AXIS4 FOCUSER 1
// see https://onstep.groups.io/g/main/wiki/Configuration-Rotator-and-Focusers#AXIS4
#define FOCUSER1                      OFF //    OFF, ON to enable this focuser.                                               Option
#define AXIS4_STEPS_PER_MICRON        0.5 //    0.5, n. Steps per micrometer. Figure this out by testing or other means.      Adjust
#define AXIS4_SLEW_RATE_DESIRED       500 //    500, n, Where n=200..5000 (um/s.) Max microns/second. In DC mode, max pwr %   Adjust

#define AXIS4_DRIVER_MODEL            OFF //    OFF, TMC2130, TMC5160. Leave OFF for all drivers models except these.         Option
#define AXIS4_DRIVER_MICROSTEPS       OFF //    OFF, n. Microstep mode when tracking.                   For TMC2130, TMC5160. Option
#define AXIS4_DRIVER_IHOLD            OFF //    OFF, n, (mA.) Current standstill. OFF uses IRUN/2.0.                  "       Option
#define AXIS4_DRIVER_IRUN             OFF //    OFF, n, (mA.) Current tracking, appropriate for stepper/driver/etc.   "       Option
#define AXIS4_DRIVER_POWER_DOWN       OFF //    OFF, ON Powers off the motor at stand-still.                                  Option
#define AXIS4_DRIVER_REVERSE          OFF //    OFF, ON Reverses movement direction, or reverse wiring instead to correct.    Option
#define AXIS4_DRIVER_DC_MODE          OFF //    OFF, ON for pwm dc motor control on stepper driver outputs.                   Option

#define AXIS4_LIMIT_MIN_RATE           50 //     50, n. Where n=1..1000 (um/s.) Minimum microns/second. In DC mode, min pwr.  Adjust
#define AXIS4_LIMIT_MIN                0  //      0, n. Where n=0..500 (millimeters.) Minimum allowed position.               Adjust
#define AXIS4_LIMIT_MAX                80 //     50, n. Where n=0..500 (millimeters.) Maximum allowed position.               Adjust

// AXIS5 FOCUSER 2
// see https://onstep.groups.io/g/main/wiki/Configuration-Rotator-and-Focusers#AXIS5
#define FOCUSER2                      OFF //    OFF, ON to enable this focuser.                                               Option
#define AXIS5_STEPS_PER_MICRON        0.5 //    0.5, n. Steps per micrometer. Figure this out by testing or other means.      Adjust
#define AXIS5_SLEW_RATE_DESIRED       500 //    500, n, Where n=200..5000 (um/s.) Max microns/second. In DC mode, max pwr %   Adjust

#define AXIS5_DRIVER_MODEL            OFF //    OFF, TMC2130, TMC5160. Leave OFF for all drivers models except these.         Option
#define AXIS5_DRIVER_MICROSTEPS       OFF //    OFF, n. Microstep mode when tracking.                   For TMC2130, TMC5160. Option
#define AXIS5_DRIVER_IHOLD            OFF //    OFF, n, (mA.) Current standstill. OFF uses IRUN/2.0.                  "       Option
#define AXIS5_DRIVER_IRUN             OFF //    OFF, n, (mA.) Current tracking, appropriate for stepper/driver/etc.   "       Option
#define AXIS5_DRIVER_POWER_DOWN       OFF //    OFF, ON Powers off the motor at stand-still.                                  Option
#define AXIS5_DRIVER_REVERSE          OFF //    OFF, ON Reverses movement direction, or reverse wiring instead to correct.    Option
#define AXIS5_DRIVER_DC_MODE          OFF //    OFF, ON for pwm dc motor control on stepper driver outputs.                   Option

#define AXIS5_LIMIT_MIN_RATE          50  //     50, n. Where n=1..1000 (um/s.) Minimum microns/second. In DC mode, min pwr.  Adjust
#define AXIS5_LIMIT_MIN               0   //      0, n. Where n=0..500 (millimeters.) Minimum allowed position.               Adjust
#define AXIS5_LIMIT_MAX               50  //     50, n. Where n=0..500 (millimeters.) Maximum allowed position.               Adjust

// 辅助功能控制(AUXILIARY FEATURE CONTROL) ------------------------------ see https://onstep.groups.io/g/main/wiki/6-ConfigurationMaster#AUXILIARY
// For additional infrequently used _PURPOSE options see Constants.h "various auxillary features"
// 有关其他不常用的 _PURPOSE 选项，请参阅 Constants.h 中的“各种辅助功能”。
#define FEATURE_LIST_DS               OFF //    OFF, temporarily set ON to list ds18b20 and ds2413 device serial numbers.     Adjust
#define FEATURE1_NAME          "FEATURE1" // "FE..", Name of feature being controlled.                                        Adjust
#define FEATURE1_PURPOSE              OFF //    OFF, SWITCH, ANALOG_OUT, DEW_HEATER, etc.                                     Option
#define FEATURE1_TEMP                 OFF //    OFF, DS1820, n. Where n is the ds18b20 s/n for DEW_HEATER temperature.        Adjust
#define FEATURE1_PIN                  OFF //    OFF, AUX, DS2413, n. Where n is ds2413 s/n (gpio0) or n=0 to 255 (pin#.)      Adjust
#define FEATURE1_DEFAULT_VALUE        OFF //    OFF, ON, n. Where n=0..255 for ANALOG_OUT purpose.                            Adjust
#define FEATURE2_NAME          "FEATURE2" // "FE..", Name of feature being controlled.                                        Adjust
#define FEATURE2_PURPOSE              OFF //    OFF, SWITCH, ANALOG_OUT, DEW_HEATER, etc.                                     Option
#define FEATURE2_TEMP                 OFF //    OFF, DS1820, n. Where n is the ds18b20 s/n for DEW_HEATER temperature.        Adjust
#define FEATURE2_PIN                  OFF //    OFF, AUX, CHAIN (ds2413 gpio1,) n. Where n=0 to 255 (pin#.)                   Adjust
#define FEATURE2_DEFAULT_VALUE        OFF //    OFF, ON, n. Where n=0..255 for ANALOG_OUT purpose.                            Adjust
#define FEATURE3_NAME          "FEATURE3" // "FE..", Name of feature being controlled.                                        Adjust
#define FEATURE3_PURPOSE              OFF //    OFF, SWITCH, ANALOG_OUT, DEW_HEATER, etc.                                     Option
#define FEATURE3_TEMP                 OFF //    OFF, DS1820, n. Where n is the ds18b20 s/n for DEW_HEATER temperature.        Adjust
#define FEATURE3_PIN                  OFF //    OFF, AUX, DS2413, n. Where n is ds2413 s/n (gpio0) or n=0 to 255 (pin#.)      Adjust
#define FEATURE3_DEFAULT_VALUE        OFF //    OFF, ON, n. Where n=0..255 for ANALOG_OUT purpose.                            Adjust
#define FEATURE4_NAME          "FEATURE4" // "FE..", Name of feature being controlled.                                        Adjust
#define FEATURE4_PURPOSE              OFF //    OFF, SWITCH, ANALOG_OUT, DEW_HEATER, etc.                                     Option
#define FEATURE4_TEMP                 OFF //    OFF, DS1820, n. Where n is the ds18b20 s/n for DEW_HEATER temperature.        Adjust
#define FEATURE4_PIN                  OFF //    OFF, AUX, CHAIN (ds2413 gpio1,) n. Where n=0 to 255 (pin#.)                   Adjust
#define FEATURE4_DEFAULT_VALUE        OFF //    OFF, ON, n. Where n=0..255 for ANALOG_OUT purpose.                            Adjust
#define FEATURE5_NAME          "FEATURE5" // "FE..", Name of feature being controlled.                                        Adjust
#define FEATURE5_PURPOSE              OFF //    OFF, SWITCH, ANALOG_OUT, DEW_HEATER, etc.                                     Option
#define FEATURE5_TEMP                 OFF //    OFF, DS1820, n. Where n is the ds18b20 s/n for DEW_HEATER temperature.        Adjust
#define FEATURE5_PIN                  OFF //    OFF, AUX, DS2413, n. Where n is ds2413 s/n (gpio0) or n=0 to 255 (pin#.)      Adjust
#define FEATURE5_DEFAULT_VALUE        OFF //    OFF, ON, n. Where n=0..255 for ANALOG_OUT purpose.                            Adjust
#define FEATURE6_NAME          "FEATURE6" // "FE..", Name of feature being controlled.                                        Adjust
#define FEATURE6_PURPOSE              OFF //    OFF, SWITCH, ANALOG_OUT, DEW_HEATER, etc.                                     Option
#define FEATURE6_TEMP                 OFF //    OFF, DS1820, n. Where n is the ds18b20 s/n for DEW_HEATER temperature.        Adjust
#define FEATURE6_PIN                  OFF //    OFF, AUX, CHAIN (ds2413 gpio1,) n. Where n=0 to 255 (pin#.)                   Adjust
#define FEATURE6_DEFAULT_VALUE        OFF //    OFF, ON, n. Where n=0..255 for ANALOG_OUT purpose.                            Adjust
#define FEATURE7_NAME          "FEATURE7" // "FE..", Name of feature being controlled.                                        Adjust
#define FEATURE7_PURPOSE              OFF //    OFF, SWITCH, ANALOG_OUT, DEW_HEATER, etc.                                     Option
#define FEATURE7_TEMP                 OFF //    OFF, DS1820, n. Where n is the ds18b20 s/n for DEW_HEATER temperature.        Adjust
#define FEATURE7_PIN                  OFF //    OFF, AUX, DS2413, n. Where n is ds2413 s/n (gpio0) or n=0 to 255 (pin#.)      Adjust
#define FEATURE7_DEFAULT_VALUE        OFF //    OFF, ON, n. Where n=0..255 for ANALOG_OUT purpose.                            Adjust
#define FEATURE8_NAME          "FEATURE8" // "FE..", Name of feature being controlled.                                        Adjust
#define FEATURE8_PURPOSE              OFF //    OFF, SWITCH, ANALOG_OUT, DEW_HEATER, etc.                                     Option
#define FEATURE8_TEMP                 OFF //    OFF, DS1820, n. Where n is the ds18b20 s/n for DEW_HEATER temperature.        Adjust
#define FEATURE8_PIN                  OFF //    OFF, AUX, CHAIN (ds2413 gpio1,) n. Where n=0 to 255 (pin#.)                   Adjust
#define FEATURE8_DEFAULT_VALUE        OFF //    OFF, ON, n. Where n=0..255 for ANALOG_OUT purpose.                            Adjust

// THAT'S IT FOR USER CONFIGURATION!


// -------------------------------------------------------------------------------------------------------------------------
#define FileVersionConfig 4
