#include <iostream>
#include <cstring>
#include <string>

#include <cerrno>
#include <stdexcept>

#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#include <thread>

#include "TcpServer.h"
#include "ClientSession.h"

TcpServer::TcpServer(int port, PacketProcessor& packetProcessor, int maxClients) 
: port(port), packetProcessor(packetProcessor), maxClients(maxClients), activeClients(0) {}

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

    //while : accept, handleClient(recv + close)
    while(true){
        // 6. 클라이언트 연결 수락 accept
        // 클라이언트 접속 받기
        sockaddr_in clientAddress{};
        socklen_t clientAddressSize = sizeof(clientAddress);

        int clientSocket = accept(serverSocket, (sockaddr*)&clientAddress, &clientAddressSize);

        if (clientSocket == -1){
            std::cerr << "Failed to accept client: " << std::strerror(errno) << std::endl;
            continue;
        }

        std::cout << "Client connected!" << std::endl;

        // thread 만들기 전, activeClients 체크
        if(activeClients.load() >= maxClients){
            std::cout << "server busy. Connection rejected." << std::endl;
            close(clientSocket); // 접속한 client Socket 닫아버리기.
            continue;
        }

        activeClients++; //.fetch_add(1);

        // 7. 멀티 스레드 구조로, ClientSession에서 recv/process/close 담당
        std::thread clientThread([this, clientSocket]() {
            try{
                ClientSession session(clientSocket, packetProcessor);
                session.handle();
            } catch(const std::exception& e){
                std::cerr << "Client thread error: " << e.what() << std::endl;
            }
            
            activeClients--;
            
            std::cout << "Client session finished. Active clients: " 
            << activeClients.load() << "/" << maxClients << std::endl;
        });
        
        clientThread.detach();
    }

}