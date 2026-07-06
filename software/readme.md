# OnStep 谐波赤道仪安全回零与限位脱困补丁 (For N.I.N.A. / ASCOM / APP)

---

## 📝 摘要 (Summary)

本次提交对 OnStep 4.24 的底层控制、回零状态机、LX200 指令处理和限位保护逻辑进行了安全增强。修改目标是让 DIY 谐波赤道仪在开机、未回零、GOTO、Tracking、物理限位触发以及回零偏置过程中都保持可控状态。

当前代码实现的核心策略是：**自动动作严格、手动动作保留、限位后可脱困、回零成功才解锁**。

也就是说，开机后系统会自动使能电机形成电子保持力矩；在真实回零完成前，N.I.N.A. / ASCOM / APP 发出的 GOTO 与 Tracking 会被拦截；但手动方向键不会因为“未回零”而被完全禁止，用户仍可手动调整姿态或进行脱困。若触发物理限位，代码会记录危险方向，只禁止继续撞限位的一侧，同时允许反方向安全离开。Home 成功、Set Home 或无传感器模式下的原生 Home 完成后，系统才重新允许 GOTO 与 Tracking。

---

## 📌 背景与痛点 (Pain Points)

在将原生 OnStep 用于 DIY 谐波赤道仪时，会遇到以下软硬件协同风险：

1. **开机/待机溜车风险**：谐波减速器没有传统离合结构，若镜筒不平衡，开机初期电机未保持时容易发生重力滑落。

2. **未回零前坐标不可信**：Home 传感器启用时，开机后的机械绝对位置尚未确认，此时若允许 N.I.N.A. 或 APP 直接 GOTO，可能造成错误指向、撞机或绕线。

3. **GOTO 失败后幽灵 Tracking**：部分上位机在 GOTO 前后会尝试开启 Tracking。若未完成 Home 或安全中断后仍允许 Tracking，可能导致赤道仪在错误坐标基准下继续运动。

4. **物理限位后不能完全锁死**：触发限位后，如果所有方向键都被禁止，赤道仪无法反向脱困；如果完全不锁，又可能继续朝危险方向撞击。

5. **回零失败不能伪装成成功**：如果只根据“回零状态结束”来标记已回零，回零超时、启动失败或安全中断都可能被误判为成功，后续 GOTO 风险极高。

6. **DIY 传感器安装误差需要大角度补偿**：实际 Home 传感器位置和真正机械零位可能有较大偏差，因此需要以“度”为单位进行第三阶段零位偏置。

7. **代码需要兼容有/无 Home 传感器与有/无限位传感器的硬件**：HOME_SENSE 或 LIMIT_SENSE 关闭时，不能让安全锁把原生 OnStep 行为永久锁死。

---

## ✨ 核心功能特性 (Key Features)

1. **开机电子抱闸防溜车**  
   开机后只使能步进电机驱动器，不启动 Tracking，不建立 Home 坐标，利用电机保持力矩防止谐波赤道仪滑落。

2. **未回零前禁止 GOTO**  
   当 HOME_SENSE 启用且 `systemHasHomed == false`，或上一次 GOTO 被安全中断后尚未重新 Home 时，`:MS#` 会返回 standby 状态，阻止 N.I.N.A. / ASCOM 执行自动 GOTO。

3. **未回零前禁止 Tracking**  
   当坐标基准不可信时，`:Te#` 等 Tracking 指令会被拒绝，避免 GOTO 被拦截后上位机误开恒星跟踪。

4. **未回零时允许手动方向键**  
   当前最终代码不再因为“未回零”而锁死 `:Me# / :Mw# / :Mn# / :Ms#`。这样开机后仍可手动调整姿态，便于救机、找传感器或脱离机械风险。

5. **限位智能脱困 (Smart Escape)**  
   触发物理限位后，代码记录 Axis1 / Axis2 的危险方向，只静默拒绝继续撞限位的一侧，并允许反方向移动。

