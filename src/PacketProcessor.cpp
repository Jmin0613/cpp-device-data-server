#include "PacketProcessor.h"
#include "PacketParser.h"
#include "WarningDetector.h"

#include <iostream>

PacketProcessor::PacketProcessor(const std::string& logFilePath, StatisticsCollector& statistics)
: logger(logFilePath), statistics(statistics) {}

ProcessResult PacketProcessor::process(const std::string& rawData){
    try{
        // PacketParser로 rawData -> Packet 변환
        Packet packet = PacketParser::parser(rawData);

        // WarningDetector로 이상 여부 판단
        bool isWarning = WarningDetector::isWarning(packet);

        // 통계 수집
        statistics.recordPacket(packet, isWarning);

        // Logger 로그 저장
        logger.write(packet, isWarning);

        // 데이터 처리 결과 반환
        return ProcessResult{packet, isWarning};
        
    } catch(const std::exception&){
        // 파싱 에러 통계 수집
        statistics.recordParseError();
        throw;
    }

}