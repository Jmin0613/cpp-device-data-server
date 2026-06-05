#pragma once

#include <string>

class Packet{
private :
    std::string deviceId;
    std::string timestamp;
    std::string type;
    double value;

public :
    // 생성자
    Packet(const std::string& deviceId, const std::string& timestamp,
        const std::string& type, double value);

    // Getter (read only)
    const std::string& getDeviceId() const;
    const std::string& getTimestamp() const;
    const std::string& getType() const;
    double getValue() const;

};