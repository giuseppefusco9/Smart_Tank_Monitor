#include "Arduino.h"
#include "Scheduler.h"

Scheduler::Scheduler(int basePeriod) 
  : nTasks(0), basePeriod(basePeriod) {
}

void Scheduler::init(int basePeriod) {
  this->basePeriod = basePeriod;
  nTasks = 0;
}

bool Scheduler::addTask(Task* task) {
  if (nTasks < MAX_TASKS - 1) {
    taskList[nTasks] = task;
    nTasks++;
    return true;
  } else {
    return false;
  }
}

void Scheduler::schedule() {
  for (int i = 0; i < nTasks; i++) {
    if (taskList[i]->isActive()) {
      if (taskList[i]->isPeriodic()) {
        if (taskList[i]->updateAndCheckTime(basePeriod)) {
          taskList[i]->tick();
        }
      } else {
        taskList[i]->tick();
        if (taskList[i]->isCompleted()) {
          taskList[i]->setActive(false);
        }
      }
    }
  }
}

int Scheduler::getBasePeriod() const {
  return basePeriod;
}

int Scheduler::getNumTasks() const {
  return nTasks;
}
