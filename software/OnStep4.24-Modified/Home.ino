// -----------------------------------------------------------------------------------
// Functions related to Homing the mount
// 回原点相关的功能

#if (HOME_SENSE != OFF)
// 【修改点 1】更新状态机枚举，必须包含 FH_IDLE2 和 FH_OFFSET
enum findHomeModes { FH_OFF, FH_FAST, FH_IDLE, FH_SLOW, FH_IDLE2, FH_OFFSET, FH_DONE };
findHomeModes findHomeMode = FH_OFF;
int PierSideStateAxis1 = LOW;
int PierSideStateAxis2 = LOW;
unsigned long findHomeTimeout = 0L;

// 【修改点 2】新增：用于记录第三阶段（偏置）结束时间的变量
unsigned long offsetTimeoutAxis1 = 0L;
unsigned long offsetTimeoutAxis2 = 0L;

void checkHome() {
  // 1. 第一/第二阶段的超时与错误检测
  if (findHomeMode == FH_FAST || findHomeMode == FH_SLOW) {
    if ((long)(millis()-findHomeTimeout) > 0L || (guideDirAxis1 == 0 && guideDirAxis2 == 0)) {
      if ((long)(millis()-findHomeTimeout) > 0L) generalError=ERR_LIMIT_SENSE;
      if (guideDirAxis1 == 'e' || guideDirAxis1 == 'w') guideDirAxis1='b';
      if (guideDirAxis2 == 'n' || guideDirAxis2 == 's') guideDirAxis2='b';
      safetyLimitsOn=true;
      // 传感器回零未完成时不能继续信任开环步数位置。
      mountPositionTrusted = false;
      positionRecoveryRequired = true;
      gotoAbortState = GOTO_ABORT_NONE;
      findHomeMode=FH_OFF;
    } else {
      if (digitalRead(Axis1_HOME) != PierSideStateAxis1 && (guideDirAxis1 == 'e' || guideDirAxis1 == 'w')) StopAxis1();
      if (digitalRead(Axis2_HOME) != PierSideStateAxis2 && (guideDirAxis2 == 'n' || guideDirAxis2 == 's')) StopAxis2();
    }
  }

  // 2. 快速过渡到慢速的等待阶段
  if (findHomeMode == FH_IDLE && guideDirAxis1 == 0 && guideDirAxis2 == 0) {
    findHomeMode=FH_OFF;
    goHome(false);
  }

  // =======================================================================
  // 3. 第三阶段启动：第二阶段停稳后，使用 Config.h 中选定的速度档位计算时间并启动
  // =======================================================================
  if (findHomeMode == FH_IDLE2 && guideDirAxis1 == 0 && guideDirAxis2 == 0) {
    findHomeMode = FH_OFFSET; // 进入偏置阶段

    // 第三阶段只属于 HOME_SENSE 自动回零；这里已经位于 #if HOME_SENSE != OFF 内。
    // 使用 Config.h 中选定的速度档位计算时间，不额外修改 Config.h。
    double secPerDeg = 3600.0 / (double)guideRates[HOME_OFFSET_RATE];
    CommandErrors e1 = CE_NONE;
    CommandErrors e2 = CE_NONE;

    // Axis 1 (赤经) 偏置计算与启动
    if (HOME_OFFSET_AXIS1 != 0.0 && AXIS2_TANGENT_ARM == OFF) {
      char dir1 = (HOME_OFFSET_AXIS1 > 0) ? 'e' : 'w';
      unsigned long duration1 = (unsigned long)(fabs(HOME_OFFSET_AXIS1) * secPerDeg * 1000.0);
      e1 = startGuideAxis1(dir1, HOME_OFFSET_RATE, 0, false);
      if (e1 == CE_NONE) offsetTimeoutAxis1 = millis() + duration1; else offsetTimeoutAxis1 = 0;
    } else {
      offsetTimeoutAxis1 = 0;
    }

    // Axis 2 (赤纬) 偏置计算与启动
    if (HOME_OFFSET_AXIS2 != 0.0) {
      char dir2 = (HOME_OFFSET_AXIS2 > 0) ? 'n' : 's';
      unsigned long duration2 = (unsigned long)(fabs(HOME_OFFSET_AXIS2) * secPerDeg * 1000.0);
      e2 = startGuideAxis2(dir2, HOME_OFFSET_RATE, 0, false, true);
      if (e2 == CE_NONE) offsetTimeoutAxis2 = millis() + duration2; else offsetTimeoutAxis2 = 0;
    } else {
      offsetTimeoutAxis2 = 0;
    }

    if (e1 != CE_NONE || e2 != CE_NONE) {
      findHomeMode = FH_OFF;
      safetyLimitsOn = true;
      generalError = ERR_LIMIT_SENSE;
      stopSlewingAndTracking(SS_ALL_FAST);
      VLF("MSG: Homing phase 3 failed");
      return;
    }

    if (offsetTimeoutAxis1 == 0 && offsetTimeoutAxis2 == 0) {
      findHomeMode = FH_DONE;
    } else {
      VLF("MSG: Homing started phase 3 (Zero Offset)");
    }
  }

  // =======================================================================
  // 4. 第三阶段运行：时间检测与单独刹车
  // =======================================================================
  if (findHomeMode == FH_OFFSET) {
    // 检查 Axis 1 是否达到偏置时间
    if (offsetTimeoutAxis1 > 0 && (long)(millis() - offsetTimeoutAxis1) > 0L) {
      guideDirAxis1 = 'b'; 
      offsetTimeoutAxis1 = 0; 
    }
    // 检查 Axis 2 是否达到偏置时间
    if (offsetTimeoutAxis2 > 0 && (long)(millis() - offsetTimeoutAxis2) > 0L) {
      guideDirAxis2 = 'b'; 
      offsetTimeoutAxis2 = 0; 
    }
    
    // 双轴都到达定时且完全停稳后，才进入 DONE 阶段
    if (offsetTimeoutAxis1 == 0 && offsetTimeoutAxis2 == 0 && guideDirAxis1 == 0 && guideDirAxis2 == 0) {
      findHomeMode = FH_DONE;
    }
  }

  // =======================================================================
  // 5. 最终完成阶段 (设定内部坐标原点)
  // =======================================================================
  if (findHomeMode == FH_DONE && guideDirAxis1 == 0 && guideDirAxis2 == 0) {
    findHomeMode=FH_OFF;
    VLF("MSG: Homing done");
    #if AXIS2_TANGENT_ARM == ON
      trackingState=abortTrackingState;
      cli();
      targetAxis2.part.m = 0; targetAxis2.part.f = 0;
      posAxis2           = 0;
      sei();
    #else    
      initStartPosition(); // 此时望远镜处于“传感器位置 + 偏置量”处，以此处作为真正的绝对零位
      atHome=true;
    #endif

    // 真实回原点完成后，所有结构类型都重新建立可信坐标基准。
    mountPositionTrusted = true;
    positionRecoveryRequired = false;
    gotoAbortState = GOTO_ABORT_NONE;
#if LIMIT_SENSE != OFF
    // 只有启用 LIMIT_SENSE 时才清理限位方向锁。
    Axis1_LimitLock = 0;
    Axis2_LimitLock = 0;
#endif
    abortGoto = 0;
    lastTrackingState = TrackingNone;
    abortTrackingState = TrackingNone;
    generalError = ERR_NONE;
  }
}
void StopAxis1() {
  guideDirAxis1='b';
  VLF("MSG: Homing switch detected, stopping guide on Axis1");
  if (guideDirAxis2 != 'n' && guideDirAxis2 != 's') { 
    if (findHomeMode == FH_SLOW) findHomeMode = FH_IDLE2; // 修改点：指向 IDLE2
    if (findHomeMode == FH_FAST) findHomeMode = FH_IDLE;
  }
}