6. **安全中断后强制重新 Home**  
   如果 GOTO 或运动过程中触发安全中断，系统会标记坐标不可信，后续 GOTO 与 Tracking 需要重新 Find Home 或 Set Home 后才恢复。

7. **三阶段自动回零状态机**  
   Home 流程包含快速找零、慢速精找、第三阶段零位偏置。第一/第二阶段均有超时保护，第三阶段按配置角度和速度档位进行定时偏置。

8. **回零成功才解锁坐标可信状态**  
   `systemHasHomed = true` 只在 Home 真正完成、Set Home 手动确认或 HOME_SENSE 关闭时的原生 Home 完成后设置，避免失败回零误解锁。

9. **传感器宏绑定兼容**  
   HOME_SENSE 关闭时，代码尽量恢复原生 OnStep 行为；LIMIT_SENSE 关闭时，限位方向锁和脱困逻辑不参与控制。

10. **保留原生 GOTO 校验**  
    Horizon Limit、Axis1/Axis2 机械范围、Meridian 限制等仍由原生 `validateGotoCoords()` / `goToEqu()` 路径处理。N.I.N.A. 报 `Coordinates outside limits` 时，应优先检查地平线限制、Axis1 limit、经纬度和目标几何位置。

---

## 🛠️ 代码修改与源码对照 (Code Modification Guide)

### 1. 开机自动使能电机防止“溜车”滑落

- **文件**：`Config.h`、`OnStep.ino`
- **修改逻辑**：新增并启用 `MOTOR_HOLD_ON_BOOT`。开机后使能电机驱动器，但不启动 Tracking，也不把当前位置设为 Home。

```cpp
// Config.h
#define MOTOR_HOLD_ON_BOOT             ON // 只使能驱动器，不启动 tracking，不建立 home 坐标
```

```cpp
// OnStep.ino
#if MOTOR_HOLD_ON_BOOT == ON
  #if MOUNT_TYPE != ALTAZM
    VLF("MSG: Motor hold on boot - drivers enabled, tracking off, home required");

    systemHasHomed = false;
    gotoAbortedBySafety = false;
    gotoRequiresHomeAfterAbort = false;

    Axis1_LimitLock = 0;
    Axis2_LimitLock = 0;

    trackingState = TrackingNone;
    lastTrackingState = TrackingNone;
    abortTrackingState = TrackingNone;

    enableStepperDrivers();
  #endif
#endif
```

**实际效果**：  
开机后电机立即获得保持力矩，但赤道仪不会自动跟踪，也不会误认为已经完成 Home。

---

### 2. 新增全局安全状态变量

- **文件**：`OnStep.ino`
- **修改逻辑**：在 OnStep 主文件顶部定义限位方向锁、GOTO 安全中断状态、强制回零状态。

```cpp
int Axis1_LimitLock = 0;
int Axis2_LimitLock = 0;
unsigned long lastLimitTriggerTime = 0;

bool gotoAbortedBySafety = false;
bool gotoRequiresHomeAfterAbort = false;
bool systemHasHomed = false;
```

**变量含义**：

| 变量 | 含义 |
|---|---|
| `Axis1_LimitLock` | Axis1 当前被锁死的危险方向，`0` 表示无限位锁 |
| `Axis2_LimitLock` | Axis2 当前被锁死的危险方向，`0` 表示无限位锁 |
| `lastLimitTriggerTime` | 限位释放后的清锁延时计时 |
| `gotoAbortedBySafety` | 本次 GOTO 是否被限位/安全逻辑中断 |
| `gotoRequiresHomeAfterAbort` | 安全中断后是否要求重新 Home |
| `systemHasHomed` | 当前坐标基准是否可信 |

---

### 3. 复用 ST4 接口实现物理“一键回零”

- **文件**：`Guide.ino` -> `ST4()` 函数
- **修改逻辑**：利用 ST4 East + West 同时长按 3 秒触发 `goHome(true)`。触发后使用 `homingLockout` 屏蔽按键干扰，防止回零过程被组合按键或残留按压打断。

