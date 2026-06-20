#include <sstream>
#include <vector>
#include <stdexcept>

#include "PacketParser.h"

Packet PacketParser::parser(const std::string& rawData){
    // stringstream에 rawData 넣기
    std::stringstream ss(rawData);

    // getline이용하여 vector에 집어넣기
    std::string token;
    std::vector<std::string> tokens;

    while(std::getline(ss, token, '|')){
        tokens.push_back(token);
    }

    // tokens 개수 확인
    if(tokens.size() != 4){
        throw std::invalid_argument("Invalid packet format"); //잘못된 데이터 수신 예외 던지기
    }

    // value값 string -> double
    std::string deviceId = tokens[0];
    std::string timestamp = tokens[1];
    std::string type = tokens[2];
    double value = std::stod(tokens[3]);

    // Packet 객체 반환
    return Packet(deviceId, timestamp, type, value);
}