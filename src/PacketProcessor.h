#pragma once

#include <string>

#include "Packet.h"
#include "Logger.h"
#include "StatisticsCollector.h"

// 처리 결과 저장 
struct ProcessResult{
    Packet packet;
    bool warning;
};

// 처리 과정 
class PacketProcessor{
private :
    Logger logger;
    StatisticsCollector& statistics; // 통계는 서버 전체에서 하나의 공용 저장소여야 하기에 & 참조자

public :
    // 생성자 - logFilePath 받아, logger에 넣어주기. 통계 수집을 위해 공용 저장소 받아오기.
    explicit PacketProcessor(const std::string& logFilePath, StatisticsCollector& statistics);

    // 데이터 처리
    ProcessResult process(const std::string& rawData);
};