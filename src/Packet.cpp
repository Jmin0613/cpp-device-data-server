#include "Packet.h"

// 생성자
Packet::Packet(const std::string& deviceId, const std::string& timestamp, const std::string& type, double value)
: deviceId(deviceId), timestamp(timestamp), type(type), value(value) {}

// Getter (read only)
const std::string& Packet::getDeviceId() const{
    return deviceId;
}

const std::string& Packet::getTimestamp() const{
    return timestamp;
}

const std::string& Packet::getType() const{
    return type;
}

double Packet::getValue() const{
    return value;
}