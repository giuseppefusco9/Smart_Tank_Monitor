#include "logger.h"
#include "MsgService.h"

void LoggerService::log(const String& msg){
    MsgService.sendMsg("wcs->cus-lo:"+msg);
}