#pragma once

#include <atomic>

#include "PacketProcessor.h"
#include "ThreadPool.h"

class TcpServer{
private :
    int port;
    PacketProcessor& packetProcessor;
    ThreadPool threadPool;

public :
    // 생성자
    explicit TcpServer(int port, PacketProcessor& packetProcessor, int workerCount, size_t maxQueueSize);
    
    // 서버 구동
    void start();
};