#include "WarningDetector.h"

bool WarningDetector::isWarning(const Packet& packet){
    //getType()
    std::string type = packet.getType();
    //getValue()
    double value = packet.getValue();

    // 조건 판단
    if(type == "TEMP"){
        return value > 80.0; //TEMP 80 초과하면 이상 판단
    }

    if(type == "SIGNAL"){
        return value < 20.0; //SIGNAL 20 미만시 이상 판단
    }

    // 일단 그 외 타입은 NORMAL 처리
    // 나중에 throw std::invalid_argument("Unknown packet type");으로 더 엄격하게 잡아주기.
    
    return false;
}