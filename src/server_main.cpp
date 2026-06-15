#include <iostream>
#include <exception>
#include <csignal>

#include "TcpServer.h"
#include "PacketProcessor.h"

// Ctrl+C 감지 flag
volatile sig_atomic_t shutdownRequested = 0;

void handleSignal(int){
    shutdownRequested = 1;
}

int main(){
    // Ctrl+C 감지
    std::signal(SIGINT, handleSignal);

    try{
        // 1. PacketProcessor 생성
        PacketProcessor packetProcessor("logs/device.log");

        // 2. TcpServer 생성할 때, PacketProcessor 전달
        int port = 9000;
        int workerCount = 2;
        int maxQueueSize = 2;
        TcpServer server(port, packetProcessor, workerCount, maxQueueSize);

        // 3. server.start()
        server.start();
        
    } catch(const std::exception& e){
        std::cerr << "Server error : " << e.what() << std::endl;
        return 1;
    }

    return 0;
}