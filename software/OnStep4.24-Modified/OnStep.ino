/*
 * 标题          OnStep
 * 作者          Howard Dutton
 *
 * 版权所有 (C) 2012 至 2021 Howard Dutton
 *
 * 本程序为自由软件：您可以根据自由软件基金会发布的 GNU 通用公共许可证（GNU GPL）
 * 的条款重新分发和/或修改它，无论是许可证的第 3 版，还是（由您选择）任何更高版本。
 *
 * 本程序的发布是希望它能有用，但【没有任何担保】；
 * 甚至没有对【适销性】或【特定用途适用性】的暗示性担保。
 * 有关更多详细信息，请参阅 GNU 通用公共许可证。
 *
 * 您应该随本程序一起收到了一份 GNU 通用公共许可证的副本。
 * 如果没有，请参阅 <http://www.gnu.org/licenses/>。
 *
 * 描述:
 * 功能齐全的步进电机望远镜微控制器，适用于赤道仪和经纬仪支架，
 * 采用衍生自 LX200 的指令集。
 *
 * 作者: Howard Dutton
 * http://www.stellarjourney.com
 * hjd1964@gmail.com
 *
 * 修订历史和更新版本:
 * 请参阅 GitHub: https://github.com/hjd1964/OnStep
 *
 * 文档:
 * https://groups.io/g/onstep/wiki/home
 *
 * 讨论、提问等:
 * https://groups.io/g/onstep
 */

// 请使用 Config.h 文件根据您的需求来配置 OnStep
// 固件信息，这些信息由 ":GV?#" 命令返回

#define FirmwareDate          __DATE__
#define FirmwareVersionMajor  4
#define FirmwareVersionMinor  24      // 次版本号 0 到 99
#define FirmwareVersionPatch  "s"     // 补丁版本，例如主.次 补丁: 1.3c
#define FirmwareVersionConfig 3       // 内部使用，用于跟踪配置文件更改
#define FirmwareName          "On-Step"
#define FirmwareTime          __TIME__

#include "Constants.h"

// --- [修改开始] 新增限位锁死状态变量 ---
// 0=无锁死, 1=正向锁死(如East/North), -1=反向锁死(如West/South)
int Axis1_LimitLock = 0; 
int Axis2_LimitLock = 0;
unsigned long lastLimitTriggerTime = 0; // [新增] 用于非阻塞消抖计时
// GOTO 被限位/安全逻辑中断：用于阻止 MoveTo 把安全中断伪装成 GOTO done
bool gotoAbortedBySafety = false;
// 安全中断后坐标不再可信，必须重新回原点后才允许 GOTO
bool gotoRequiresHomeAfterAbort = false;
// 开机强制回零锁
bool systemHasHomed = false; 
// --- [修改结束] -----------------------

// 首次上传时，OnStep 会自动初始化 nv 存储器（EEPROM）中的一系列设置。
// 此选项会强制再次进行初始化。
// 将其改为 ON，上传 OnStep，nv 将被重置为默认值。
// 等待约 30 秒后，将其设置为 OFF 并再次上传。
// *** 重要提示：此选项不得一直设置为 true (ON)，否则会导致 EEPROM 或 FLASH 过度损耗 ***
#define NV_FACTORY_RESET OFF

// 在指定的 DebugSer 端口上启用额外的调试和/或状态消息
// 请注意，DebugSer 端口不能用于与 OnStep 的正常通信（如控制望远镜）
#define DEBUG OFF             // 默认=OFF。使用 "DEBUG ON" 仅显示后台错误，
                              // 使用 "DEBUG VERBOSE" 显示所有错误和状态消息。
#define DebugSer SerialA      // 默认=SerialA，或者例如 Serial4（始终为 9600 波特率）

#include <errno.h>
#include <math.h>

#include "src/sd_drivers/Models.h"
#include "Config.h"
#include "src/pinmaps/Models.h"
#include "src/HAL/HAL.h"
#include "Validate.h"

// 用于调试的辅助宏，减少打字量
#if DEBUG != OFF
  #define D(x)       DebugSer.print(x)
  #define DF(x)      DebugSer.print(F(x))
  #define DL(x)      DebugSer.println(x)
  #define DLF(x)     DebugSer.println(F(x))
#else
  #define D(x)
  #define DF(x)
  #define DL(x)
  #define DLF(x)
#endif
#if DEBUG == VERBOSE
  #define V(x)        DebugSer.print(x)
  #define VF(x)       DebugSer.print(F(x))
  #define VL(x)       DebugSer.println(x)
  #define VLF(x)      DebugSer.println(F(x))
#else
  #define V(x)
  #define VF(x)
  #define VL(x)
  #define VLF(x)
#endif
// ---------------------------------------------------------------------------------------------------

#include "src/lib/St4SerialMaster.h"
#include "src/lib/FPoint.h"
#include "src/lib/Heater.h"
#include "src/lib/Intervalometer.h"
#include "Globals.h"
#include "src/lib/Julian.h"
#include "src/lib/Misc.h"
#include "src/lib/Sound.h"
#include "src/lib/Coord.h"
#include "Align.h"
#include "src/lib/Library.h"
#include "src/lib/Command.h"
#include "src/lib/TLS.h"
#include "src/lib/Weather.h"
weather ambient;
#if SERIAL_B_ESP_FLASHING == ON || defined(AddonTriggerPin)
  #include "src/lib/flashAddon.h"
  flashAddon fa;
