#include <iostream>
#include <cstring>
#include <string>

#include <stdexcept>
#include <cerrno>

#include <fcntl.h>
#include <unistd.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/epoll.h>

#include <csignal>

#include "EpollTcpServer.h"

extern volatile sig_atomic_t shutdownRequested;

EpollTcpServer::EpollTcpServer(int port, PacketTaskThreadPool& packetTaskThreadPool)
: port(port), packetTaskThreadPool(packetTaskThreadPool), serverSocket(-1), epollFd(-1) {}

EpollTcpServer::~EpollTcpServer(){
    // server 닫기
    if(serverSocket != -1){
        close(serverSocket);
        serverSocket = -1;
    }

    // epoll 닫기
    if (epollFd != -1) {
        close(epollFd);
        epollFd = -1;
    }
}

//socket, setsockopt, bind, listen, setNonBlocking(serverSocket) 담당
void EpollTcpServer::setupServerSocket(){
    // 1. socket 생성
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);

    if(serverSocket == -1){
        std::string errorMessage = std::string("Failed to create server socket: ") + std::strerror(errno);

        throw std::runtime_error(errorMessage);
    }

    // 주소 재사용 옵션 설정
    int option = 1;

    if(setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option)) == -1){
        std::string errorMessage = std::string("Failed to set SO_REUSEADDR: ") + std::strerror(errno);
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
        std::string errorMessage = std::string("Failed to bind server socket: ") + std::strerror(errno);

        close(serverSocket);
        throw std::runtime_error(errorMessage);
    }

    // 5. 연결 대기 상태로 전환 listen
    // 클라이언트 접속 기다리기
    if(listen(serverSocket, 5) == -1){ //대기큐 크기 5.
        std::string errorMessage = std::string("Failed to listen on server socket: ") + std::strerror(errno);

        close(serverSocket);
        throw std::runtime_error(errorMessage);
    }

    setNonBlocking(serverSocket);
}

// 생성한 epoll 인스턴스에 serverSocket 등록
void EpollTcpServer::setupEpoll(){
    //epoll 인스턴스 생성
    epollFd = epoll_create1(0);

    if(epollFd == -1){ //생성 실패 시,
        std::string errorMessage = std::string("Failed to create epoll instance: ") + std::strerror(errno);
        throw std::runtime_error(errorMessage);
    }

    addToEpoll(serverSocket); //serverSocket 바로 등록
}

// fd(server/client socket) non-blocking 설정
void EpollTcpServer::setNonBlocking(int fd){
    // fd(socket)의 flags(기본설정) 복사
    int flags = fcntl(fd, F_GETFL, 0);

    if(flags == -1){ // 복사 실패시
        std::string errorMessage = std::string("Failed to get socket flags: ") + std::strerror(errno);
        throw std::runtime_error(errorMessage);
    }

    if(fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1){ // non-blocking 실패시
        std::string errorMessage = std::string("Failed to set non-blocking mode: ") + std::strerror(errno);
        throw std::runtime_error(errorMessage);
    }
}

// epoll에 신규 fd(server, client socket) 등록
void EpollTcpServer::addToEpoll(int fd){
    // fd 알림 구조체 생성 및 readable 설정
    struct epoll_event event{}; // 커널에 알릴 event 구조체
    event.events = EPOLLIN; //readable 감시 설정 (설정)
    event.data.fd = fd; // events 발생시, 등록위해 fd socket 번호 저장

    // readable 감지 후 epoll 등록
    if(epoll_ctl(epollFd, EPOLL_CTL_ADD, fd, &event) == -1){ // 실패시
        std::string errorMessage = std::string("Failed to add fd to epoll: ") + std::strerror(errno);
        throw std::runtime_error(errorMessage);
    }
}

//epoll에서 client socket 제거
void EpollTcpServer::removeFromEpoll(int fd){
    if(epoll_ctl(epollFd, EPOLL_CTL_DEL, fd, nullptr) == -1){ //삭제 실패시
        std::cerr << "Failed to remove fd from epoll: " << std::strerror(errno) << std::endl;
    }
}

// 신규 client accept
void EpollTcpServer::acceptClients(){
    while(true){
        sockaddr_in clientAddress{};
        socklen_t clientAddressSize = sizeof(clientAddress);

        // 클라이언트 연결 수락 accept
        int clientSocket = accept(serverSocket, (sockaddr*)&clientAddress, &clientAddressSize);

        // non-blocking accept에서 더 이상 받을 client가 없으면 -1 + EAGAIN/EWOULDBLOCK 반환
        if(clientSocket == -1){
            // clientSocket 연결 x + timeout / signal로 accept 끊김
            if(errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR){
                // 에러 로그 x, 조용히 위로 올라가 갱신된 shutdownRequested를 검사
                break;
            }

            // timeout 아닌, 시스템 에러(메모리 부족 등)
            std::cerr << "Failed to accept client: " << std::strerror(errno) << std::endl;
            break;
        }

        try{
            setNonBlocking(clientSocket); // non-blocking 설정
            addToEpoll(clientSocket); // EpollFd에 socket 등록
        } catch(const std::exception& e){
            std::cerr << "Failed to register client fd=" << clientSocket << ": " << e.what() << std::endl;
            close(clientSocket);
            continue; //다음 accept로 넘어가기
        }

        pendingBuffers[clientSocket] = ""; //packetData(value) 빈 상태로 packetData 등록
        std::cout << "Client connected: fd=" << clientSocket << std::endl;
    }
}

