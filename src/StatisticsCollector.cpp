#include "StatisticsCollector.h"

// packet 기록
void StatisticsCollector::recordPacket(const Packet& packet, bool warning){
    std::lock_guard<std::mutex> lock(mutex);

    // 공통 cnt 증가
    totalCount++;
    
    if(warning){
        warningCount++;
    } else{
        normalCount++;
    }

    // type별 통계
    std::string type = packet.getType();
    double value = packet.getValue();

    TypeStatistics& stats = typeStats[type];

    if(stats.count == 0){ // 처음 들어오는 data
        stats.max = value;
        stats.min = value;
    } else{
        // 비교 후 갱신
        if(value > stats.max) stats.max = value;
        if(value < stats.min) stats.min = value;    
    }
    stats.sum += value;
    stats.count++; //해당 type 개수 증가
}

// 파싱 에러 기록
void StatisticsCollector::recordParseError(){
    std::lock_guard<std::mutex> lock(mutex);

    totalCount++;
    parseErrorCount++; // 파싱 에러 cnt 증가
}

// snapshot 찍기
StatisticsSnapshot StatisticsCollector::getSnapshot() const{
    std::lock_guard<std::mutex> lock(mutex);

    
    StatisticsSnapshot snapshot{};

    // 공통 통계 복사
    snapshot.totalCount = totalCount;
    snapshot.normalCount = normalCount;
    snapshot.warningCount = warningCount;
    snapshot.parseErrorCount = parseErrorCount;

    // type별 
    snapshot.typeStats = typeStats;

    return snapshot;
}