#endif

#if ROTATOR == ON
  #include "src/lib/Rotator.h"
  rotator rot;
#endif

#if FOCUSER1 == ON || FOCUSER2 == ON
  #include "src/lib/Focuser.h"
  #if FOCUSER1 == ON
    #if AXIS4_DRIVER_DC_MODE != OFF
      #include "src/lib/FocuserDC.h"
      focuserDC foc1;
    #else
      #include "src/lib/FocuserStepper.h"
      focuserStepper foc1;
    #endif
  #endif
  #if FOCUSER2 == ON
    #if AXIS5_DRIVER_DC_MODE != OFF
      #include "src/lib/FocuserDC.h"
      focuserDC foc2;
    #else
      #include "src/lib/FocuserStepper.h"
      focuserStepper foc2;
    #endif
  #endif
#endif

// 支持 SPI 模式下的 TMC2130、TMC5160 等步进电机驱动器
#if (AXIS1_DRIVER_MODEL == TMC_SPI && AXIS2_DRIVER_MODEL == TMC_SPI) || \
    (ROTATOR == ON && AXIS3_DRIVER_MODEL == TMC_SPI) || \
    (FOCUSER1 == ON && AXIS4_DRIVER_MODEL == TMC_SPI) || \
    (FOCUSER2 == ON && AXIS5_DRIVER_MODEL == TMC_SPI)
  #include "src/lib/SoftSPI.h"
  #include "src/lib/TMC_SPI.h"
  #if AXIS1_DRIVER_MODEL == TMC_SPI
    tmcSpiDriver tmcAxis1(Axis1_M2,Axis1_M1,(AXIS1_DRIVER_STATUS == TMC_SPI)?Axis1_M3:-1,Axis1_M0,AXIS1_DRIVER_SUBMODEL,AXIS1_DRIVER_RSENSE);
  #endif
  #if AXIS2_DRIVER_MODEL == TMC_SPI
    tmcSpiDriver tmcAxis2(Axis2_M2,Axis2_M1,(AXIS2_DRIVER_STATUS == TMC_SPI)?Axis2_M3:-1,Axis2_M0,AXIS2_DRIVER_SUBMODEL,AXIS2_DRIVER_RSENSE);
  #endif
  #if AXIS3_DRIVER_MODEL == TMC_SPI
    tmcSpiDriver tmcAxis3(Axis3_M2,Axis3_M1,-1,Axis3_M0,AXIS3_DRIVER_SUBMODEL,AXIS3_DRIVER_RSENSE);
  #endif
  #if AXIS4_DRIVER_MODEL == TMC_SPI
    tmcSpiDriver tmcAxis4(Axis4_M2,Axis4_M1,-1,Axis4_M0,AXIS4_DRIVER_SUBMODEL,AXIS4_DRIVER_RSENSE);
  #endif
  #if AXIS5_DRIVER_MODEL == TMC_SPI
    tmcSpiDriver tmcAxis5(Axis5_M2,Axis5_M1,-1,Axis5_M0,AXIS5_DRIVER_SUBMODEL,AXIS5_DRIVER_RSENSE);
  #endif
#endif

