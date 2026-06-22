#include <iostream>
#include <csignal>

#include "PacketProcessor.h"
#include "PacketTaskThreadPool.h"
#include "EpollTcpServer.h"

#include "StatisticsCollector.h"
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

        // 3. EpollTcpServer 생성할 때, PacketProcessor를 품은 PacketTaskThreadPool 전달
        int port = 9000;
        int workerCount = 2;
        int maxQueueSize = 1000;

        PacketTaskThreadPool packetTaskThreadPool(workerCount, maxQueueSize, packetProcessor);
        EpollTcpServer server(port, packetTaskThreadPool);

        // 4. server.start()
        server.start();

        // ---------------------- server shutdown() 

        // 5. PacketTaskThreadPool.stop()
        packetTaskThreadPool.shutdown();

        // 6. reporter.stop()
        reporter.stop();
        
    } catch(const std::exception& e){
        std::cerr << "Server error : " << e.what() << std::endl;
        return 1;
    }

    return 0;
}