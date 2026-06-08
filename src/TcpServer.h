#pragma once

#include "PacketProcessor.h"

class TcpServer{
private :
    int port;
    PacketProcessor& packetProcessor;

public :
    // 생성자
    explicit TcpServer(int port, PacketProcessor& packetProcessor);
    
    // 서버 구동
    void start();
};