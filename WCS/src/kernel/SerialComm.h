#ifndef __SERIAL_COMM__
#define __SERIAL_COMM__

#include <Arduino.h>
#include <ArduinoJson.h>

/**
 * Serial Communication Handler
 * Manages JSON-based communication with CUS via Serial
 */
class SerialComm {
private:
    static const size_t JSON_BUFFER_SIZE = 256;
    String inputBuffer;
    
public:
    SerialComm();
    
    /**
     * Initialize serial communication
     */
    void init(unsigned long baudRate);
    
    /**
     * Check if a complete message is available
     */
    bool messageAvailable();
    
    /**
     * Receive and parse JSON message
     * Returns true if message was successfully parsed
     */
    bool receiveMessage(String& type, String& value);
    
    /**
     * Send JSON message to CUS
     * Format: {"type": "...", "value": "..."}
     */
    void sendMessage(const String& type, const String& value);
    void sendMessage(const String& type, int value);
    
    /**
     * Process incoming serial data (call frequently)
     */
    void update();
};

#endif
