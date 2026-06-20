#pragma once

#include <string>
#include <unordered_map>

#include "PacketTaskThreadPool.h"

class EpollTcpServer{
private :
    int port;
    PacketTaskThreadPool& packetTaskThreadPool; //완성된 packetData 넘길 worker Pool

    int serverSocket; //listen socket
    int epollFd; // epoll 인스턴스 fd

    // client fd(socket)별 미완성 data 저장
    std::unordered_map<int, std::string> pendingBuffers;

    static constexpr int MAX_EVENTS = 64; //epoll_wait() 받아오는 이벤트 최대 크기
    static constexpr int BUFFER_SIZE = 1024; //recv()로 한 번에 읽을 임시 버퍼 크기

    void setupServerSocket(); //socket, setsockopt, bind, listen, setNonBlocking(serverSocket) 담당
    void setupEpoll(); // 생성한 epoll 인스턴스에 serverSocket 등록

    void setNonBlocking(int fd); // fcntl() 이용하여 non blocking처리

    void addToEpoll(int fd); //epoll에 신규 fd(client + serverSocket) 등록
    void removeFromEpoll(int fd); //epoll에서 client socket 제거

    void acceptClients(); // 신규 client accept
    
    void handleClientData(int clientSocket); // recv 수행
    void processCompletePackets(int clientSocket); // pendingBuffer에서 '\n' 기준 packetData 분리
    void closeClient(int clientSocket); //client socket 종료

public :
    // 생성자 + 소멸자
    explicit EpollTcpServer(int port, PacketTaskThreadPool& packetTaskThreadPool);
    ~EpollTcpServer();

    // 서버 구동
    void start();

};