void setup() {
  initPre();
  // 初始化 ESP8266 插件（Addon）烧录器
#if SERIAL_B_ESP_FLASHING == ON
  fa.init(-1,AddonResetPin,AddonBootModePin);
#elif defined(AddonTriggerPin)
  fa.init(AddonTriggerPin,AddonResetPin,AddonBootModePin);
#endif

  // 等待半秒钟，让所有连接的设备启动完毕，然后再开始设置引脚。
  delay(10);//[修改] 缩短等待时间，原始值500
#if DEBUG != OFF
  // 提前初始化 USB 串口调试，以便在需要时可以使用 DebugSer.print() 进行调试。
  DebugSer.begin(9600);
  delay(5000); DebugSer.flush(); VLF(""); VLF("");
#endif

  // 打印欢迎信息/版本号
  VF("MSG: OnStep "); V(FirmwareVersionMajor); V("."); V(FirmwareVersionMinor); VL(FirmwareVersionPatch);
  VF("MSG: MCU =  "); VF(MCU_STR); V(", "); VF("Pinmap = "); VLF(PINMAP_STR);
  // 调用硬件特定初始化
  VLF("MSG: Init HAL");
  HAL_Initialize();

  VLF("MSG: Init serial");
  // 等待半秒钟，让串口缓冲区清空，然后再可能需要重启调试端口
  delay(10);// [修改2] 缩短串口缓冲等待，原始值500
  SerialA.begin(SERIAL_A_BAUD_DEFAULT);
#ifdef HAL_SERIAL_B_ENABLED
  #ifdef SERIAL_B_RX
    SerialB.begin(SERIAL_B_BAUD_DEFAULT, SERIAL_8N1, SERIAL_B_RX, SERIAL_B_TX);
  #else
    SerialB.begin(SERIAL_B_BAUD_DEFAULT);
  #endif
#endif
#ifdef HAL_SERIAL_C_ENABLED
  SerialC.begin(SERIAL_C_BAUD_DEFAULT);
#endif
#ifdef HAL_SERIAL_D_ENABLED
  #ifdef SERIAL_D_RX
    SerialD.begin(SERIAL_D_BAUD_DEFAULT, SERIAL_8N1, SERIAL_D_RX, SERIAL_D_TX);
  #else
    SerialD.begin(SERIAL_D_BAUD_DEFAULT);
  #endif
#endif
#ifdef HAL_SERIAL_E_ENABLED
  SerialE.begin(SERIAL_E_BAUD_DEFAULT);
#endif
#if TIME_LOCATION_SOURCE == GPS
  #ifdef SERIAL_GPS_RX
    SerialGPS.begin(SERIAL_GPS_BAUD, SERIAL_8N1, SERIAL_GPS_RX, SERIAL_GPS_TX);
  #else
    SerialGPS.begin(SERIAL_GPS_BAUD);
  #endif
#endif
#if ST4_HAND_CONTROL == ON && ST4_INTERFACE != OFF
  SerialST4.begin();
#endif

  // 再等待两秒钟，以确保串口完全上线
  delay(100);// [修改3] 缩短串口上线等待 (从2000改到100，兼容SPI初始化时间)，原始值2000
  // 根据 Config.h 和 PinMap.h 中的定义设置输入/输出引脚
  VLF("MSG: Init pins");
  initPins();

  // 准备好 TLS（时间地点源）（如果存在）
  VLF("MSG: Init TLS");
  if (!tls.init()) generalError=ERR_SITE_INIT;
  // 检查非易失性存储器 (NV Memory)
  VF("MSG: Start NV ");
  if (!nv.init()) {
    VLF("");
    SerialA.print("NV (EEPROM) failure!#\r\n");
    while (true) {
      delay(10);
      #ifdef HAL_SERIAL_TRANSMIT
        SerialA.transmit();
      #endif
    }
  }
  V(E2END+1);
  VLF(" Bytes");

  // 如果是首次启动，则将 EEPROM 设置为默认值
  initWriteNvValues();
  // 现在从 EEPROM 中读取已保存的值到变量中，以恢复我们之前的状态
  VLF("MSG: Read NV settings");
  initReadNvValues();

  // 设置某些变量的初始值
  VLF("MSG: Init startup settings");
  initStartupValues();
  initStartPosition();
  // 初始化天体对象库 (Object Library)
  VLF("MSG: Init library/catalogs");
  Lib.init();
  // 准备好导星功能
  VLF("MSG: Init guiding");
  initGuide();
  // 准备好天气监测设备
#ifdef ONEWIRE_DEVICES_PRESENT
  VLF("MSG: Init weather and 1-Wire");
#else
  VLF("MSG: Init weather");
#endif
  if (!ambient.init() && WEATHER_SUPRESS_ERRORS == OFF) generalError=ERR_WEATHER_INIT;
  // 设置辅助功能
#ifdef FEATURES_PRESENT
  VLF("MSG: Init auxiliary features");
  featuresInit();
#endif

  // 这会设置恒星时计时器和跟踪速率
  VLF("MSG: Init sidereal timer");
  siderealInterval=nv.readLong(EE_siderealInterval); // 一个恒星秒内的 16MHz 时钟数（这是根据实际处理器速度缩放的）
  if (siderealInterval < 14360682L || siderealInterval > 17551944L) { DF("ERR, setup(): bad NV siderealInterval ("); D(siderealInterval); DL(")");
  siderealInterval=masterSiderealInterval; }
  siderealRate=siderealInterval/stepsPerSecondAxis1;
  timerRateAxis1=siderealRate;
  timerRateAxis2=siderealRate;

  // 设置反向间隙补偿（Backlash）速率
  backlashTakeupRate=siderealRate/TRACK_BACKLASH_RATE;
  timerRateBacklashAxis1=siderealRate/TRACK_BACKLASH_RATE;
  timerRateBacklashAxis2=(siderealRate/TRACK_BACKLASH_RATE)*timerRateRatio;
  // 设置步进电机驱动模式
  VLF("MSG: Init motor timers");
  StepperModeTrackingInit();
  // 启动保持恒星时、驱动电机等的硬件定时器。
  setTrackingRate(DefaultTrackingRate);
  setDeltaTrackingRate();
  initStartTimers();
  // 跟踪自动启动逻辑
// 跟踪自动启动逻辑：只负责真正的 tracking autostart
#if TRACK_AUTOSTART == ON
  #if MOUNT_TYPE != ALTAZM

    if (!tls.active) {
      VLF("MSG: Tracking autostart - TLS/orientation unknown, limits disabled");
      setHome();
      safetyLimitsOn = false;
    } else {
      if (parkStatus != Parked) {
        VLF("MSG: Tracking autostart - TLS/orientation unknown, limits disabled");
        setHome();
        safetyLimitsOn = false;
      } else {
        VLF("MSG: Tracking autostart - assuming TLS/orientation are correct, limits enabled and automatic unpark");
        unPark(true);
      }
    }

    trackingState = TrackingSidereal;
    enableStepperDrivers();

  #else
    #warning "Tracking autostart ignored for MOUNT_TYPE ALTAZM"
  #endif

#else
  if (parkStatus == Parked) {
    VLF("MSG: Restoring parked telescope pointing state");
    unPark(false);
  }
#endif


// 独立的开机电机保持逻辑：只使能电机，不 tracking，不 setHome
#if MOTOR_HOLD_ON_BOOT == ON
  #if MOUNT_TYPE != ALTAZM

    VLF("MSG: Motor hold on boot - drivers enabled, tracking off, home required");

    // 开机后不认为已经真实回原点
    systemHasHomed = false;

    // 开机清理上一次安全中断残留
    gotoAbortedBySafety = false;
    gotoRequiresHomeAfterAbort = false;

    // 清理限位方向锁
    Axis1_LimitLock = 0;
    Axis2_LimitLock = 0;

    // 明确不 tracking
    trackingState = TrackingNone;
    lastTrackingState = TrackingNone;
    abortTrackingState = TrackingNone;

    // 电机使能，但不发跟踪脉冲
    enableStepperDrivers();

    // 使用 tracking 微步/电流保持，不切 GOTO 模式
    axis1DriverTrackingMode(false);
    axis2DriverTrackingMode(false);

    // 未真实回原点前，不启用坐标安全限制
    safetyLimitsOn = false;

  #endif
#endif

  // 如果存在，则启动旋转器 (Rotator)
#if ROTATOR == ON
  VLF("MSG: Init rotator");
  rot.init(Axis3_STEP,Axis3_DIR,Axis3_EN,EE_rotBaseAxis3,AXIS3_STEP_RATE_MAX,axis3Settings.stepsPerMeasure,axis3Settings.min,axis3Settings.max);
  if (axis3Settings.reverse == ON) rot.setReverseState(HIGH);
  rot.setDisableState(AXIS3_DRIVER_DISABLE);
  
  #if AXIS3_DRIVER_MODEL == TMC_SPI
    tmcAxis3.setup(AXIS3_DRIVER_INTPOL,AXIS3_DRIVER_DECAY_MODE,AXIS3_DRIVER_CODE,axis3Settings.IRUN,axis3Settings.IRUN);
    delay(150);
    tmcAxis3.setup(AXIS3_DRIVER_INTPOL,AXIS3_DRIVER_DECAY_MODE,AXIS3_DRIVER_CODE,axis3Settings.IRUN,axis3SettingsEx.IHOLD);
  #endif
  
  rot.powerDownActive(AXIS3_DRIVER_POWER_DOWN == ON);
#endif

  // 如果存在，则启动调焦器 (Focusers)
#if FOCUSER1 == ON
  VLF("MSG: Init focuser1");
  foc1.init(Axis4_STEP,Axis4_DIR,Axis4_EN,EE_focBaseAxis4,AXIS4_STEP_RATE_MAX,axis4Settings.stepsPerMeasure,axis4Settings.min*1000.0,axis4Settings.max*1000.0,AXIS4_LIMIT_MIN_RATE);
  if (AXIS4_DRIVER_DC_MODE != OFF) foc1.setPhase1();
  if (axis4Settings.reverse == ON) foc1.setReverseState(HIGH);
  foc1.setDisableState(AXIS4_DRIVER_DISABLE);

  #if AXIS4_DRIVER_MODEL == TMC_SPI
    tmcAxis4.setup(AXIS4_DRIVER_INTPOL,AXIS4_DRIVER_DECAY_MODE,AXIS4_DRIVER_CODE,axis4Settings.IRUN,axis4Settings.IRUN);
    delay(150);
    tmcAxis4.setup(AXIS4_DRIVER_INTPOL,AXIS4_DRIVER_DECAY_MODE,AXIS4_DRIVER_CODE,axis4Settings.IRUN,axis4SettingsEx.IHOLD);
  #endif

  foc1.powerDownActive(AXIS4_DRIVER_POWER_DOWN == ON, AXIS4_DRIVER_POWER_DOWN == STARTUP);
#endif

#if FOCUSER2 == ON
  VLF("MSG: Init focuser2");
  foc2.init(Axis5_STEP,Axis5_DIR,Axis5_EN,EE_focBaseAxis5,AXIS5_STEP_RATE_MAX,axis5Settings.stepsPerMeasure,axis5Settings.min*1000.0,axis5Settings.max*1000.0,AXIS5_LIMIT_MIN_RATE);
  if (AXIS5_DRIVER_DC_MODE != OFF) foc2.setPhase2();
  if (axis5Settings.reverse == ON) foc2.setReverseState(HIGH);
  foc2.setDisableState(AXIS5_DRIVER_DISABLE);

  #if AXIS5_DRIVER_MODEL == TMC_SPI
    tmcAxis5.setup(AXIS5_DRIVER_INTPOL,AXIS5_DRIVER_DECAY_MODE,AXIS5_DRIVER_CODE,axis5Settings.IRUN,axis5Settings.IRUN);
    delay(150);
    tmcAxis5.setup(AXIS5_DRIVER_INTPOL,AXIS5_DRIVER_DECAY_MODE,AXIS5_DRIVER_CODE,axis5Settings.IRUN,axis5SettingsEx.IHOLD);
  #endif

  foc2.powerDownActive(AXIS5_DRIVER_POWER_DOWN == ON, AXIS5_DRIVER_POWER_DOWN == STARTUP);
#endif

  // 最后清除通信通道
  VLF("MSG: Serial buffer flush");
  delay(10);// [修改4] 缩短最后的缓冲清除等待，原始值500
  SerialA.flush();
  while (SerialA.available()) SerialA.read();
#ifdef HAL_SERIAL_B_ENABLED
  SerialB.flush();
  while (SerialB.available()) SerialB.read();
#endif
#ifdef HAL_SERIAL_C_ENABLED
  SerialC.flush();
  while (SerialC.available()) SerialC.read();
#endif
#ifdef HAL_SERIAL_D_ENABLED
  SerialD.flush();
  while (SerialD.available()) SerialD.read();
#endif
#ifdef HAL_SERIAL_E_ENABLED
  SerialE.flush();
  while (SerialE.available()) SerialE.read();
#endif
  delay(500);
  // 准备计数器（用于在主循环中计时）
  cli(); siderealTimer=lst; guideSiderealTimer=lst; pecSiderealTimer=lst; sei();
  last_loop_micros=micros();
  VLF("MSG: OnStep is ready"); VL("");
}

