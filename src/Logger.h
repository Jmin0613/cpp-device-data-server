#pragma once

#include <string>
#include "Packet.h"

class Logger{
private :
    std::string logFilePath; //로그 파일 경로

public :
    // 생성자
    Logger(const std::string& logFilePath);

    // 로그 작성
    void write(const Packet& packet, bool isWarning);
};