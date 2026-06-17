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

void ClientSession::processPacketData(const std::string& packetData){
    // 빈 패킷 넘너뛰기
    if(packetData.empty()){
        return;
    }

    // packet마다 processor 처리
    try{
        ProcessResult result = packetProcessor.process(packetData);

        std::cout << "Received: " << packetData << std::endl;
        std::cout << "Status: " << (result.warning ? "WARNING" : "NORMAL") << std::endl;
    } catch(const std::exception& e){
        std::cerr << "Failed to process packet: " << e.what() << std::endl;
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
    std::string pendingBuffer; // data 누적할 buffer

    while(true){
        // 클라이언트가 보낸 문자열 받기
        ssize_t receivedBytes = recv(clientSocket, buffer, sizeof(buffer), 0);

        if(receivedBytes > 0){ // 데이터 받음
            // data 누적
            pendingBuffer.append(buffer, receivedBytes);

            // packet 분리
            size_t linePos;
            while((linePos = pendingBuffer.find('\n')) != std::string::npos){
                // \n 기준으로 packet 추출 후, 지우기
                std::string packetData = pendingBuffer.substr(0, linePos);
                pendingBuffer.erase(0, linePos + 1);

                // packet마다 processor처리
                processPacketData(packetData);  
            }

        } else if(receivedBytes == 0){ // 클라이언트가 연결을 종료
            // client close 시, 마지막 데이터 불완전 packet 판단.
            if(!pendingBuffer.empty()){
                std::cerr << "Discarding incomplete packet: " << pendingBuffer << std::endl;
                pendingBuffer.clear(); // 비우기
            }

            std::cout << "Client disconnected" << std::endl;
            break;
        } else{ // 음수값
            // receivedBytes == -1
            if(errno == EAGAIN || errno == EWOULDBLOCK){ // timeout
                std::cout << "recv timeout. Closing client socket." << std::endl;
            } else if(errno == EINTR){ // signal로 인한 중단
                std::cout << "recv interrupted. Closing client socket." << std::endl;
            } else{ // 그 외 에러
                std::cerr << "Failed to receive data: " << std::strerror(errno) << std::endl;
            }
            break;
            // close(clientSocket);
        }

    }
    // 8. 소켓 종료 close
    // close(clientSocket); → 소멸자
}