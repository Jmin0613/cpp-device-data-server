#include <iostream>
#include <stdexcept>

#include <cerrno> // 실패 시 Linux socket가 errno에 남긴 실패 이유 가져오기
#include <cstring>
#include <string>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "FakeDeviceClient.h"

FakeDeviceClient::FakeDeviceClient(const std::string& serverIp, int serverPort)
: serverIp(serverIp), serverPort(serverPort){}

void FakeDeviceClient::sendPacket(const std::string& packetData){
    // 1. socket 생성
    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);

    if(clientSocket== -1){
        std::string errorMessage = std::string("Failed to create client socket: ") + std::strerror(errno);
        throw std::runtime_error(errorMessage);
    }

    // 서버 주소 구조체 설정
    sockaddr_in serverAddress{};

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(serverPort); //가려는 서버 포트
    // inet_pton(AF_INET, serverIp.c_str(), &serverAddress.sin_addr); //가려는 서버 IP

    if(inet_pton(AF_INET, serverIp.c_str(), &serverAddress.sin_addr) <= 0 ){
        close(clientSocket);
        throw std::runtime_error(std::string("Invalid server IP address: ") + serverIp);
    }

    // 2. 서버에 접속 - connect
    if(connect(clientSocket, (sockaddr*)&serverAddress, sizeof(serverAddress)) == -1){
        std::string errorMessage = std::string("Failed to connect to server: ") + std::strerror(errno);

        close(clientSocket);
        throw std::runtime_error(errorMessage);
    }

    // 3. 접속 성공 후 데이터 보내기 - send
    ssize_t sentBytes = send(clientSocket, packetData.c_str(), packetData.size(), 0);

    if(sentBytes == -1){
        std::string errorMessage = std::string("Failed to send packet data: ") + std::strerror(errno);

        close(clientSocket);
        throw std::runtime_error(errorMessage);
    }

    std::cout << "Sent : " << packetData << std::endl;

    // 4. close
    close(clientSocket);

}