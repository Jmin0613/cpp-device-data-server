#include <iostream>
#include <cstring>
#include <string>

#include <cerrno>
#include <stdexcept>

#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#include "TcpServer.h"

TcpServer::TcpServer(int port) : port(port){}

void TcpServer::start(){
    // 1. socket 생성
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);

    if(serverSocket == -1){
        std::string errorMessage = std::string("Failed to create server socket: ") + std::strerror(errno);

        throw std::runtime_error(errorMessage);
    }

    // 주소 재사용 옵션 설정
    int option = 1;
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));

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

    // 6. 클라이언트 연결 수락 accept
    // 클라이언트 접속 받기
    sockaddr_in clientAddress{};
    socklen_t clientAddressSize = sizeof(clientAddress);

    int clientSocket = accept(serverSocket, (sockaddr*)&clientAddress, &clientAddressSize);

    if (clientSocket == -1){
        std::string errorMessage = std::string("Failed to accept client: ") + std::strerror(errno);

        close(serverSocket);
        throw std::runtime_error(errorMessage);
    }

    std::cout << "Client connected!" << std::endl;

    // 7. 데이터 송수신 recv
    char buffer[1024];
    std::memset(buffer, 0, sizeof(buffer)); // 버퍼 초기화 

    // 클라이언트가 보낸 문자열 받기
    ssize_t receivedBytes = recv(clientSocket, buffer, sizeof(buffer)-1, 0);

    if(receivedBytes > 0){ // 데이터 받음
        buffer[receivedBytes] = '\0';
        std::cout << "Received : " << buffer << std::endl;
    } else if(receivedBytes == 0){ // 클라이언트가 연결을 종료
        std::cout << "Client disconnected" << std::endl;
    } else{ // 음수값 -> 에러 발생
        std::string errorMessage = std::string("Failed to receive data: ") + std::strerror(errno);

        close(clientSocket);
        close(serverSocket);
        throw std::runtime_error(errorMessage);
    }

    // 8. 소켓 종료 close
    close(clientSocket);
    close(serverSocket);

}