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
    
    // Extract type and value (using ArduinoJson v7 API)
    if (!doc["type"].isNull() && !doc["value"].isNull()) {
        type = doc["type"].as<String>();
        value = doc["value"].as<String>();
        
        // Clear buffer after successful parse
        inputBuffer = "";
        return true;
    }
    
    // Clear buffer if format is wrong
    inputBuffer = "";
    return false;
}

void SerialComm::sendMessage(const String& type, const String& value) {
    JsonDocument doc;
    doc["type"] = type;
    doc["value"] = value;
    
    serializeJson(doc, Serial);
    Serial.println();  // Add newline
    Serial.flush();    // Ensure message is sent
}

void SerialComm::sendMessage(const String& type, int value) {
    JsonDocument doc;
    doc["type"] = type;
    doc["value"] = value;
    
    serializeJson(doc, Serial);
    Serial.println();
    Serial.flush();
}
