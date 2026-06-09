#pragma once

#include <atomic>

#include "PacketProcessor.h"

class TcpServer{
private :
    int port;
    PacketProcessor& packetProcessor;

    int maxClients; // 접속 가능한 클라이언트 수 
    std::atomic<int> activeClients; // 현재 서버에 접속한 클라이언트 수 

public :
    // 생성자
    explicit TcpServer(int port, PacketProcessor& packetProcessor, int maxClients);
    
    // 서버 구동
    void start();
};