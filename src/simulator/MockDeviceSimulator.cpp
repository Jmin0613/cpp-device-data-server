#include <iostream>
#include <string>
#include <cstring>
#include <sstream>

#include <random>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <thread>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>

#include <cerrno>
#include <stdexcept>

#include "MockDeviceSimulator.h"

// 생성자
MockDeviceSimulator::MockDeviceSimulator(const std::string& serverIp, int serverPort)
: serverIp(serverIp), serverPort(serverPort){}

// Packet 자동 생성
std::string MockDeviceSimulator::generatePacket(){
    // deviceId (고정)
    std::string deviceId = "SAT-001";

    // type + value
    std::string type;
    double value;

    // type 지정
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<int> typeDist(0, 1);

    // value 생성
    if(typeDist(gen) == 0){ // TEMP
        type = "TEMP";
        std::uniform_real_distribution<double> valueDist(20.0, 95.0);
        value = valueDist(gen);
    } else{ // SIGNAL
        type = "SIGNAL";
        std::uniform_real_distribution<double> valueDist(10.0, 100.0);
        value = valueDist(gen);
    }

    // 현재 시간 받기
    auto now = std::chrono::system_clock::now();
    std::time_t nowTime = std::chrono::system_clock::to_time_t(now);

    // local timezone
    std::tm localTime{}; // 년, 월, 일, 시, 분, 초
    localtime_r(&nowTime, &localTime);

    // timestamp 만들기
    std::ostringstream timestampStream;
    timestampStream << std::put_time(&localTime, "%Y-%m-%dT%H:%M:%S");
    
    // packet 문자열 데이터 조립 - SAT-001|2026-06-16T18:51:12|TEMP|67.1
    std::ostringstream packetStream;
    packetStream << deviceId << "|"
                 << timestampStream.str()<< "|" 
                 << type<< "|"
                 << std::fixed << std::setprecision(1) << value;

    // 조립한 패킷 반환
    return packetStream.str();
}

// 생성한 packet을 server로 전송
void MockDeviceSimulator::sendPacket(const std::string& packet){
    // client socket 생성
    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);

    if(clientSocket == -1){
        std::string errorMessage = std::string("Failed to create client socket: ") + std::strerror(errno);

        throw std::runtime_error(errorMessage);
    }

    // 접속할 server 주소 설정
    sockaddr_in serverAddress{};

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(serverPort); //가려는 서버 포트
    
    if(inet_pton(AF_INET, serverIp.c_str(), &serverAddress.sin_addr) <= 0 ){
        close(clientSocket);
        throw std::runtime_error(std::string("Invalid server IP address: ") + serverIp);
    }

    // 서버 접속 - connect
    if(connect(clientSocket, (sockaddr*)&serverAddress, sizeof(serverAddress)) == -1){
        std::string errorMessage = std::string("Failed to connect to server: ") + std::strerror(errno);

        close(clientSocket);
        throw std::runtime_error(errorMessage);
    }

    // 접속 성공 후 데이터 보내기 - send
    ssize_t sentBytes = send(clientSocket, packet.c_str(), packet.size(), 0);

    // 실패없이 보냈는지 체크
    if(sentBytes == -1){
        std::string errorMessage = std::string("Failed to send packet data: ") + std::strerror(errno);

        close(clientSocket);
        throw std::runtime_error(errorMessage);
    }

    // 보낸 packet 바이트 수 체크 (안 잘리고 다 보내졌는지)
    if(sentBytes != static_cast<ssize_t>(packet.size())) {
        close(clientSocket);
        throw std::runtime_error("Failed to send entire packet data");
    }

    std::cout << "Sent : " << packet << std::endl;

    // close
    close(clientSocket);

}

// 실행
void MockDeviceSimulator::run(int packetCount, int delayMs){
    for(int i=0; i < packetCount; i++){
        // packet 생성
        std::string packet = generatePacket();

        // server로 send
        sendPacket(packet);

        // 지연
        std::this_thread::sleep_for(std::chrono::milliseconds(delayMs));
    }
}

// (단일 client) 다중 전송 실행
void MockDeviceSimulator::runContinuous(int packetCount, int delayMs){
    // client socket 생성
    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);

    if(clientSocket == -1){
        std::string errorMessage = std::string("Failed to create client socket: ") + std::strerror(errno);

        throw std::runtime_error(errorMessage);
    }

    // 접속할 server 주소 설정
    sockaddr_in serverAddress{};

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(serverPort); //가려는 서버 포트
    
    if(inet_pton(AF_INET, serverIp.c_str(), &serverAddress.sin_addr) <= 0 ){
        close(clientSocket);
        throw std::runtime_error(std::string("Invalid server IP address: ") + serverIp);
    }

    // 서버 접속 - connect
    if(connect(clientSocket, (sockaddr*)&serverAddress, sizeof(serverAddress)) == -1){
        std::string errorMessage = std::string("Failed to connect to server: ") + std::strerror(errno);

        close(clientSocket);
        throw std::runtime_error(errorMessage);
    }

    // 다중 패캣 전송
    for(int i=0; i < packetCount; i++){
        std::string packet = generatePacket() + "\n"; //구분자 추가
        // send
        ssize_t sentBytes = send(clientSocket, packet.c_str(), packet.size(), 0);

        // 실패없이 보냈는지 체크
        if(sentBytes == -1){
            std::string errorMessage = std::string("Failed to send packet data: ") + std::strerror(errno);

            close(clientSocket);
            throw std::runtime_error(errorMessage);
        }

        // 보낸 packet 바이트 수 체크 (안 잘리고 다 보내졌는지)
        if(sentBytes != static_cast<ssize_t>(packet.size())) {
            close(clientSocket);
            throw std::runtime_error("Failed to send entire packet data");
        }

        std::cout << "Sent : " << packet;

        // 지연
        std::this_thread::sleep_for(std::chrono::milliseconds(delayMs));
    }

    // close
    close(clientSocket);
}

// 테스트용 (다중 client)
void MockDeviceSimulator::runContinuous(int clientCount, int packetCount, int delayMs){
    // client 역할을 해줄 thread 생성
    std::vector<std::thread> clientThreads;
    clientThreads.reserve(clientCount);

    // 각 clientThread 별, 패킷 전송 실행
    for(int i = 0; i < clientCount; i++){
        clientThreads.emplace_back([this, packetCount, delayMs]() {
            runContinuous(packetCount, delayMs);
        });
    }

    // client대용 thread 정리
    for (auto& clientThread : clientThreads) {
        if (clientThread.joinable()) {
            clientThread.join();
        }
    }

    std::cout << "All mock clients finished" << std::endl;

}
