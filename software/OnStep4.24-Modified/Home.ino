// -----------------------------------------------------------------------------------
// Functions related to Homing the mount
// 回原点相关的功能

#if (HOME_SENSE != OFF)
// 1.更新状态机枚举，必须包含 FH_IDLE2 和 FH_OFFSET
enum findHomeModes { FH_OFF, FH_FAST, FH_IDLE, FH_SLOW, FH_IDLE2, FH_OFFSET, FH_DONE };
findHomeModes findHomeMode = FH_OFF;
bool homeAbortRequested = false;
bool homeAbortRestoreSafetyLimits = true;
int PierSideStateAxis1 = LOW;
int PierSideStateAxis2 = LOW;
unsigned long findHomeTimeout = 0L;

// 2.新增：用于记录第三阶段（偏置）结束时间的变量
unsigned long offsetTimeoutAxis1 = 0L;
unsigned long offsetTimeoutAxis2 = 0L;

// 可选的第三阶段：仅在至少一轴设置了偏置时调用
bool startHomeOffset() {
  findHomeMode = FH_OFFSET;
  double secPerDeg = 3600.0 / (double)guideRates[HOME_OFFSET_RATE];
  CommandErrors e1 = CE_NONE;
  CommandErrors e2 = CE_NONE;

  if (HOME_OFFSET_AXIS1 != 0.0 && AXIS2_TANGENT_ARM == OFF) {
    char dir1 = (HOME_OFFSET_AXIS1 > 0) ? 'e' : 'w';
    unsigned long duration1 = (unsigned long)(fabs(HOME_OFFSET_AXIS1) * secPerDeg * 1000.0);
    e1 = startGuideAxis1(dir1, HOME_OFFSET_RATE, 0, false);
    if (e1 == CE_NONE) offsetTimeoutAxis1 = millis() + duration1; else offsetTimeoutAxis1 = 0;
  } else {
    offsetTimeoutAxis1 = 0;
  }

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
    generalError = ERR_LIMIT_SENSE;
    stopSlewingAndTracking(SS_ALL_FAST);
    VLF("MSG: Homing phase 3 failed");
    return false;
  }

  if (offsetTimeoutAxis1 == 0 && offsetTimeoutAxis2 == 0) {
    findHomeMode = FH_DONE;
  } else {
    VLF("MSG: Homing started phase 3 (Zero Offset)");
  }
  return true;
}

