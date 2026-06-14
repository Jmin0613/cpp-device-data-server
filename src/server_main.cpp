#include <iostream>
#include <exception>

#include "TcpServer.h"
#include "PacketProcessor.h"

int main(){
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