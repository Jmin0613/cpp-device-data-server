#include <chrono>
#include <iostream>
#include <iomanip>

#include "StatisticsReporter.h"

StatisticsReporter::StatisticsReporter(StatisticsCollector& statistics, int intervalSeconds)
: statistics(statistics), intervalSeconds(intervalSeconds), running(false) {}

StatisticsReporter::~StatisticsReporter(){
    stop();
}

void StatisticsReporter::start(){
    if(running) return; //이미 동작중이면 return

    running = true; // false → true

    reporterThread = std::thread(&StatisticsReporter::run, this); // 상태 갱신 thread 생성 + run() 실행.
}

void StatisticsReporter::stop(){
    if(!running) return; //이미 종료중이면 return

    running = false; // true → false

    // 모든 상태 갱신 threads 종료 위해 대기
    if(reporterThread.joinable()){
        reporterThread.join();
    }
}

// 내부에서 intervalSecond마다 Loop
void StatisticsReporter::run(){
    while(running){
        std::this_thread::sleep_for(std::chrono::seconds(intervalSeconds));

        if(!running) break; //sleep + false → Loop 탈출

        StatisticsSnapshot snap = statistics.getSnapshot();

        // 새로운 data 안들어오면 출력 X
        if(snap.totalCount == lastPrintedTotalCount){
            continue;
        }

        printSnapshot(snap);
        lastPrintedTotalCount = snap.totalCount; // totalCnt 기억
    }
}

// snapshot 뽑기
void StatisticsReporter::printSnapshot(const StatisticsSnapshot& snapshot) const{
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

    std::cout << "=======================================" << std::endl;
}
