#include <iostream>
#include <exception>
#include <csignal>

#include <iomanip>

#include "TcpServer.h"
#include "PacketProcessor.h"
#include "StatisticsCollector.h"

// Ctrl+C 감지 flag
volatile sig_atomic_t shutdownRequested = 0;

void handleSignal(int){
    shutdownRequested = 1;
}

// snapshot 뽑기
void printStatisticsSnapshot(const StatisticsSnapshot& snapshot){
    std::cout << "\n========== Statistics ==========\n";
    std::cout << "Total packets : " << snapshot.totalCount << "\n";
    std::cout << "Normal        : " << snapshot.normalCount << "\n";
    std::cout << "Warning       : " << snapshot.warningCount << "\n";
    std::cout << "Parse errors  : " << snapshot.parseErrorCount << "\n";

    for(const auto& pair : snapshot.typeStats){
        const std::string& type = pair.first;
        const TypeStatistics& stats = pair.second;

        double avg = stats.count == 0 ? 0.0 : stats.sum / stats.count;

        std::cout << "\n[" << type << "]\n";
        std::cout << "Count : " << stats.count << "\n";
        std::cout << "Avg   : " << std::fixed << std::setprecision(2) << avg << "\n";
        std::cout << "Min   : " << stats.min << "\n";
        std::cout << "Max   : " << stats.max << "\n";
    }

    std::cout << "=======================================\n";
}

int main(){
    // Ctrl+C 감지
    std::signal(SIGINT, handleSignal);

    try{
        // 1. PacketProcessor(data 처리기), StatisticsCollector(통계 수집) 생성
        StatisticsCollector statistics;
        PacketProcessor packetProcessor("logs/device.log", statistics);

        // 2. TcpServer 생성할 때, PacketProcessor 전달
        int port = 9000;
        int workerCount = 2;
        int maxQueueSize = 2;
        TcpServer server(port, packetProcessor, workerCount, maxQueueSize);

        // 3. server.start()
        server.start();

        // snapshot 뽑아서 통계 수집 확인
        StatisticsSnapshot snapshot = statistics.getSnapshot();
        printStatisticsSnapshot(snapshot);
        
    } catch(const std::exception& e){
        std::cerr << "Server error : " << e.what() << std::endl;
        return 1;
    }

    return 0;
}