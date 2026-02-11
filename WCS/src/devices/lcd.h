#ifndef MYLCD_H
#define MYLCD_H

#include <Arduino.h>

class Lcd {
private:
public:
    Lcd();
    void writeModeMessage(String message);
    void writePercMessage(String message);
    void writeMessage(String message);
private:
    void refresh();

    String modeMessage;
    String percMessage;
    String msg;
};

#endif