```cpp
static bool homingLockout = false;

if (homingLockout) {
  if (!st4e.isDown() && !st4w.isDown() && !st4n.isDown() && !st4s.isDown()) {
    homingLockout = false;
  }
  return;
}

if ((trackingState != TrackingMoveTo) && (!waitingHome)) {
  if (st4e.isDown() && st4w.isDown()) {
    if ((st4e.timeDown() > 3000) && (st4w.timeDown() > 3000)) {
      homingLockout = true;
      soundBeep();
      altModeA = false;
      stopGuideAxis1();
      stopGuideAxis2();
      stopSlewingAndTracking(SS_ALL_FAST);
      goHome(true);
      return;
    }
  }
}
```

**实际效果**：  
外接实体按键同时按下 East + West 超过 3 秒，即可触发自动回零。代码保留原本 ST4 AltMode 逻辑，但在触发回零时会清理 `altModeA`，避免 2 秒组合键状态污染 3 秒回零动作。

---

### 4. GOTO 指令拦截：未 Home 或安全中断后禁止 `:MS#`

- **文件**：`Command.ino`
- **修改位置**：`:MS#` 指令处理处
- **修改逻辑**：当坐标基准不可信时，不进入原生 `goToEqu()`，而是返回 `3`，表示 controller in standby / not ready。

```cpp
if (command[1] == 'S' && parameter[0] == 0)  {
  if (!systemHasHomed || gotoRequiresHomeAfterAbort) {
    reply[0] = '3';
    reply[1] = 0;
    boolReply = false;
    supress_frame = true;
    commandError = CE_SLEW_ERR_IN_STANDBY;
  } else {
    newTargetRA = origTargetRA;
    newTargetDec = origTargetDec;
#if TELESCOPE_COORDINATES == TOPOCENTRIC
    topocentricToObservedPlace(&newTargetRA, &newTargetDec);
#endif
    CommandErrors e = goToEqu(newTargetRA, newTargetDec);
    if (e >= CE_GOTO_ERR_BELOW_HORIZON && e <= CE_GOTO_ERR_UNSPECIFIED) reply[0] = (char)(e - CE_GOTO_ERR_BELOW_HORIZON) + '1';
    if (e == CE_NONE) reply[0] = '0';
    reply[1] = 0;
    boolReply = false;
    supress_frame = true;
    commandError = e;
  }
}
```

**实际效果**：  
未完成 Home、或触发过安全中断但尚未重新 Home 时，N.I.N.A. / ASCOM 不会真正启动 GOTO。

---

### 5. Tracking 指令拦截：未 Home 或安全中断后禁止开启跟踪

- **文件**：`Command.ino`
- **修改位置**：`:T...#` 指令处理处
- **修改逻辑**：只要坐标基准不可信，就拒绝 Tracking 指令，避免“幽灵跟踪”。

```cpp
if (command[0] == 'T' && parameter[0] == 0) {
  if (!systemHasHomed || gotoRequiresHomeAfterAbort) {
    reply[0] = 0;
    boolReply = false;
    supress_frame = true;
    commandError = CE_SLEW_ERR_IN_STANDBY;
    return;
  }

  // 原生 Tracking 逻辑继续执行
}
```

**实际效果**：  
GOTO 被拒绝后，上位机即使尝试打开 Tracking，也会被拦截。

---

### 6. 手动方向键逻辑：未 Home 允许移动，只锁限位危险方向

- **文件**：`Command.ino`
- **修改位置**：`:Me# / :Mw# / :Mn# / :Ms#`
- **修改逻辑**：删除“未 Home 禁止手动方向键”的限制；只根据限位锁判断是否禁止当前方向。

