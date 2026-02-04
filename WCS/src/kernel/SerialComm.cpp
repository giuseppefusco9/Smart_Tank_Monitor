#include "SerialComm.h"

SerialComm::SerialComm() : inputBuffer("") {}

void SerialComm::init(unsigned long baudRate) {
    Serial.begin(baudRate);
    inputBuffer.reserve(JSON_BUFFER_SIZE);
}

void SerialComm::update() {
    while (Serial.available() > 0) {
        char c = (char)Serial.read();
        
        // Skip control characters like \r \n \t if at start of buffer to keep JSON clean
        if (inputBuffer.length() == 0 && (c == '\n' || c == '\r' || c == '\t' || c == ' ')) {
            continue;
        }
        
        inputBuffer += c;
        
        // Prevent buffer overflow
        if (inputBuffer.length() >= JSON_BUFFER_SIZE) {
            inputBuffer = "";  // Reset on overflow
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
    
    // Find the end of the first JSON object
    int endIdx = inputBuffer.indexOf('}');
    if (endIdx == -1) return false;
    
    // Extract the JSON string
    String jsonStr = inputBuffer.substring(0, endIdx + 1);
    
    // Parse JSON
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, jsonStr);
    
    if (error) {
        // Clear buffer on error to prevent being stuck
        inputBuffer = "";
        return false;
    }
    
    // Extract type (required)
    if (doc["type"].isNull()) {
        // Remove only this invalid segment
        inputBuffer = inputBuffer.substring(endIdx + 1);
        return false;
    }
    
    type = doc["type"].as<String>();
    
    // Flexible value extraction
    if (!doc["mode"].isNull() && !doc["valve"].isNull()) {
        // Build composite string for display updates: "MODE|VALVE"
        value = doc["mode"].as<String>() + "|" + doc["valve"].as<String>();
    } else if (!doc["value"].isNull()) {
        value = doc["value"].as<String>();
    } else if (!doc["mode"].isNull()) {
        value = doc["mode"].as<String>();
    } else if (!doc["valve"].isNull()) {
        value = doc["valve"].as<String>();
    } else {
        value = "";
    }
    
    // Remove only the processed message from buffer
    inputBuffer = inputBuffer.substring(endIdx + 1);
    
    // Trim potential leading whitespace/newlines for next message
    inputBuffer.trim();
    
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