// clientSocket에 readable event 왔을때, recv 수행
void EpollTcpServer::handleClientData(int clientSocket){
    // recv()
    char buffer[BUFFER_SIZE]; //1024

    // pendingBuffer에 append
    while(true){
        // 클라이언트가 보낸 문자열 받기
        ssize_t receivedBytes = recv(clientSocket, buffer, sizeof(buffer), 0);

        if(receivedBytes > 0){ // 데이터 받음
            //buffer에 추가
            pendingBuffers[clientSocket].append(buffer, receivedBytes);
            processCompletePackets(clientSocket); // packet 분리

        } else if(receivedBytes == 0){ // client close 감지
            // client close 시, 마지막 데이터 불완전 packet 판단.
            closeClient(clientSocket);
            break;
        } else{ // 음수값 → recv 에러 처리
            // receivedBytes == -1
            if(errno == EAGAIN || errno == EWOULDBLOCK){
                // non-blocking socket에서 현재 읽을 데이터가 더 없음
                break;
            } else if(errno == EINTR){
                // signal로 인한 중단
                break;
            } else{ // 그 외 에러
                std::cerr << "Failed to receive data from fd=" << clientSocket << ": " << std::strerror(errno) << std::endl;
                closeClient(clientSocket);
                break;
            }
        }
    }

}

// pendingBuffers에서 '\n' 기준 packetData 분리
void EpollTcpServer::processCompletePackets(int clientSocket){
    // clientSocket의 packetData 꺼내오기
    std::string& pendingBuffer = pendingBuffers[clientSocket];

    // packet 분리
    size_t linePos;

    while((linePos = pendingBuffer.find('\n')) != std::string::npos){
        // \n 기준으로 packet 추출 후, 지우기
        std::string packetData = pendingBuffer.substr(0, linePos);
        pendingBuffer.erase(0, linePos + 1);

        // 끝까지 도달.
        if (packetData.empty()) {
            continue;
        }

        // task queue에 넣기
        bool enqueued = packetTaskThreadPool.enqueue(packetData);

        // task queue is full
        if (!enqueued) {
            std::cerr << "Packet task queue full. Dropped packet: " << packetData << std::endl;
        }
    }
}

//client socket 종료
void EpollTcpServer::closeClient(int clientSocket){
    // pendingBuffers에 남은 미완성 packetData 확인

    // std::unordered_map<int, std::string>::iterator → (ptr)
    auto it = pendingBuffers.find(clientSocket); // clientSocket에 대응하는 value값 가져오기
    // 못찾으면 pendingBuffers.end()를 던져줌 → 끝까지 찾았는데 못찾았다 의미. (미완성 pacvketData X)

    // 해당 clientSocket이 buffer에 존재 시,
    if(it != pendingBuffers.end()){
        // 미완성 packetData 존재 시,
        if (!it->second.empty()) { // it->second == clientSocket의 value값
            // 에러 문자열만 출력하여, 미완성 packetData 지운 흔적 남겨주기
            std::cerr << "Incomplete packet discarded from fd=" << clientSocket << ": " << it->second << std::endl;
        }

        // pendingBuffers에서 제거
        pendingBuffers.erase(it);
    }

    // epoll에서 제거
    removeFromEpoll(clientSocket);

    // socket close
    close(clientSocket);

    std::cout << "Client disconnected: fd=" << clientSocket << std::endl;
    
}

// 조립하여 server 구동
void EpollTcpServer::start(){
    // serverSocket 세팅 준비(생성 + bind  + listen + non-blocking)
    setupServerSocket();

    // epoll 인스턴스 생성 및 serverSocket 등록
    setupEpoll();

    std::cout << "EpollTcpServer started on port " << port << std::endl;

    // event 발생한 fd 저장하는 배열 저장소
    struct epoll_event events[MAX_EVENTS]; //64

    // epoll_wait loop 실행
    while(!shutdownRequested){
        int eventCount = epoll_wait(epollFd, events, MAX_EVENTS, 1000); //모아둔 event들 넘기기

        if(eventCount == -1){
            if(errno == EINTR){ //signal로 epoll_wait가 중단됨
                // shutdownRequested가 갱신되었을 수 있음 → while 조건 검사
                continue;
            }

            std::string errorMessage = std::string("epoll_wait failed: ") + std::strerror(errno);
            throw std::runtime_error(errorMessage);
        }

        if(eventCount == 0){ // epoll_wait의 timeout은 0.
            // timeout: 1000ms 동안 이벤트 없음
            // while 조건 검사하기 위해 다음 loop로 넘김
            continue;
        }

        // 종료 요청시 Loop 탈출
        if(shutdownRequested){
            break;
        }

        // 받아온 eventCnt만큼 epoll_loop → events 처리
        for(int i = 0; i < eventCount; i++){
            // 종료 요청시 Loop 탈출
            if(shutdownRequested){
                break;
            }

            int eventSocket = events[i].data.fd; //eventSocket = serverSocket || clientSocket

            // serverSocket event → 새로운 clientSocket의 연결 요청 → acceptClients()
            // clientSocket event → packetData send → handleClientData()
            if(eventSocket == serverSocket){
                acceptClients();
            } else{
                handleClientData(eventSocket);
            }
        }
    }

    // 종료를 위해 epollFd 등록 삭제 + clientSocket close
    for(auto& [clientSocket, buffer] : pendingBuffers){
        // client close 시, 마지막 데이터 불완전 packet 판단.
        if(!buffer.empty()){
            std::cerr << "Incomplete packet discarded from fd=" << clientSocket << ": " << buffer << std::endl;
        }

        removeFromEpoll(clientSocket); //epollFd에서 해당 socket 제거
        close(clientSocket); // 이후 socket 닫아주기
    }

    pendingBuffers.clear(); // buffer 비우기

    std::cout << "EpollTcpServer stopped gracefully" << std::endl;

}
