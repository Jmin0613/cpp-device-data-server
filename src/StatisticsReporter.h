#pragma once

#include <atomic>
#include <thread>

#include "StatisticsCollector.h"

class StatisticsReporter{
private :
    StatisticsCollector& statistics; // 통계 수집 접근
    int intervalSeconds; // 상태 갱신 주기

    std::atomic<bool> running; // thread 동작 상태 (종료 신호 false)
    std::thread reporterThread; // 상태 갱신 thread

    int lastPrintedTotalCount = 0; // 마지막 출력 시점의 totalCnt

    void run(); 
    void printSnapshot(const StatisticsSnapshot& snapshot) const;

public :
    // 생성자 + 소멸자
    StatisticsReporter(StatisticsCollector& statistics, int intervalSeconds);
    ~StatisticsReporter();

    void start(); // 상태 갱신 thread 시작
    void stop(); // 상태 갱신 thread 정지

    // 복사 막기
    StatisticsReporter(const StatisticsReporter&) = delete;
    StatisticsReporter& operator=(const StatisticsReporter&) = delete;

};