```cpp
// :Me# :Mw#
if ((command[1] == 'e' || command[1] == 'w') && parameter[0] == 0) {
  if ((command[1] == 'e' && Axis1_LimitLock == 1) ||
      (command[1] == 'w' && Axis1_LimitLock == -1)) {
    boolReply = false;
    commandError = CE_NONE;
  } else {
    commandError = startGuideAxis1(command[1], currentGuideRate, GUIDE_TIME_LIMIT * 1000, false);
    boolReply = false;
  }
}

// :Mn# :Ms#
if ((command[1] == 'n' || command[1] == 's') && parameter[0] == 0) {
  if ((command[1] == 'n' && Axis2_LimitLock == 1) ||
      (command[1] == 's' && Axis2_LimitLock == -1)) {
    boolReply = false;
    commandError = CE_NONE;
  } else {
    commandError = startGuideAxis2(command[1], currentGuideRate, GUIDE_TIME_LIMIT * 1000, false);
    boolReply = false;
  }
}
```

**实际效果**：  
开机未回零时可以手动移动。触发限位后，危险方向静默拒绝，反方向仍可脱困。

---

### 7. Guide 层限位脱困：危险方向禁止，反方向允许

- **文件**：`Guide.ino`
- **修改位置**：`startGuideAxis1()`、`startGuideAxis2()`
- **修改逻辑**：在底层 guide 启动处再次判断限位方向锁，防止任何入口绕过 Command 层。

```cpp
// Axis1
if (direction == 'e' && Axis1_LimitLock == 1)  return CE_SLEW_ERR_OUTSIDE_LIMITS;
if (direction == 'w' && Axis1_LimitLock == -1) return CE_SLEW_ERR_OUTSIDE_LIMITS;

bool escapingPhysicalLimitAxis1 = (generalError == ERR_LIMIT_SENSE) &&
                                  ((direction == 'e' && Axis1_LimitLock == -1) ||
                                   (direction == 'w' && Axis1_LimitLock == 1));
```

```cpp
// Axis2
if (direction == 'n' && Axis2_LimitLock == 1)  return CE_SLEW_ERR_OUTSIDE_LIMITS;
if (direction == 's' && Axis2_LimitLock == -1) return CE_SLEW_ERR_OUTSIDE_LIMITS;

bool escapingPhysicalLimitAxis2 = (generalError == ERR_LIMIT_SENSE) &&
                                  ((direction == 'n' && Axis2_LimitLock == -1) ||
                                   (direction == 's' && Axis2_LimitLock == 1));
```

**实际效果**：  
即使 `generalError == ERR_LIMIT_SENSE`，只要当前方向是反向脱困方向，guide 仍可启动。

---

### 8. loop2() 限位检测与智能方向锁

- **文件**：`OnStep.ino`
- **修改位置**：`loop2()` 中 `#if LIMIT_SENSE != OFF` 段
- **修改逻辑**：读取物理限位引脚，检测到稳定触发后判断当前运动方向，记录危险方向，并在非脱困方向上执行硬停止。

核心逻辑：

```cpp
#if LIMIT_SENSE != OFF
  byte limit_reading = digitalRead(LimitPin);
  unsigned long currentTime = millis();

  if (limit_reading == LIMIT_SENSE_STATE) {
    lastLimitTriggerTime = currentTime;

    if (isHoming()) {
      Axis1_LimitLock = 0;
      Axis2_LimitLock = 0;
      return;
    }

    delay(2);
    if (digitalRead(LimitPin) == LIMIT_SENSE_STATE) {
      int currentMotionDir1 = 0;
      int currentMotionDir2 = 0;

      // 根据 guideDir 或 GOTO target-pos 判断当前运动方向
      // 然后写入 Axis1_LimitLock / Axis2_LimitLock

      bool isEscaping = false;
      if (Axis1_LimitLock != 0 && currentMotionDir1 != 0 && currentMotionDir1 != Axis1_LimitLock) isEscaping = true;
      if (Axis2_LimitLock != 0 && currentMotionDir2 != 0 && currentMotionDir2 != Axis2_LimitLock) isEscaping = true;

      if (!isEscaping) {
        generalError = ERR_LIMIT_SENSE;

#if HOME_SENSE != OFF
        systemHasHomed = false;
        gotoRequiresHomeAfterAbort = true;
        if (trackingState == TrackingMoveTo) gotoAbortedBySafety = true;
#else
        systemHasHomed = true;
        gotoRequiresHomeAfterAbort = false;
#endif
        stopGuideAxis1();
        stopGuideAxis2();
        stopSlewingAndTracking(SS_LIMIT_HARD);
      }
    }
  } else {
    if (currentTime - lastLimitTriggerTime > 500) {
      if (guideDirAxis1 == 0 && trackingState != TrackingMoveTo) Axis1_LimitLock = 0;
      if (guideDirAxis2 == 0 && trackingState != TrackingMoveTo) Axis2_LimitLock = 0;
    }
  }
#endif
```

