#include "SerialComm.h"

SerialComm::SerialComm() : inputBuffer("") {}

void SerialComm::init(unsigned long baudRate) {
    Serial.begin(baudRate);
    inputBuffer.reserve(JSON_BUFFER_SIZE);
}

void SerialComm::update() {
    while (Serial.available() > 0) {
        char c = Serial.read();
        
        if (c == '\n') {
            // Message complete, ready for parsing
            return;
        } else {
            inputBuffer += c;
            
            // Prevent buffer overflow
            if (inputBuffer.length() >= JSON_BUFFER_SIZE) {
                inputBuffer = "";  // Reset on overflow
            }
        }
    }
}

bool SerialComm::messageAvailable() {
    return inputBuffer.length() > 0 && inputBuffer.indexOf('}') >= 0;
}

bool SerialComm::receiveMessage(String& type, String& value) {
    if (!messageAvailable()) {
        return false;
    }
    
    // Parse JSON
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, inputBuffer);
    
    if (error) {
        // Clear buffer on error
        inputBuffer = "";
        return false;
    }
    
    // Extract type (required)
    if (doc["type"].isNull()) {
        inputBuffer = "";
        return false;
    }
    
    type = doc["type"].as<String>();
    
    // Flexible value extraction
    if (!doc["value"].isNull()) {
        value = doc["value"].as<String>();
    } else if (!doc["mode"].isNull()) {
        // Support display updates
        value = doc["mode"].as<String>();
    } else if (!doc["valve"].isNull()) {
        // Support display updates
        value = doc["valve"].as<String>();
    } else {
        value = "";
    }
    
    // Clear buffer after successful extraction
    inputBuffer = "";
    return true;
}

void SerialComm::sendMessage(const String& type, int value) {
    JsonDocument doc;
    doc["type"] = type;
    doc["value"] = value;
    
    serializeJson(doc, Serial);
    Serial.println();
    Serial.flush();
}
