#include "PacketProcessor.h"
#include "PacketParser.h"
#include "WarningDetector.h"

PacketProcessor::PacketProcessor(const std::string& logFilePath)
: logger(logFilePath) {}

ProcessResult PacketProcessor::process(const std::string& rawData){
    // PacketParser로 rawData -> Packet 변환
    Packet packet = PacketParser::parser(rawData);

    // WarningDetector로 이상 여부 판단
    bool isWarning = WarningDetector::isWarning(packet);

    // Logger 로그 저장
    logger.write(packet, isWarning);

    // 데이터 처리 결과 반환
    return ProcessResult{packet, isWarning};
}