#include <Arduino.h>
#include "config.h"
#include "kernel/scheduler.h"
#include "kernel/logger.h"
#include "kernel/MsgService.h"
#include "model/HWPlatform.h"
#include "tasks/WCSTask.h"

Scheduler sched;

HWPlatform *pHWPlatform;

void setup()
{
  MsgService.init();
  sched.init(75);
  Logger.log(":::::: WCS ::::::");

  pHWPlatform = new HWPlatform();
}

void loop()
{
  sched.schedule();
}