void loop() {
  loop2();
  Align.model(0);
  // GTA 计算指向模型，这将在扩展处理期间调用 loop2()
}

void loop2() {
#if HOME_SENSE != OFF
  // =========================================================
  // 监听自动回原点 (Homing) 状态
  // =========================================================
  static bool wasHoming = false;
  if (isHoming()) {
      wasHoming = true; // 系统正在回原点
  } else if (wasHoming) {
      // 不在主循环里直接把 systemHasHomed 置 true。
      // 真实自动回零成功只由 Home.ino 的 FH_DONE 阶段确认，避免回零失败也误解锁。
      wasHoming = false;
  }
  // =========================================================
#endif

  // 导星 (GUIDING) -------------------------------------------------------------------------------------------
  ST4();
  if ((trackingState != TrackingMoveTo) && (parkStatus == NotParked)) guide();

#if HOME_SENSE != OFF
  // 自动回原点 (AUTOMATIC HOMING) ----------------------------------------------------------------------------------
  checkHome();
#endif

  // 1/100 秒定时任务 --------------------------------------------------------------------------------
  cli(); long lstNow=lst; sei();
  if (lstNow != siderealTimer) {
    siderealTimer=lstNow;

#ifdef ESP32
    timerSupervisor(true);
#endif
    
#if AXIS1_PEC == ON
    // 周期误差修正 (PEC)
    pec();
#endif

    // 恒星跟踪期间闪烁 LED
#if LED_STATUS == ON
    if (trackingState == TrackingSidereal) {
      if (siderealTimer%20L == 0L) { if (ledOn) { digitalWrite(LEDnegPin,HIGH);
      ledOn=false; } else { digitalWrite(LEDnegPin,LOW); ledOn=true; } }
    }
#endif

    // GOTO 期间保持恒星跟踪计算
    // 确保 GOTO 过程中目标位置依然随时间更新
    if (trackingState == TrackingMoveTo) {
      moveTo();
      if (lastTrackingState == TrackingSidereal) {
        origTargetAxis1.fixed+=fstepAxis1.fixed;
        origTargetAxis2.fixed+=fstepAxis2.fixed;
        // 中天翻转或同步期间不推进目标位置
        if (getInstrPierSide() == PierSideEast || getInstrPierSide() == PierSideWest) {
          cli();
          targetAxis1.fixed+=fstepAxis1.fixed;
          targetAxis2.fixed+=fstepAxis2.fixed;
          sei();
        }
      }
    }

    // 旋转器/调焦器，移动目标
#if ROTATOR == ON
    rot.poll(trackingState == TrackingSidereal);
#endif
#if FOCUSER1 == ON
    foc1.poll();
#endif
#if FOCUSER2 == ON
    foc2.poll();
#endif

    // 计算一些跟踪速率等
    if (lstNow%3 == 0) doFastAltCalc(false);
#if MOUNT_TYPE == ALTAZM
    // 计算当前的 Alt/Azm（经纬仪）跟踪速率
    if (lstNow%3 != 0) doHorRateCalc();
#else
    // 计算当前大气折射补偿后的跟踪速率
    if (rateCompensation != RC_NONE && lstNow%3 != 0) doRefractionRateCalc();
#endif

#if LIMIT_SENSE != OFF
    byte limit_reading = digitalRead(LimitPin);
    unsigned long currentTime = millis(); 
    
    // [检测到限位触发]
    if (limit_reading == LIMIT_SENSE_STATE) {
      
      lastLimitTriggerTime = currentTime;

      // 1. 【最高权限】回零模式直接放行
      if (isHoming()) {
          Axis1_LimitLock = 0;
          Axis2_LimitLock = 0;
          return; 
      }
      
      // 2. 简单的触发滤波
      delay(2);
      if (digitalRead(LimitPin) == LIMIT_SENSE_STATE) {

        // =========================================================
        // 3. 统一方向定义
        // =========================================================
        int currentMotionDir1 = 0;
        int currentMotionDir2 = 0;

        // --- Axis 1 (RA) ---
        if (guideDirAxis1 == 'e') currentMotionDir1 = 1;       
        else if (guideDirAxis1 == 'w') currentMotionDir1 = -1; 
        else if (trackingState == TrackingMoveTo) {            
             if (targetAxis1.part.m < posAxis1) currentMotionDir1 = 1; 
             else if (targetAxis1.part.m > posAxis1) currentMotionDir1 = -1; 
        }

        // --- Axis 2 (DEC) 恢复标准逻辑 ---
        if (guideDirAxis2 == 'n') currentMotionDir2 = 1;       
        else if (guideDirAxis2 == 's') currentMotionDir2 = -1; 
        else if (trackingState == TrackingMoveTo) {            
             if (targetAxis2.part.m > posAxis2) currentMotionDir2 = 1; 
             else if (targetAxis2.part.m < posAxis2) currentMotionDir2 = -1; 
        }

        // =========================================================
        // 4. 智能记录锁死方向 (解决静止打断死锁)
        // =========================================================
        
        // --- Axis 1 智能判断 ---
        if (Axis1_LimitLock == 0) {
            // 如果是 GOTO 过程中撞击，意图明确，直接记录
            if (currentMotionDir1 != 0 && trackingState == TrackingMoveTo) {
                Axis1_LimitLock = currentMotionDir1;
            } else {
                // 如果是静止状态下触发(被打断/震动)，通过绝对坐标推断撞了哪边
                long threshold = 500L; // 容错阈值
                if (posAxis1 > threshold) Axis1_LimitLock = -1;       // 在西半区，锁西 (-1)
                else if (posAxis1 < -threshold) Axis1_LimitLock = 1;  // 在东半区，锁东 (1)
                else if (currentMotionDir1 != 0) Axis1_LimitLock = currentMotionDir1; // 兜底
            }
        }

        // --- Axis 2 智能判断 ---
        if (Axis2_LimitLock == 0) {
            if (currentMotionDir2 != 0 && trackingState == TrackingMoveTo) {
                Axis2_LimitLock = currentMotionDir2;
            } else {
                long threshold = 500L;
                if (posAxis2 > threshold) Axis2_LimitLock = 1;        // 在北半区，锁北 (1)
                else if (posAxis2 < -threshold) Axis2_LimitLock = -1; // 在南半区，锁南 (-1)
                else if (currentMotionDir2 != 0) Axis2_LimitLock = currentMotionDir2;
            }
        }

        // =========================================================
        // 5. 逃离判断 (Escape Logic)
        // =========================================================
        bool isEscaping = false;
        
        if (Axis1_LimitLock != 0 && currentMotionDir1 != 0 && currentMotionDir1 != Axis1_LimitLock) isEscaping = true;
        if (Axis2_LimitLock != 0 && currentMotionDir2 != 0 && currentMotionDir2 != Axis2_LimitLock) isEscaping = true;

        // =========================================================
        // 6. 执行急停
        // =========================================================
        if (!isEscaping) {
            generalError = ERR_LIMIT_SENSE;

#if HOME_SENSE != OFF
            // 只有启用 HOME_SENSE 时，限位中断后才进入“必须重新真实回零”的增强保护。
            // HOME_SENSE 关闭时，不引入回零锁，尽量保持 OnStep 4.24 原始控制路径。
            systemHasHomed = false;
            gotoRequiresHomeAfterAbort = true;
            if (trackingState == TrackingMoveTo) gotoAbortedBySafety = true;
#endif

            stopGuideAxis1(); 
            stopGuideAxis2();
            stopSlewingAndTracking(SS_LIMIT_HARD);
        }
      } 
    } else {
       // [限位松开]
       if (currentTime - lastLimitTriggerTime > 500) {
           if (guideDirAxis1 == 0 && trackingState != TrackingMoveTo) Axis1_LimitLock = 0;
           if (guideDirAxis2 == 0 && trackingState != TrackingMoveTo) Axis2_LimitLock = 0;
       }
    }
#endif

    // 检查故障信号，停止任何旋转或导星并关闭跟踪
#if AXIS1_DRIVER_STATUS == LOW || AXIS1_DRIVER_STATUS == HIGH
    faultAxis1=(digitalRead(Axis1_FAULT) == AXIS1_DRIVER_STATUS);
#elif AXIS1_DRIVER_STATUS == TMC_SPI
    if (lst%2 == 0) faultAxis1=tmcAxis1.error();
#endif
#if AXIS2_DRIVER_STATUS == LOW || AXIS2_DRIVER_STATUS == HIGH
    faultAxis2=(digitalRead(Axis2_FAULT) == AXIS2_DRIVER_STATUS);
#elif AXIS2_DRIVER_STATUS == TMC_SPI
    if (lst%2 == 1) faultAxis2=tmcAxis2.error();
#endif

    if (faultAxis1 || faultAxis2) { generalError=ERR_MOTOR_FAULT; stopSlewingAndTracking(SS_LIMIT_HARD);
    }

    if (safetyLimitsOn) {
      // 检查高度角上限和地平线限制
      if (currentAlt < minAlt) { generalError=ERR_ALT_MIN;
      stopSlewingAndTracking((MOUNT_TYPE == ALTAZM)?SS_LIMIT_AXIS2_MIN:SS_LIMIT); }
      if (currentAlt > maxAlt) { generalError=ERR_ALT_MAX; stopSlewingAndTracking((MOUNT_TYPE == ALTAZM)?SS_LIMIT_AXIS2_MAX:SS_LIMIT);
      }
    }

    // 选项：如果轴2不移动则断电
#if AXIS2_DRIVER_POWER_DOWN == ON && MOUNT_TYPE != ALTAZM
    autoPowerDownAxis2();
#endif

    // 0.01秒 轮询任务 
#if TIME_LOCATION_SOURCE == GPS
    if ((PPS_SENSE == OFF || ppsSynced) && !tls.active && tls.poll()) {
      SerialGPS.end();
      currentSite=0; nv.update(EE_currentSite,currentSite);

      tls.getSite(latitude,longitude);
      tls.get(JD,LMT);

      timeZone=nv.read(EE_sites+currentSite*25+8)-128;
      timeZone=decodeTimeZone(timeZone);
      UT1=LMT+timeZone;

      nv.writeString(EE_sites+currentSite*25+9,(char*)"GPS");
      setLatitude(latitude);
      nv.writeFloat(EE_sites+currentSite*25+4,longitude);
      updateLST(jd2last(JD,UT1,false));

      if (generalError == ERR_SITE_INIT) generalError=ERR_NONE;

      dateWasSet=true;
      timeWasSet=true;
      if (parkStatus == Parked) {
        VLF("MSG: Restoring parked telescope pointing state");
        unPark(false);
      }
}
#endif

    // 更新 UT1 时钟
    cli(); long cs=lst; sei();
    double t2=(double)((cs-lst_start)/100.0)/1.00273790935;
    // 这只需要精确到秒，精度大约高 10 倍
    UT1=UT1_start+(t2/3600.0);
    // 更新辅助功能
#ifdef FEATURES_PRESENT
    featuresPoll();
#endif
    
    // 天气监测
    if (!isSlewing()) ambient.poll();
    // 监控 NV 缓存
#if DEBUG == VERBOSE && DEBUG_NV == ON
    static bool lastCommitted=true;
    bool committed=nv.committed();
    if (committed && !lastCommitted) { DLF("MSG: NV commit done"); lastCommitted=committed;
    }
    if (!committed && lastCommitted) { DLF("MSG: NV data in cache"); lastCommitted=committed;
    }
#endif

    // 触发 ESPFLASH (固件烧写)
#if defined(AddonTriggerPin)
    fa.poll();
#endif
  }

  // 最快轮询 (FASTEST POLLING) 
#if MODE_SWITCH_BEFORE_SLEW == OFF && AXIS1_DRIVER_MODEL == TMC_SPI
  autoModeSwitch();
#endif

#if ROTATOR == ON
  rot.follow(isSlewing());
#endif
#if FOCUSER1 == ON
  foc1.follow(isSlewing());
#endif
#if FOCUSER2 == ON
  foc2.follow(isSlewing());
#endif
  if (!isSlewing()) nv.poll();
  
  // 工作负载监控 (WORKLOAD MONITORING) 
  unsigned long this_loop_micros=micros();
  loop_time=(long)(this_loop_micros-last_loop_micros);
  if (loop_time > worst_loop_time) worst_loop_time=loop_time;
  last_loop_micros=this_loop_micros;
  average_loop_time=(average_loop_time*49+loop_time)/50;

  // 1 秒定时任务 
  unsigned long tempMs=millis();
  static unsigned long housekeepingTimer=0;
  if ((long)(tempMs-housekeepingTimer) > 1000L) {
    housekeepingTimer=tempMs;

#if ROTATOR == ON && MOUNT_TYPE == ALTAZM
    // 根据需要计算并设置场旋消除速率
    double h,d;
    getApproxEqu(&h,&d,true);
    if (trackingState == TrackingSidereal) rot.derotate(h,d);
#endif

    // 调整经纬仪 (Alt/Azm) 的跟踪速率
    // 调整折射补偿的跟踪速率
    setDeltaTrackingRate();
    // 基本检查，看我们是否不在原点 (Home)
    if (trackingState != TrackingNone) atHome=false;
#if PPS_SENSE != OFF
    // 通过 PPS (每秒脉冲) 更新时钟
    cli();
    ppsRateRatio=((double)1000000.0/(double)(ppsAvgMicroS));
    if ((long)(micros()-(ppsLastMicroS+2000000UL)) > 0) ppsSynced=false; // 如果超过两秒没有脉冲，则失去同步
    sei();
#if LED_STATUS2 == ON
    if (trackingState == TrackingSidereal) {
      if (ppsSynced) { if (led2On) { digitalWrite(LEDneg2Pin,HIGH);
      led2On=false; } else { digitalWrite(LEDneg2Pin,LOW); led2On=true; } } else { digitalWrite(LEDneg2Pin,HIGH); led2On=false;
      } // 指示 PPS 状态
    }
  #endif
    if (ppsLastRateRatio != ppsRateRatio) { SiderealClockSetInterval(siderealInterval); ppsLastRateRatio=ppsRateRatio;
    }
#endif

#if LED_STATUS == ON
    // LED 指示电源开启 
    if (trackingState != TrackingSidereal) if (!ledOn) { digitalWrite(LEDnegPin,LOW);
    ledOn=true; }
#endif
#if LED_STATUS2 == ON
    // LED 指示停止和 GOTO
    if (trackingState == TrackingMoveTo) if (!led2On) { digitalWrite(LEDneg2Pin,LOW);
    led2On=true; }
  #if PPS_SENSE != OFF
    if (trackingState == TrackingNone) if (led2On) { digitalWrite(LEDneg2Pin,HIGH); led2On=false;
    }
  #else
    if (trackingState != TrackingMoveTo) if (led2On) { digitalWrite(LEDneg2Pin,HIGH); led2On=false;
    }
  #endif
#endif

    // 安全检查 (SAFETY CHECKS)
    // 防止支架跟踪超过中天限制、超过 AXIS1_LIMIT_MAX 或超过 Dec 限制
    if (safetyLimitsOn) {
      // 检查是否超过 AXIS1_LIMIT_MIN 或 AXIS1_LIMIT_MAX
      if (getInstrAxis1() < axis1Settings.min) { generalError=(MOUNT_TYPE==ALTAZM)?ERR_AZM:ERR_UNDER_POLE;
      stopSlewingAndTracking(SS_LIMIT_AXIS1_MIN); } else
      if (getInstrAxis1() > axis1Settings.max) { generalError=(MOUNT_TYPE==ALTAZM)?ERR_AZM:ERR_UNDER_POLE; stopSlewingAndTracking(SS_LIMIT_AXIS1_MAX);
      } else
      // 检查是否超过中天限制
      if (meridianFlip != MeridianFlipNever) {
        if (getInstrPierSide() == PierSideWest) {
          if (getInstrAxis1() > degreesPastMeridianW && (!(autoMeridianFlip && goToHere(true) == CE_NONE))) { generalError=ERR_MERIDIAN;
          stopSlewingAndTracking(SS_LIMIT_AXIS1_MAX); }
        } else
        if (getInstrAxis1() < -degreesPastMeridianE) { generalError=ERR_MERIDIAN;
          stopSlewingAndTracking(SS_LIMIT_AXIS1_MIN); }
      }
    }
    double a2;
    if (AXIS2_TANGENT_ARM == ON) { cli(); a2=posAxis2/axis2Settings.stepsPerMeasure; sei(); } else a2=getInstrAxis2();
    // 检查是否超过 AXIS2_LIMIT_MIN 或 AXIS2_LIMIT_MAX
    if (a2 < axis2Settings.min) { generalError=ERR_DEC; stopSlewingAndTracking(SS_LIMIT_AXIS2_MIN);
    } else
    if (a2 > axis2Settings.max) { generalError=ERR_DEC; stopSlewingAndTracking(SS_LIMIT_AXIS2_MAX);
    } else
    // 在切线臂模式下自动清除错误
    if (AXIS2_TANGENT_ARM == ON && (trackingState == TrackingSidereal && generalError == ERR_DEC)) generalError=ERR_NONE;
  } else {
    // 命令处理 (COMMAND PROCESSING) 
    processCommands();
  }
}

