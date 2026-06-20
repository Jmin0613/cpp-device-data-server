#pragma once

#include <atomic>
#include <thread>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <string>

#include "PacketProcessor.h"

class PacketTaskThreadPool{
private : 
    PacketProcessor& packetProcessor; //데이터 처리

    std::vector<std::thread> workers; // worker thread 보관하는 vector

    size_t maxQueueSize; // queue 최대 길이 제한. .size() == size_t
    std::queue<std::string> taskQueue; // work/task 저장하는 job queue

    std::condition_variable condition;
    std::mutex queueMutex;

    std::atomic<bool> running; // threadPool 동작 상태 (종료 신호 false)

    void workerLoop(); //worker thread가 수행할 Loop

public :
    // 생성자/소멸자
    PacketTaskThreadPool(int workerCount, size_t maxQueueSize, PacketProcessor& packetProcessor);
    ~PacketTaskThreadPool();

    // job 넣기
    bool enqueue(const std::string& packetData);
    
    // 종료 감지 후, worker 정리
    void shutdown();
};