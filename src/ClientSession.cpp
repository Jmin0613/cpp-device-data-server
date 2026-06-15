#include <iostream>
#include <cstring>
#include <string>

#include <cerrno>
#include <exception>

#include <sys/socket.h>
#include <unistd.h>
#include <sys/time.h>

#include "ClientSession.h"

// 생성자 + 소멸자(socket 닫기)
ClientSession::ClientSession(int clientSocket, PacketProcessor& packetProcessor)
: clientSocket(clientSocket), packetProcessor(packetProcessor) {}

ClientSession::~ClientSession(){
    if(clientSocket != -1){
        close(clientSocket);
        clientSocket = -1; // 해당 socket 번호 삭제.
    }
}

void ClientSession::handle(){
    // timeout 설정
    timeval timeout{};
    timeout.tv_sec = 3;
    timeout.tv_usec = 0;

    if(setsockopt(clientSocket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) == -1){
        std::cerr << "Failed to set client recv timeout: " << std::strerror(errno) << std::endl;
        return;
    }

    // 7. 데이터 송수신 recv
    char buffer[1024];
    std::memset(buffer, 0, sizeof(buffer)); // 버퍼 초기화 

    // 클라이언트가 보낸 문자열 받기
    ssize_t receivedBytes = recv(clientSocket, buffer, sizeof(buffer)-1, 0);

    if(receivedBytes > 0){ // 데이터 받음
        // rawData 생성
        std::string receivedData(buffer, receivedBytes);
        std::cout << "Received : " << receivedData << std::endl;

        // PacketProcessor 처리 연결
        try{
            ProcessResult result = packetProcessor.process(receivedData);
            std::cout << "Status: " << (result.warning ? "WARNING" : "NORMAL") << std::endl;
        } catch(const std::exception& e){
            std::cerr << "Failed to process packet: " << e.what() << std::endl;
        }

    } else if(receivedBytes == 0){ // 클라이언트가 연결을 종료
        std::cout << "Client disconnected" << std::endl;
    } else{ // 음수값
        // receivedBytes == -1
        if(errno == EAGAIN || errno == EWOULDBLOCK){ // timeout
            std::cout << "recv timeout. Closing client socket." << std::endl;
        } else if(errno == EINTR){ // signal로 인한 중단
            std::cout << "recv interrupted. Closing client socket." << std::endl;
        } else{ // 그 외 에러
            std::cerr << "Failed to receive data: " << std::strerror(errno) << std::endl;
        }
        // close(clientSocket);
    }

    // 8. 소켓 종료 close
    // close(clientSocket); → 소멸자
}