// 根据需要停止快速运动
// SS_ALL_FAST 停止旋转（Slewing）但不停止跟踪
// SS_LIMIT 停止 GOTO + 螺旋搜寻 + 跟踪
// SS_LIMIT_HARD 停止旋转 + 跟踪
// SS_LIMIT_AXIS1_MIN 停止 GOTO + 螺旋搜寻 + 跟踪，并停止/阻止反向的赤经/方位角导星
// SS_LIMIT_AXIS1_MAX 停止 GOTO + 螺旋搜寻 + 跟踪，并停止/阻止正向的赤经/方位角导星
// SS_LIMIT_AXIS2_MIN 停止 GOTO + 螺旋搜寻 + 跟踪，并停止/阻止反向的赤纬/高度角导星
// SS_LIMIT_AXIS2_MAX 停止 GOTO + 螺旋搜寻 + 跟踪，并停止/阻止正向的赤纬/高度角导星
void stopSlewingAndTracking(StopSlewActions ss) {

  if (trackingState == TrackingMoveTo) {

    // GOTO 过程中，所有非普通停止都视为安全中断。
    // 这样 MoveTo 不会把当前位置伪装成正常目标完成。
    if (ss != SS_ALL_FAST) {
      gotoAbortedBySafety = true;
      gotoRequiresHomeAfterAbort = true;
    }

    if (!abortGoto) {
      abortGoto = StartAbortGoto;
      VLF("MSG: Goto aborted");
    }

  } else {

    if (spiralGuide) stopGuideSpiral();

    if (ss == SS_ALL_FAST || ss == SS_LIMIT_HARD) {
      stopGuideAxis1();
      stopGuideAxis2();
    } else
    if (ss == SS_LIMIT_AXIS1_MIN) {
      if (guideDirAxis1 == 'e') guideDirAxis1 = 'b';
    } else
    if (ss == SS_LIMIT_AXIS1_MAX) {
      if (guideDirAxis1 == 'w') guideDirAxis1 = 'b';
    } else
    if (ss == SS_LIMIT_AXIS2_MIN) {
      if (getInstrPierSide() == PierSideWest) {
        if (guideDirAxis2 == 'n') guideDirAxis2 = 'b';
      } else {
        if (guideDirAxis2 == 's') guideDirAxis2 = 'b';
      }
    } else
    if (ss == SS_LIMIT_AXIS2_MAX) {
      if (getInstrPierSide() == PierSideWest) {
        if (guideDirAxis2 == 's') guideDirAxis2 = 'b';
      } else {
        if (guideDirAxis2 == 'n') guideDirAxis2 = 'b';
      }
    }

    if (trackingState != TrackingNone) {
      if (ss != SS_ALL_FAST) {
        if (generalError != ERR_DEC) {
          stopGuideAxis1();
          stopGuideAxis2();
          trackingState = TrackingNone;
          VLF("MSG: Limit exceeded guiding/tracking stopped");
        }
      }
    }
  }
}