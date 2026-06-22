#include <iostream>
#include <exception>
#include <string>

#include "MockDeviceSimulator.h"

int main(int argc, char* argv[]){
    // 공통 서버 설정
    std::string serverIp = "127.0.0.1";
    int serverPort = 9000;
    
    // 테스트 조건
    int clientCount = 1;
    int packetCount = 30;
    int delayMs = 100;

    try{
        // argv에서 값 꺼내기
        if(argc != 4){
            std::cerr << "Invalid test condition." << std::endl;
            std::cerr << "./mock_simulator <clientCount> <packetCount> <delayMs>" << std::endl;
            return 1;
        }

        clientCount = std::stoi(argv[1]);
        packetCount = std::stoi(argv[2]);
        delayMs = std::stoi(argv[3]);

        std::cout << "========== Mock Simulator Test ==========" << std::endl;
        std::cout << "Client Count           : " << clientCount << std::endl;
        std::cout << "Packet Count Per Client: " << packetCount << std::endl;
        std::cout << "Delay(ms)              : " << delayMs << std::endl;
        std::cout << "Expected Total Packets : " << clientCount * packetCount << std::endl;
        std::cout << "=========================================" << std::endl;


        // 시뮬레이터 생성
        MockDeviceSimulator simulator(serverIp, serverPort);

        // 테스트 실행
        simulator.runContinuous(clientCount, packetCount, delayMs);

    } catch(const std::exception& e){
        std::cerr << "Mock simulator error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}