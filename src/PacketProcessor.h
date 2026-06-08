#pragma once

#include <string>

#include "Packet.h"
#include "Logger.h"

// 처리 결과 저장 
struct ProcessResult{
    Packet packet;
    bool warning;
};

// 처리 과정 
class PacketProcessor{
private :
    Logger logger;

public :
    // 생성자 - logFilePath 받아, logger에 넣어주기.
    explicit PacketProcessor(const std::string& logFilePath);

    // 데이터 처리
    ProcessResult process(const std::string& rawData);
};