**实际效果**：

| 场景 | 结果 |
|---|---|
| GOTO 中撞限位 | 急停，坐标失信，要求重新 Home |
| 手动撞限位 | 记录危险方向，危险方向禁止，反方向允许 |
| 回零过程中检测到 LimitPin | 清限位锁并返回，不让普通限位逻辑打断 Home 状态机 |
| 限位释放并停止运动超过 500ms | 清除方向锁 |

---

### 9. 修正 wasHoming 误解锁问题

- **文件**：`OnStep.ino`
- **修改位置**：`loop2()` 开头的 Homing 状态监听
- **修改逻辑**：只记录 Homing 是否结束，不在这里设置 `systemHasHomed = true`。

```cpp
static bool wasHoming = false;
if (isHoming()) {
  wasHoming = true;
} else if (wasHoming) {
  // 不在这里把 systemHasHomed 置 true。
  // 真实回零成功只由 Home.ino 的 FH_DONE 或 setHome() 明确设置。
  wasHoming = false;
}
```

**实际效果**：  
回零超时、回零启动失败、回零被中断时，不会因为 Homing 状态结束而被误判为已回零。

---

### 10. 三阶段自动回零状态机

- **文件**：`Home.ino`
- **修改逻辑**：扩展 `findHomeMode`，增加 `FH_IDLE2` 和 `FH_OFFSET`，形成“快速找零 → 慢速精找 → 零位偏置 → 完成解锁”的闭环。

```cpp
enum findHomeModes { FH_OFF, FH_FAST, FH_IDLE, FH_SLOW, FH_IDLE2, FH_OFFSET, FH_DONE };
```

流程如下：

```text
FH_FAST   第一阶段：快速寻找 Home switch
FH_IDLE   第一阶段完成后的过渡状态，自动启动慢速精找
FH_SLOW   第二阶段：慢速精找 Home switch
FH_IDLE2  第二阶段完成后的过渡状态，自动启动零位偏置
FH_OFFSET 第三阶段：按 Config.h 中的偏置角度定时移动
FH_DONE   完成，初始化真实零位并解锁 GOTO / Tracking
```

**关键保护**：

| 阶段 | 保护逻辑 |
|---|---|
| 第一阶段 | 按 180° 行程估算超时时间 |
| 第二阶段 | 30 秒超时 |
| 第三阶段 | 根据偏置角度和速度计算运行时间，到点停止 |
| 任一阶段启动失败 | 退出 Home，不标记成功 |

---

### 11. Home Offset 改为 Config.h 中的度数与速度档位

- **文件**：`Config.h`、`Home.ino`
- **修改逻辑**：以度为单位配置零位偏置，并通过 `HOME_OFFSET_RATE` 选择第三阶段偏置速度。

```cpp
#define HOME_OFFSET_AXIS1          -1.5
#define HOME_OFFSET_AXIS2             1
#define HOME_OFFSET_RATE              8

#if HOME_OFFSET_RATE < 0 || HOME_OFFSET_RATE > 9
  #error "HOME_OFFSET_RATE must be from 0 to 9"
#endif
```

第三阶段根据速度档位换算每度所需时间：

```cpp
double secPerDeg = 3600.0 / (double)guideRates[HOME_OFFSET_RATE];
```

