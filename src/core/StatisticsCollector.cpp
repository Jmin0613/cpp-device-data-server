#include "StatisticsCollector.h"

// packet 기록
void StatisticsCollector::recordPacket(const Packet& packet, bool warning){
    std::lock_guard<std::mutex> lock(mutex);

    // 테스트용 : elapsed time 체크
    auto now = std::chrono::steady_clock::now();

    if(!hasFirstPacketTime){
        firstPacketTime = now;
        hasFirstPacketTime = true;
    }
    lastPacketTime = now;

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

// 테스트용 : getter
long long StatisticsCollector::getProcessingElapsedMs() const{
    std::lock_guard<std::mutex> lock(mutex);

    // packet 처리 x
     if(!hasFirstPacketTime){
        return 0;
    }

    // elapsed time 계산
    return std::chrono::duration_cast<std::chrono::milliseconds>(lastPacketTime - firstPacketTime).count();

}