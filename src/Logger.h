#pragma once

#include <string>
#include <mutex>

#include "Packet.h"

class Logger{
private :
    std::string logFilePath; //로그 파일 경로
    std::mutex logMutex; // 로그 mutex (자물쇠)

public :
    // 생성자
    explicit Logger(const std::string& logFilePath);

    // 복사 막기
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    // 로그 작성
    void write(const Packet& packet, bool isWarning);
};