再根据偏置角度启动对应方向 guide：

```cpp
char dir1 = (HOME_OFFSET_AXIS1 > 0) ? 'e' : 'w';
unsigned long duration1 = (unsigned long)(fabs(HOME_OFFSET_AXIS1) * secPerDeg * 1000.0);
e1 = startGuideAxis1(dir1, HOME_OFFSET_RATE, 0, false);
```

**实际效果**：  
Home switch 停止位置不再被直接当作最终零位，而是在第二阶段精找后继续执行第三阶段偏置，最终把“传感器位置 + 偏置量”作为真实零位。

---

### 12. 第三阶段启动失败保护

- **文件**：`Home.ino`
- **修改逻辑**：第三阶段 Axis1 或 Axis2 偏置启动失败时，不继续伪装为完成，而是退出 Home 并报错。

```cpp
if (e1 != CE_NONE || e2 != CE_NONE) {
  findHomeMode = FH_OFF;
  safetyLimitsOn = true;
  generalError = ERR_LIMIT_SENSE;
  stopSlewingAndTracking(SS_ALL_FAST);
  VLF("MSG: Homing phase 3 failed");
  return;
}
```

**实际效果**：  
第三阶段如果因为限位、硬件、状态错误等原因无法启动，不会进入 `FH_DONE`，也不会把 `systemHasHomed` 置为 true。

---

### 13. Home 完成后才设置坐标可信

- **文件**：`Home.ino`
- **修改位置**：`FH_DONE` 分支
- **修改逻辑**：只有三阶段全部完成并停稳后，才初始化起始位置，设置 `atHome = true`，并清理安全锁。

```cpp
if (findHomeMode == FH_DONE && guideDirAxis1 == 0 && guideDirAxis2 == 0) {
  findHomeMode = FH_OFF;
  VLF("MSG: Homing done");

  initStartPosition();
  atHome = true;

  systemHasHomed = true;
  gotoAbortedBySafety = false;
  gotoRequiresHomeAfterAbort = false;

  Axis1_LimitLock = 0;
  Axis2_LimitLock = 0;

  abortGoto = 0;
  lastTrackingState = TrackingNone;
  abortTrackingState = TrackingNone;
  generalError = ERR_NONE;
}
```

**实际效果**：  
只有 Home 真正完成后，后续 GOTO / Tracking 才会恢复。

---

### 14. goHome() 启动失败保护

- **文件**：`Home.ino`
- **修改位置**：`goHome(bool fast)`
- **修改逻辑**：第一阶段或第二阶段 guide 启动失败时，立即退出 Homing 状态，恢复安全限制并停止运动。

```cpp
if (e != CE_NONE) {
  findHomeMode = FH_OFF;
  safetyLimitsOn = true;
  stopSlewingAndTracking(SS_ALL_FAST);
}
return e;
```

**实际效果**：  
如果 `startGuideAxis1()` 或 `startGuideAxis2()` 没能成功启动，系统不会卡在 FH_FAST / FH_SLOW，也不会进入后续阶段。

---

### 15. isHoming() 与 HOME_SENSE 宏绑定

- **文件**：`Home.ino`
- **修改逻辑**：当 HOME_SENSE 启用时，Homing 包含自动传感器回零状态机；当 HOME_SENSE 关闭时，只保留原生 `homeMount` 行为。

```cpp
bool isHoming() {
#if HOME_SENSE != OFF
  return (homeMount || (findHomeMode != FH_OFF));
#else
  return homeMount;
#endif
}
```

**实际效果**：  
启用 Home 传感器时，安全锁知道当前正在自动回零；关闭 Home 传感器时，不会因为缺少 `findHomeMode` 而破坏原生逻辑。

---

### 16. Set Home 手动确认后解锁

- **文件**：`Home.ino`
- **修改位置**：`setHome()`
- **修改逻辑**：用户手动执行 Set Home 后，视为当前位置已经被人工确认可信，因此清理安全锁并允许后续 GOTO / Tracking。

