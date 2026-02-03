#ifndef __WCS_TASK__
#define __WCS_TASK__

#include "kernel/Task.h"
#include "model/HWPlatform.h"

class WCSTask: public Task {
public:
    WCSTask(ServoMotor* pServo, Lcd* pLcd, button* pButton, Potentiometer* pPot);
    void tick();

private:
    enum WCSState { AUTOMATIC, LOCAL_MANUAL, REMOTE_MANUAL, UNCONECTED } state;

    void checkUnconnectedMessage();
    void checkAutomaticMessage();
    void checkControlMessage();
    void checkRemoteMessage();
    void checkLocalMessage();
    void processPotentiometerInput();

    void setState(WCSState state);
    bool checkAndSetJustEntered();
    int msgMotorPerc(String message);

    HWPlatform* pHW;

    bool justEntered;
    int lastAngle;

    ServoMotor* pServo;
    Lcd* pLcd;
    button* pButton;
    Potentiometer* pPot;
};

#endif