void StopAxis2() {
  guideDirAxis2='b';
  VLF("MSG: Homing switch detected, stopping guide on Axis2");
  if (guideDirAxis1 != 'e' && guideDirAxis1 != 'w') { 
    if (findHomeMode == FH_SLOW) findHomeMode = FH_IDLE2; // 修改点：指向 IDLE2
    if (findHomeMode == FH_FAST) findHomeMode = FH_IDLE;
  }
}
#endif

// moves telescope to the home position, then stops tracking
// 将望远镜移回初始位置，然后停止跟踪
CommandErrors goHome(bool fast) {
  CommandErrors e=validateGoto();
  
#if HOME_SENSE != OFF
  if (e != CE_NONE && e != CE_SLEW_ERR_IN_STANDBY) return e;
  // 自动回零是恢复 standby/位置不可信状态的合法通道。
  if (e == CE_SLEW_ERR_IN_STANDBY) e = CE_NONE;

  if (findHomeMode != FH_OFF) return CE_MOUNT_IN_MOTION;

  // stop tracking
  // 停止跟踪
  abortTrackingState=trackingState;
  trackingState=TrackingNone;

  // 自动回零开始后，直到 FH_DONE 都不再信任原开环坐标。
  mountPositionTrusted = false;
  positionRecoveryRequired = true;
  gotoAbortState = GOTO_ABORT_NONE;

  // decide direction to guide
  // 决定引导方向
  char a1; if (digitalRead(Axis1_HOME) == HOME_SENSE_STATE_AXIS1) a1='w'; else a1='e';
  char a2; if (digitalRead(Axis2_HOME) == HOME_SENSE_STATE_AXIS2) a2='n'; else a2='s';

  // attach interrupts to stop guide
  // 附加中断以停止引导
  PierSideStateAxis1=digitalRead(Axis1_HOME);
  PierSideStateAxis2=digitalRead(Axis2_HOME);
  
  // disable limits
  // 禁用限制
  safetyLimitsOn=false;
  
  // start guides
  // 开始引导
  if (fast) {
    #if AXIS2_TANGENT_ARM == OFF
      // make sure tracking is disabled
      // 确认追踪禁止
      trackingState=TrackingNone;
    #endif

    // make sure motors are powered on
    // 确认电机上电
    enableStepperDrivers();

    findHomeMode=FH_FAST;
    double secPerDeg=3600.0/(double)guideRates[8];
    findHomeTimeout=millis()+(unsigned long)(secPerDeg*180.0*1000.0);
    
    // 8=HalfMaxRate半速，9＝全速
    if (AXIS2_TANGENT_ARM == OFF) e=startGuideAxis1(a1,9,0,false);
    if (e == CE_NONE) e=startGuideAxis2(a2,9,0,false,true);
    if (e == CE_NONE) VLF("MSG: Homing started phase 1"); else VLF("MSG: Homing start phase 1 failed");
  } else {
    findHomeMode=FH_SLOW;
    findHomeTimeout=millis()+30000UL;
    
    // 7=48x sidereal，8=HalfMaxRate半速
    if (AXIS2_TANGENT_ARM == OFF) e=startGuideAxis1(a1,7,0,false);
    if (e == CE_NONE) e=startGuideAxis2(a2,7,0,false,true);
    if (e == CE_NONE) VLF("MSG: Homing started phase 2"); else VLF("MSG: Homing start phase 2 failed");
  }
  if (e != CE_NONE) {
    findHomeMode = FH_OFF;
    safetyLimitsOn = true;
    stopSlewingAndTracking(SS_ALL_FAST);
  }
  return e;
#else
  if (e != CE_NONE) return e; 

  abortTrackingState=trackingState;

  #if AXIS2_TANGENT_ARM == ON
    double h=getInstrAxis1();
    double i2=indexAxis2;
    int p=getInstrPierSide();
    if (latitude >= 0) { if (p == PierSideWest) i2=180.0-i2; } else { if (p == PierSideWest) i2=-180.0-i2; }
    e=goTo(h,i2,h,i2,p);
  #else
    trackingState=TrackingNone;
    e=goTo(homePositionAxis1,homePositionAxis2,homePositionAxis1,homePositionAxis2,PierSideEast);
  #endif

  if (e == CE_NONE) { VLF("MSG: Homing started"); homeMount=true; } else { VLF("MSG: Homing failed"); trackingState=abortTrackingState; }
  return e;
#endif
}

