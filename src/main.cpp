#include <iostream>
#include <string>
#include <exception>
#include "PacketParser.h" //#include "Packet.h"
#include "WarningDetector.h"

int main(){
    // //Packet 객체 직접 만들어 test
    // Packet packet("SAT-003", "2026-06-05T17:05:05", "TEMP", 91.2);

    // PacketParser를 통해 Packet객체 생성.
    std::string rawData = "SAT-003|2026-06-05T23:30:07|TEMP|91.2";
    
    try{
        Packet packet = PacketParser::parser(rawData);

        std::cout << "Device ID : " <<packet.getDeviceId() << std::endl;
        std::cout << "Timestamp ID : " <<packet.getTimestamp() << std::endl;
        std::cout << "Type : " <<packet.getType() << std::endl;
        std::cout << "Value : " <<packet.getValue() << std::endl;

        if(WarningDetector::isWarning(packet)){
            std::cerr << "Status : WARNING" << std::endl;
        }
        else{
            std::cout << "Status : NORMAL" << std::endl;
        }

    } catch (const std::exception& e){
        // PacketParser::parse 호출 실패시 예외
        std::cerr << "Packet parsing failed: " << e.what() << std::endl;
    }

    return 0;
}