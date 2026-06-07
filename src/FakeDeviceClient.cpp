#include <iostream>
#include <stdexcept>

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
        std::cerr << "Failed to create client socket" << std::endl;
        return ;
    }

    // 서버 주소 구조체 설정
    sockaddr_in serverAddress{};

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(serverPort); //가려는 서버 포트
    // inet_pton(AF_INET, serverIp.c_str(), &serverAddress.sin_addr); //가려는 서버 IP

    if(inet_pton(AF_INET, serverIp.c_str(), &serverAddress.sin_addr) <= 0 ){
        std::cerr << "Invalid server IP address" << std::endl;
        close(clientSocket);
        return;
    }

    // 2. 서버에 접속 - connect
    if(connect(clientSocket, (sockaddr*)&serverAddress, sizeof(serverAddress)) == -1){
        std::cerr << "Failed to connect to server" << std::endl;
        close(clientSocket);
        return;
    }

    // 3. 접속 성공 후 데이터 보내기 - send
    ssize_t sentBytes = send(clientSocket, packetData.c_str(), packetData.size(), 0);

    if(sentBytes == -1){
        std::cerr << "Failed to send packet data" << std::endl;
        close(clientSocket);
        return;
    }

    std::cout << "Sent : " << packetData << std::endl;

    // 4. close
    close(clientSocket);

}