```cpp
systemHasHomed = true;
gotoAbortedBySafety = false;
gotoRequiresHomeAfterAbort = false;
Axis1_LimitLock = 0;
Axis2_LimitLock = 0;
abortGoto = 0;
generalError = ERR_NONE;
```

**实际效果**：  
如果不使用自动 Find Home，也可以通过手动移动到正确零位后执行 Set Home 来建立坐标基准。

---

### 17. HOME_SENSE 关闭时保留原生 Home 完成逻辑

- **文件**：`MoveTo.ino`
- **修改逻辑**：当 HOME_SENSE 关闭时，原生 `goHome()` 通过 `homeMount` 和 `moveTo()` 回到 Home。完成后也要恢复 `systemHasHomed = true`，否则无 Home 传感器设备会被永久锁定。

```cpp
if (homeMount) {
  homeMount = false;
  if (AXIS2_TANGENT_ARM == OFF) atHome = true;

  systemHasHomed = true;
  gotoAbortedBySafety = false;
  gotoRequiresHomeAfterAbort = false;
  Axis1_LimitLock = 0;
  Axis2_LimitLock = 0;

  VLF("MSG: Homing done");
}
```

**实际效果**：  
关闭 HOME_SENSE 时，代码仍兼容原生 OnStep 的 Home 行为，不会因为新增安全锁导致无传感器设备无法使用。

---

### 18. MoveTo 安全中断保护：不把失败 GOTO 伪装成完成

- **文件**：`MoveTo.ino`
- **修改逻辑**：如果 GOTO 过程中被限位或安全逻辑中断，`moveTo()` 不再恢复为正常 GOTO done，而是保持 TrackingNone，并要求重新 Home。

```cpp
if (gotoAbortedBySafety) {
  trackingState = TrackingNone;
  lastTrackingState = TrackingNone;

  SiderealClockSetInterval(siderealInterval);
  setDeltaTrackingRate();

  if (generalError == ERR_NONE) generalError = ERR_LIMIT_SENSE;

  gotoAbortedBySafety = false;
  gotoRequiresHomeAfterAbort = true;

  VLF("MSG: Goto failed by safety abort; home required");
}
```

**实际效果**：  
限位急停后，N.I.N.A. / ASCOM 不会被误导为 GOTO 已经正常完成。

---

### 19. stopSlewingAndTracking() 标记安全中断

- **文件**：`OnStep.ino`
- **修改位置**：`stopSlewingAndTracking()`
- **修改逻辑**：GOTO 过程中只要不是普通 `SS_ALL_FAST` 停止，就标记为安全中断，并要求重新 Home。

```cpp
if (trackingState == TrackingMoveTo) {
  if (ss != SS_ALL_FAST) {
    gotoAbortedBySafety = true;
    gotoRequiresHomeAfterAbort = true;
  }

  if (!abortGoto) {
    abortGoto = StartAbortGoto;
    VLF("MSG: Goto aborted");
  }
}
```

**实际效果**：  
GOTO 中遇到限位、低高度、安全停止等情况时，系统不会继续相信当前位置坐标。

---

### 20. Axis1 机械限位与低空 GOTO 说明

- **文件**：`Config.h`
- **当前配置**：

```cpp
#define AXIS1_LIMIT_MIN              -92
#define AXIS1_LIMIT_MAX               92
```

这不是 Horizon Limit，而是 GEM 模式下 Axis1 / HA 的机械运动范围限制。低空目标尤其容易接近或超过这个范围，因此 N.I.N.A. 可能报：

```text
SlewError: Coordinates outside limits
```

若需要给地平线附近留少量余量，可考虑：

```cpp
#define AXIS1_LIMIT_MIN             -115
#define AXIS1_LIMIT_MAX              115
```

不建议直接放到 `±180°`，否则可能增加撞三脚架、绕线或错误墩侧路径风险。

---

## 🔁 运行状态流程 (Runtime Flow)

### 正常开机流程

