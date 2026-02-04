#include "logger.h"
#include "MsgService.h"

void LoggerService::log(const String& msg){
    // DISABLED: This interferes with SerialComm JSON protocol
    // MsgService.sendMsg("wcs->cus-lo:"+msg);
    
    // Logger is now silent - SerialComm handles all communication with CUS
}