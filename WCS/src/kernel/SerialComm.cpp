#include "SerialComm.h"

SerialComm::SerialComm() : inputBuffer("") {}

void SerialComm::init(unsigned long baudRate) {
    Serial.begin(baudRate);
    inputBuffer.reserve(JSON_BUFFER_SIZE);
}

void SerialComm::update() {
    while (Serial.available() > 0) {
        char c = (char)Serial.read();
        
        if (inputBuffer.length() == 0 && (c == '\n' || c == '\r' || c == '\t' || c == ' ')) {
            continue;
        }
            
        inputBuffer += c;
        
        if (inputBuffer.length() >= JSON_BUFFER_SIZE) {
            inputBuffer = "";  
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
    
    int endIdx = inputBuffer.indexOf('}');
    if (endIdx == -1) return false;
    
    String jsonStr = inputBuffer.substring(0, endIdx + 1);
    
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, jsonStr);
    
    if (error) {
        inputBuffer = "";
        return false;
    }
    
    if (doc["type"].isNull()) {
        inputBuffer = inputBuffer.substring(endIdx + 1);
        return false;
    }
    
    type = doc["type"].as<String>();
    
    if (!doc["mode"].isNull() && !doc["valve"].isNull()) {
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
    
    inputBuffer = inputBuffer.substring(endIdx + 1);
    
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

void SerialComm::sendMessage(const String& type, const String& value) {
    JsonDocument doc;
    doc["type"] = type;
    doc["value"] = value;
    
    serializeJson(doc, Serial);
    Serial.println(); 
    Serial.flush();
}