```text
上电
  ↓
MOTOR_HOLD_ON_BOOT 使能电机保持
  ↓
trackingState = TrackingNone
  ↓
systemHasHomed = false
  ↓
手动方向键允许移动
  ↓
GOTO / Tracking 被拦截
  ↓
执行 Find Home 或 Set Home
  ↓
成功后 systemHasHomed = true
  ↓
允许 N.I.N.A. / ASCOM GOTO 与 Tracking
```

### 自动 Find Home 流程

```text
ST4 East + West 长按 3 秒 / APP Find Home
  ↓
FH_FAST 快速寻找 Home switch
  ↓
FH_IDLE
  ↓
FH_SLOW 慢速精找 Home switch
  ↓
FH_IDLE2
  ↓
FH_OFFSET 根据 HOME_OFFSET_AXIS1/2 和 HOME_OFFSET_RATE 执行偏置
  ↓
FH_DONE
  ↓
initStartPosition()
  ↓
systemHasHomed = true
```

### 限位触发流程

```text
LimitPin 触发
  ↓
判断当前运动方向
  ↓
写入 Axis1_LimitLock / Axis2_LimitLock
  ↓
如果不是反向脱困方向：急停
  ↓
HOME_SENSE 启用时 systemHasHomed = false
  ↓
gotoRequiresHomeAfterAbort = true
  ↓
禁止后续 GOTO / Tracking
  ↓
允许反方向手动脱困
  ↓
重新 Find Home 或 Set Home 后解锁
```

---

## ✅ 当前代码行为总表

| 功能场景 | 当前行为 |
|---|---|
| 开机 | 电机保持，Tracking 关闭，不自动建立 Home |
| 未 Home 前手动方向键 | 允许移动 |
| 未 Home 前 GOTO | 拦截，返回 standby |
| 未 Home 前 Tracking | 拒绝开启 |
| 触发物理限位 | 记录危险方向，急停 |
| 限位后危险方向 | 静默拒绝 |
| 限位后反方向 | 允许移动，用于脱困 |
| 第一阶段 Home 超时 | 停止 Home，不进入第二阶段 |
| 第二阶段 Home 超时 | 停止 Home，不进入第三阶段 |
| 第三阶段偏置启动失败 | 停止 Home，不标记成功 |
| Home 完成 | 设置 `systemHasHomed = true`，清理安全锁 |
| Set Home | 手动确认坐标可信并解锁 |
| HOME_SENSE OFF | 尽量保持原生 OnStep Home/GOTO 行为 |
| LIMIT_SENSE OFF | 限位方向锁不参与控制 |

---

## 💡 总结与建议

这版补丁的重点不是简单地“锁死所有操作”，而是把风险动作和必要动作分开处理：

- **GOTO / Tracking 属于自动动作，必须在坐标可信后才允许。**
- **手动方向键属于救机动作，未 Home 时仍然保留。**
- **物理限位触发后，危险方向锁死，但反方向必须允许。**
- **Home 失败、超时或安全中断后，绝不伪装为 Home 成功。**
- **只有 FH_DONE、Set Home 或 HOME_SENSE 关闭时的原生 Home 完成，才恢复坐标可信状态。**

日常使用建议：

```text
1. 上电后先确认电机已保持
2. 如有需要，可用手动方向键调整姿态
3. 执行 Find Home
4. 等待三阶段 Home 完成
5. 再连接 N.I.N.A. 执行 GOTO、居中、拍摄
6. 如触发限位，先反方向脱困，再重新 Find Home
```

低空目标建议：

```text
Horizon Limit 推荐：:Sh+00#
需要少量余量：:Sh-02#
不建议长期使用：:Sh-10#
福州低空目标若仍报 outside limits，可在机械安全确认后考虑 AXIS1_LIMIT ±110 或 ±115
```

最终效果是：**开机不会溜车，未回零不会自动乱跑，手动仍可救机，撞限位后可以脱困，回零失败不会误解锁，N.I.N.A. / ASCOM 的 GOTO 与 Tracking 行为也更加可控**
