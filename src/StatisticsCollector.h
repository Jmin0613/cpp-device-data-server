#pragma once

#include <mutex>
#include <string>
#include <unordered_map>

#include "Packet.h"

// type별 통계 저장
struct TypeStatistics{
    int count = 0;
    double sum = 0.0;
    double max = 0.0;
    double min = 0.0;
};

// snapshot 결과 저장 
struct StatisticsSnapshot{
    int totalCount = 0;
    int normalCount = 0;
    int warningCount = 0;
    int parseErrorCount = 0;

    // type별 통계 data 창고 (snapshot 출력용 복사본)
    std::unordered_map<std::string, TypeStatistics> typeStats; //Type별 snapshot
};

// 통계 수집 담당
class StatisticsCollector{
private :
    // const내부에서 mutex
    mutable std::mutex mutex;

    // 공통 통계
    int totalCount = 0;
    int normalCount = 0;
    int warningCount = 0;
    int parseErrorCount = 0;

    // sever 실행되는 동안 update되는 실제 data 창고 (원본)
    std::unordered_map<std::string, TypeStatistics> typeStats;

public :
    void recordPacket(const Packet& packet, bool warning);
    void recordParseError();

    // snapshot 찍기(복사)
    StatisticsSnapshot getSnapshot() const;

};