bool isHoming() {
#if HOME_SENSE != OFF
  return (homeMount || (findHomeMode != FH_OFF));
#else
  return homeMount;
#endif
}

// sets telescope home position; user manually moves to Hour Angle 90 and Declination 90 (CWD position),
// 设置望远镜的初始位置；用户手动移动到时角 90 度、赤纬 90 度（CWD 位置），
// then the first gotoEqu will set the pier side and turn on tracking
// 然后，第一个 gotoEqu 函数会设置码头侧并启用跟踪功能
CommandErrors setHome() {
  if (isSlewing()) return CE_MOUNT_IN_MOTION;

  // back to startup state
  reactivateBacklashComp();
  initStartupValues();
  initStartPosition();

  currentAlt=45.0;
  doFastAltCalc(true);

  safetyLimitsOn=true;

  // initialize and disable the stepper drivers
  // 初始化并禁用步进电机驱动器
  StepperModeTrackingInit();
 
  // not parked, but don't wipe the park position if it's saved - we can still use it
  // 未停车，但如果已保存停车位置信息，请不要清除我们仍然可以使用它
  parkStatus=NotParked;
  nv.write(EE_parkStatus,parkStatus);
  
  // reset PEC, unless we have an index to recover from this
  // 除非我们有索引可以从中恢复，否则请重置PEC
  pecRecorded=nv.read(EE_pecRecorded);
  if (pecRecorded != true && pecRecorded != false) { pecRecorded=false; DLF("ERR, setHome(): bad NV pecRecorded"); }
  #if PEC_SENSE == OFF
    pecStatus=IgnorePEC;
    nv.write(EE_pecStatus,pecStatus);
  #else
    pecStatus=nv.read(EE_pecStatus);
    if (pecStatus < PEC_STATUS_FIRST || pecStatus > PEC_STATUS_LAST) { pecStatus=IgnorePEC; DLF("ERR, setHome(): bad NV pecStatus"); }
  #endif
  if (!pecRecorded) pecStatus=IgnorePEC;

  // Set Home 的语义是用户确认机械位置与定义的 Home 坐标一致，
  // 因此无论是否安装 HOME_SENSE 都可重新建立可信坐标基准。
  mountPositionTrusted = true;
  positionRecoveryRequired = false;
  gotoAbortState = GOTO_ABORT_NONE;

#if LIMIT_SENSE != OFF
  // 有 LIMIT_SENSE 时，手动 setHome 后清除物理限位方向锁。
  Axis1_LimitLock = 0;
  Axis2_LimitLock = 0;
#endif

  // 清理上一次安全中断或限位导致的残留状态。
  abortGoto = 0;
  generalError = ERR_NONE;

  return CE_NONE;
}
