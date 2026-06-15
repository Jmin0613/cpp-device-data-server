#include <iostream>
#include <cstring>
#include <string>

#include <cerrno>
#include <stdexcept>

#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#include <csignal>
#include <sys/time.h>

#include "TcpServer.h"

extern volatile sig_atomic_t shutdownRequested;

TcpServer::TcpServer(int port, PacketProcessor& packetProcessor, int workerCount, int maxQueueSize) 
: port(port), packetProcessor(packetProcessor), threadPool(workerCount, maxQueueSize, packetProcessor) {}

void TcpServer::start(){
    // 1. socket 생성
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);

    if(serverSocket == -1){
        std::string errorMessage = std::string("Failed to create server socket: ") + std::strerror(errno);

        throw std::runtime_error(errorMessage);
    }

    // 주소 재사용 옵션 설정
    int option = 1;

    if(setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option)) == -1){
        std::string errorMessage = std::string("Failed to set SO_RCVTIMEO: ") + std::strerror(errno);
        close(serverSocket);
        throw std::runtime_error(errorMessage);
    }

    // timeout 설정
    timeval timeout{};
    timeout.tv_sec = 3;
    timeout.tv_usec = 0;

    if(setsockopt(serverSocket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) == -1){
        std::string errorMessage = std::string("Failed to set server timeout: ") + std::strerror(errno);
        close(serverSocket);
        throw std::runtime_error(errorMessage);
    }

    // 3. 서버 구조체 설정
    // socket 구성요소를 담을 구조체 생성 및 값 할당
    sockaddr_in serverAddress{};

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(port); //생성자로 받은 포트 번호 사용

    // 4. 소켓에 주소 bind
    // 서버 socket에 IP/PORT 붙이기
    if(bind(serverSocket, (sockaddr*)&serverAddress, sizeof(serverAddress)) == -1){
        std::string errorMessage = std::string("Failed to bind socket: ") + std::strerror(errno);

        close(serverSocket);
        throw std::runtime_error(errorMessage);
    }

    // 5. 연결 대기 상태로 전환 listen
    // 클라이언트 접속 기다리기
    if(listen(serverSocket, 5) == -1){ //대기큐 크기 5.
        std::string errorMessage = std::string("Failed to listen: ") + std::strerror(errno);

        close(serverSocket);
        throw std::runtime_error(errorMessage);
    }

    std::cout << "Server listening on port " << port << "..." << std::endl;

    // while : accept, handleClient(recv + close)
    while(!shutdownRequested){
        sockaddr_in clientAddress{};
        socklen_t clientAddressSize = sizeof(clientAddress);

        // 6. 클라이언트 연결 수락 accept
        int clientSocket = accept(serverSocket, (sockaddr*)&clientAddress, &clientAddressSize);

        // client 접속 X → SO_RCVTIMEO에 의해 accept()가 timeout으로 -1 반환.
        if(clientSocket == -1){
            // 종료 요청(ctrl+c) + clientSocket 연결 x
            if(shutdownRequested){
                break; // 바로 탈출
            }

            // clientSocket 연결 x + timeout / signal로 accept 끊김
            if(errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR){
                // 에러 로그 x, 조용히 위로 올라가 갱신된 shutdownRequested를 검사
                continue;
            }

            // timeout 아닌, 시스템 에러(메모리 부족 등)
            std::cerr << "Failed to accept client: " << std::strerror(errno) << std::endl;
            continue;
        }

        // clientSocket받는 순간 종료 요청
        if(shutdownRequested){
            close(clientSocket);
            break; //socket 닫고 루프 탈출.
        }

        std::cout << "Client connected!" << std::endl;

        // threadPool job queue에 clientSocket 작업 넣기
        if(!threadPool.enqueue(clientSocket)){ //꽉차서 false 오면 
            std::cout << "Server busy. Connection rejected." << std::endl;
            close(clientSocket); //해당 clientSocket close하고
            continue; // 다음 요청 받기
        }

        // enqueue true일 경우, job 통신 처리 성공(recv/process/close)
        std::cout << "Client queued." << std::endl;
    }

    //정상적으로 while 탈출 시, server socket close.
    std::cout << "Stopping server ..." << std::endl;

    if(close(serverSocket) == -1){
        std::cerr << "Failed to close server socket: " << std::strerror(errno) << std::endl;
    }

    // 종료위해 worker 정리
    threadPool.shutdown();

    std::cout << "Server stopped gracefully." << std::endl;

}