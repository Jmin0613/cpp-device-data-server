#include <iostream>
#include <exception>
#include <csignal>

#include "TcpServer.h"
#include "PacketProcessor.h"
#include "StatisticsReporter.h"

// Ctrl+C 감지 flag
volatile sig_atomic_t shutdownRequested = 0;

void handleSignal(int){
    shutdownRequested = 1;
}

int main(){
    // Ctrl+C 감지
    std::signal(SIGINT, handleSignal);

    try{
        // 1. PacketProcessor(data 처리기), StatisticsCollector(통계 수집) 생성
        StatisticsCollector statistics;
        PacketProcessor packetProcessor("logs/device.log", statistics);

        // 2. 통계 수집 출력
        StatisticsReporter reporter(statistics, 5); //5초마다 상태 갱신
        reporter.start();

        // 3. TcpServer 생성할 때, PacketProcessor 전달
        int port = 9000;
        int workerCount = 2;
        int maxQueueSize = 2;
        TcpServer server(port, packetProcessor, workerCount, maxQueueSize);

        // 4. server.start()
        server.start();

        // 5. reporter.stop()
        reporter.stop();
        
    } catch(const std::exception& e){
        std::cerr << "Server error : " << e.what() << std::endl;
        return 1;
    }

    return 0;
}