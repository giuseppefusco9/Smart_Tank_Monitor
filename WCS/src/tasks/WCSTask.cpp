#include "WCSTask.h"
#include "Arduino.h"
#include "kernel/MsgService.h"
#include "kernel/logger.h"

#define UNCONECTED_MSG "cus->wcs-st-unconnected"
#define AUTOMATIC_MSG "cus->wcs-st-automatic"
#define REMOTE_MANUAL_MSG "cus->wcs-st-remote-manual"
#define LOCAL_MANUAL_MSG "cus->wcs-st-local-manual"
#define CONTROL_MSG "cus->wcs-op-"

#define LOCAL_MANUAL_SEND_MSG "wcs->cus-st-local-manual"
#define AUTOMATIC_SEND_MSG "wcs->cus-st-automatic"
#define CONTROL_SEND_MSG "wcs->cus-op-"

#define POTENTIOMETER_MIN 0
#define POTENTIOMETER_MAX 1023

#define PERCENTAGE_MIN 0
#define PERCENTAGE_MAX 100

class AutomaticPattern : public Pattern {
public:
  boolean match(const Msg& m) override {
    return m.getContent() == AUTOMATIC_MSG;
  }
};

class UnconectedPattern : public Pattern {
public:
  boolean match(const Msg& m) override {
    return m.getContent() == UNCONECTED_MSG;
  }
};

class RemotePattern : public Pattern {
public:
  boolean match(const Msg& m) override {
    return m.getContent() == REMOTE_MANUAL_MSG;
  }
};

class LocalPattern : public Pattern {
public:
  boolean match(const Msg& m) override {
    return m.getContent() == LOCAL_MANUAL_MSG;
  }
};

class ControlPattern : public Pattern {
public:
  boolean match(const Msg& m) override {
    return m.getContent().startsWith(CONTROL_MSG);
  }
};

WCSTask::WCSTask(ServoMotor* pServo, Lcd *pLcd, button *pButton, Potentiometer* pPot) : 
    pServo(pServo), pLcd(pLcd), pButton(pButton), pPot(pPot)
{
    this->setState(AUTOMATIC);    
}

void WCSTask::tick(){
   switch(state){
     case AUTOMATIC:
        if (checkAndSetJustEntered())
        {
            Logger.log(F("WCSTask:AUTOMATIC"));
            pLcd->writeModeMessage("AUTOMATIC");
        }
        this->checkUnconnectedMessage();
        this->checkControlMessage();    
        this->checkRemoteMessage();
        this->checkLocalMessage();

        break;
     case LOCAL_MANUAL:
        if (checkAndSetJustEntered())
        {
            this->lastAngle = -1;
            Logger.log(F("WCSTask:LOCAL_MANUAL"));
            pLcd->writeModeMessage("LOCAL MANUAL");
        }

        
        this->checkUnconnectedMessage();
        this->checkAutomaticMessage();

        if(pButton->isPressed())
        {
            MsgService.sendMsg(AUTOMATIC_SEND_MSG);  
        } 

        this->processPotentiometerInput();

        break;
     case REMOTE_MANUAL:
        {
                if (checkAndSetJustEntered())
            {
                Logger.log(F("WCSTask:REMOTE_MANUAL"));
                pLcd->writeModeMessage("REMOTE MANUAL");
            }

            this->checkUnconnectedMessage();
            this->checkAutomaticMessage();
            this->checkControlMessage();
    
        break;
        }
        break;
     case UNCONECTED:
       {
            if (checkAndSetJustEntered())
            {
                Logger.log(F("WCSTask:UNCONECTED"));
                pLcd->writeModeMessage("UNCONECTED");
            }

            this->checkAutomaticMessage();

            break;
        }   
    default:
        break;
    
   }
}

void WCSTask::checkUnconnectedMessage(){
    static UnconectedPattern unconected;
    if(MsgService.isMsgAvailable(unconected))
    {
        Msg* msg = MsgService.receiveMsg(unconected);
        delete msg;
        this->setState(UNCONECTED);
    }
}

void WCSTask::checkAutomaticMessage(){
    static AutomaticPattern automatic;
    if(MsgService.isMsgAvailable(automatic))
    {
        Msg* msg = MsgService.receiveMsg(automatic);
        delete msg;
        this->setState(AUTOMATIC);
    }
}

void WCSTask::checkControlMessage(){
    static ControlPattern control;
    if(MsgService.isMsgAvailable(control))
    {
        Logger.log(F("Servo angle is chaning"));
        Msg* msg = MsgService.receiveMsg(control);
        int perc = this->msgMotorPerc(msg->getContent());
        int angle = map(perc, PERCENTAGE_MIN, PERCENTAGE_MAX, 0, 180);
        this->pServo->setPosition(angle);
        this->pLcd->writePercMessage(String(perc));
        delete msg;
    }
}

void WCSTask::checkRemoteMessage(){
    static RemotePattern remote;
    if(MsgService.isMsgAvailable(remote))
    {
        Msg* msg = MsgService.receiveMsg(remote);
        delete msg;
        this->setState(REMOTE_MANUAL);
    }
}

void WCSTask::checkLocalMessage(){
    static LocalPattern local;
    if(MsgService.isMsgAvailable(local))
    {
        Msg* msg = MsgService.receiveMsg(local);
        delete msg;
        this->setState(LOCAL_MANUAL);
    }
}

void WCSTask::processPotentiometerInput(){
    pPot->sync();
    int potValue = pPot->getValue();
    int angle = map(potValue, POTENTIOMETER_MIN, POTENTIOMETER_MAX, 0, 180);
    String percentageString = String(map(angle, 0, 180, 0, 100));
    pServo->setPosition(angle);
    String angleString = String(angle);
    if (this->lastAngle != angle) {
        this->lastAngle = angle;
        pLcd->writePercMessage(percentageString);
    }
    MsgService.sendMsg(CONTROL_SEND_MSG+percentageString);
}

void WCSTask::setState(WCSState s)
{
    state = s;
    justEntered = true;
}

bool WCSTask::checkAndSetJustEntered()
{
    bool bak = justEntered;
    if (justEntered)
    {
        justEntered = false;
    }
    return bak;
}

int WCSTask::msgMotorPerc(String m)
{
    return m.substring(String(CONTROL_MSG).length()).toInt();
}