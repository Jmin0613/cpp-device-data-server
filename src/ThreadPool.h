#pragma once

#include <atomic>
#include <thread>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>

#include "PacketProcessor.h"

class ThreadPool{
private :
    PacketProcessor& packetProcessor; //데이터 처리
    
    std::vector<std::thread> workers; // worker thread 보관하는 vector

    size_t maxQueueSize; // queue 최대 길이 제한. .size() == size_t
    std::queue<int> clientQueue; // work 저장하는 job queue

    std::condition_variable condition;
    std::mutex queueMutex;

    std::atomic<bool> running; // thread 동작 상태 (종료 신호 false)

    void workerLoop(); //worker thread가 수행할 Loop

public :
    // 생성자/소멸자
    ThreadPool(int workerCount, int maxQueueSize, PacketProcessor& packetProcessor);
    ~ThreadPool();

    // 복사 막기
    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;

    // job 넣기
    bool enqueue(int clientSocket);
};