void checkHome() {
  // 人工停止或硬故障一旦请求取消 Home，就只负责让双轴安全停稳
  // 取消路径不得再经过任何正常阶段转换，更不能进入 FH_DONE 恢复位置可信状态
  if (homeAbortRequested) {
    stopGuideAxis1();
    stopGuideAxis2();
    offsetTimeoutAxis1 = 0;
    offsetTimeoutAxis2 = 0;

    if (guideDirAxis1 == 0 && guideDirAxis2 == 0) {
      findHomeMode = FH_OFF;
      homeAbortRequested = false;
      findHomeTimeout = 0;
      safetyLimitsOn = homeAbortRestoreSafetyLimits;
      homeAbortRestoreSafetyLimits = true;
      trackingState = TrackingNone;
      lastTrackingState = TrackingNone;
      abortTrackingState = TrackingNone;
      trackingSyncSeconds = 0;
      gotoAbortState = GOTO_ABORT_NONE;
      gotoStartTrackingOnSuccess = false;
      atHome = false;
      VLF("MSG: Homing aborted; Home/Set Home required");
    }
    return;
  }

  // 1. 第一/第二阶段的超时与错误检测
  if (findHomeMode == FH_FAST || findHomeMode == FH_SLOW) {
    if ((long)(millis()-findHomeTimeout) > 0L || (guideDirAxis1 == 0 && guideDirAxis2 == 0)) {
      if ((long)(millis()-findHomeTimeout) > 0L) generalError=ERR_LIMIT_SENSE;
      if (guideDirAxis1 == 'e' || guideDirAxis1 == 'w') guideDirAxis1='b';
      if (guideDirAxis2 == 'n' || guideDirAxis2 == 's') guideDirAxis2='b';
      safetyLimitsOn=true;
      // 传感器回零未完成时不能继续信任开环步数位置
      invalidatePositionReference();
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
  // 3. 第二阶段停稳后：零偏置直接完成，否则启动独立的偏置阶段
  // =======================================================================
  if (findHomeMode == FH_IDLE2 && guideDirAxis1 == 0 && guideDirAxis2 == 0) {
    if (HOME_OFFSET_AXIS1 == 0.0 && HOME_OFFSET_AXIS2 == 0.0) {
      // 不进入 FH_OFFSET，不读取偏置速度，也不启动偏置运动
      offsetTimeoutAxis1 = 0;
      offsetTimeoutAxis2 = 0;
      findHomeMode = FH_DONE;
    } else if (!startHomeOffset()) {
      return;
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

    // 真实回原点完成后重新建立可信坐标基准；沿用原版 OnStep 的限位时序，
    // 自动 Home 成功后保持坐标软件限位关闭，直到原版路径再次启用
    completePositionRecovery();
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

// 取消正在执行的 Home，停止过程中以及停稳后都保持位置参考不可信
// 只有重新完整 Find Home 或用户确认机械位置后 Set Home 才能解除普通 GOTO 锁
void requestHomeAbort(bool restoreSafetyLimits) {
  bool homingActive = homeMount;

#if HOME_SENSE != OFF
  if (findHomeMode != FH_OFF) {
    if (!homeAbortRequested) homeAbortRestoreSafetyLimits = restoreSafetyLimits;
    else homeAbortRestoreSafetyLimits = homeAbortRestoreSafetyLimits && restoreSafetyLimits;
    homeAbortRequested = true;
    offsetTimeoutAxis1 = 0;
    offsetTimeoutAxis2 = 0;
    homingActive = true;
  }
#endif

  if (!homingActive) return;

  invalidatePositionReference();
  gotoStartTrackingOnSuccess = false;
  abortTrackingState = TrackingNone;
  trackingSyncSeconds = 0;
  atHome = false;
  if (trackingState == TrackingMoveTo) gotoAbortState = GOTO_ABORT_POSITION_LOST;
  stopGuideAxis1();
  stopGuideAxis2();
  VLF("MSG: Homing abort requested");
}

// moves telescope to the home position, then stops tracking
// 将望远镜移回初始位置，然后停止跟踪
CommandErrors goHome(bool fast) {
#if LIMIT_SENSE != OFF
  // 物理限位恢复必须先由用户手动移出限位区域
  // 限位输入释放并稳定 500 ms 后才允许 Home，ERR_LIMIT_SENSE 保留到 Home/Set Home 成功
  const bool physicalLimitActive = digitalRead(LimitPin) == LIMIT_SENSE_STATE;
  const bool physicalLimitReleaseSettling =
    generalError == ERR_LIMIT_SENSE && (unsigned long)(millis() - lastLimitTriggerTime) <= 500UL;
  if (physicalLimitActive || physicalLimitReleaseSettling) return CE_SLEW_ERR_OUTSIDE_LIMITS;
#endif

  // 恢复性 Home 可覆盖残留的 Parked 标记
#if HOME_SENSE == OFF
  const bool homeMayOverridePark = positionHomeReturnOnly();
#else
  const bool homeMayOverridePark = !positionReady();
#endif
  if (parkStatus == Parked && homeMayOverridePark) {
    parkStatus=NotParked;
    nv.write(EE_parkStatus,parkStatus);
  }

  CommandErrors e=validateGoto();

#if HOME_SENSE == OFF
  // 无 Home 传感器时，保留的可信坐标只能用于返回 Home
  const bool coordinateHomeSafe =
    parkStatus == NotParked && !trackingSyncInProgress() &&
    trackingState != TrackingMoveTo && guideDirAxis1 == 0 && guideDirAxis2 == 0 &&
    !faultAxis1 && !faultAxis2;
  if (e == CE_SLEW_ERR_IN_STANDBY && coordinateHomeSafe && positionHomeReturnOnly()) {
    enableStepperDrivers();
    e=CE_NONE;
  }
#endif
  
#if HOME_SENSE != OFF
  if (e != CE_NONE && e != CE_SLEW_ERR_IN_STANDBY) return e;
  // 自动回零是恢复 standby/位置不可信状态的合法通道
  if (e == CE_SLEW_ERR_IN_STANDBY) e = CE_NONE;

  if (findHomeMode != FH_OFF) return CE_MOUNT_IN_MOTION;

  homeAbortRequested=false;
  homeAbortRestoreSafetyLimits=true;

  // stop tracking
  // 停止跟踪
  abortTrackingState=trackingState;
  trackingState=TrackingNone;

  // 自动回零开始后，直到 FH_DONE 都不再信任原开环坐标
  invalidatePositionReference();
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
    // 默认9档时与原版超时一致；改变速度后仍保持约360度的搜索余量
    double secPerDeg=3600.0/(double)guideRates[HOME_FAST_RATE];
    findHomeTimeout=millis()+(unsigned long)(secPerDeg*360.0*1000.0);
    
    // 8=HalfMaxRate半速，9＝全速
    if (AXIS2_TANGENT_ARM == OFF) e=startGuideAxis1(a1,HOME_FAST_RATE,0,false);
    if (e == CE_NONE) e=startGuideAxis2(a2,HOME_FAST_RATE,0,false,true);
    if (e == CE_NONE) VLF("MSG: Homing started phase 1"); else VLF("MSG: Homing start phase 1 failed");
  } else {
    findHomeMode=FH_SLOW;
    // 默认7档时为原来的30秒；改变速度后仍保持约6度的精找范围
    double secPerDeg=3600.0/(double)guideRates[HOME_SLOW_RATE];
    findHomeTimeout=millis()+(unsigned long)(secPerDeg*6.0*1000.0);
    
    // 7=48x sidereal，8=HalfMaxRate半速
    if (AXIS2_TANGENT_ARM == OFF) e=startGuideAxis1(a1,HOME_SLOW_RATE,0,false);
    if (e == CE_NONE) e=startGuideAxis2(a2,HOME_SLOW_RATE,0,false,true);
    if (e == CE_NONE) VLF("MSG: Homing started phase 2"); else VLF("MSG: Homing start phase 2 failed");
  }
  if (e != CE_NONE) {
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
#if LIMIT_SENSE != OFF
  // 物理限位释放前禁止重置坐标和清除限位方向锁
  if (digitalRead(LimitPin) == LIMIT_SENSE_STATE) return CE_SLEW_ERR_OUTSIDE_LIMITS;
#endif

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
  // 因此无论是否安装 HOME_SENSE 都可重新建立可信坐标基准
  completePositionRecovery();

  // 清理上一次安全中断或限位导致的残留状态
  abortGoto = 0;
  generalError = ERR_NONE;

  return CE_NONE;
}
