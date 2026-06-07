#include <fstream>
#include <stdexcept>

#include "Logger.h"

Logger::Logger(const std::string& logFilePath)
: logFilePath(logFilePath) {}

void Logger::write(const Packet& packet, bool isWarning){
    // append 이어쓰기 모드로 열기
    std::ofstream outFile(logFilePath, std::ios::app);

    // 파일 열기 실패 시, 예외
    if(!outFile){
        throw std::runtime_error("Failed to open log file : " + logFilePath);
    }

    // status 문자열 만들기
    std::string status = isWarning? "WARNING" : "NORMAL";

    //packet의 요소들과 판단 결과를 |로 이어서 파일에 작성.
    outFile << packet.getDeviceId() << "|"
            << packet.getTimestamp() << "|"
            << packet.getType() << "|"
            << packet.getValue() << "|"
            << status << std::endl;  

    // 파일 닫기
    outFile.close(); //ofsteam 소멸 시 자동으로 닫히지만, 명시적으로 닫아줌.

}