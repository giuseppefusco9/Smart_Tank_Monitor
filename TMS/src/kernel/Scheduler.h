#ifndef __SCHEDULER__
#define __SCHEDULER__

#include "Task.h"

#define MAX_TASKS 10

/**
 * Task Scheduler
 * Manages and executes multiple tasks based on their periods
 */
class Scheduler {
private:
  Task* taskList[MAX_TASKS];
  int nTasks;
  int basePeriod;

public:
  Scheduler(int basePeriod = 10);

  /**
   * Initialize scheduler with base period
   */
  virtual void init(int basePeriod);

  /**
   * Add a task to the scheduler
   * Returns: true if added successfully, false if task list is full
   */
  virtual bool addTask(Task* task);

  /**
   * Execute one scheduler cycle
   * Checks all tasks and executes them if their period has elapsed
   */
  virtual void schedule();

  /**
   * Get base period in milliseconds
   */
  int getBasePeriod() const;

  /**
   * Get number of registered tasks
   */
  int getNumTasks() const;
};

#endif
