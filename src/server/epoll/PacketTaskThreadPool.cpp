#include <iostream>
#include <exception>
#include <stdexcept>

#include "PacketTaskThreadPool.h"

PacketTaskThreadPool::PacketTaskThreadPool(int workerCount, size_t maxQueueSize, PacketProcessor& packetProcessor)
: packetProcessor(packetProcessor), maxQueueSize(maxQueueSize),running(true) {
    if(workerCount <= 0){
        throw std::invalid_argument("workerCount must be positive");
    }

    if(maxQueueSize == 0) {
        throw std::invalid_argument("maxQueueSize must be positive");
    }

    workers.reserve(workerCount);
    
    for(int i=0; i < workerCount; i++){
        workers.emplace_back(&PacketTaskThreadPool::workerLoop, this);
    }
}

PacketTaskThreadPool::~PacketTaskThreadPool(){
    shutdown();
}

bool PacketTaskThreadPool::enqueue(const std::string& packetData){
    {
        std::lock_guard<std::mutex> lock(queueMutex);

        // 서버 종료 중이면, 새 packet task 받지 않음.
        if(!running){
            return false;
        }

        // task queue is_full
        if(taskQueue.size() >= maxQueueSize){
            return false; // packetData 처리 작업 등록 실패
        }

        taskQueue.push(packetData); //packet 넘기기
    }

    condition.notify_one();
    return true;
}

void PacketTaskThreadPool::shutdown(){
    {
        std::unique_lock<std::mutex> lock(queueMutex);

        if(!running) return;

        running = false; //동작 종료로 변경
    }

    condition.notify_all(); // 전체 worker 깨우기

    // queue에 들어온 job 처리하고 들어옴 → 안 비워도 됨.

    // 모든 worker threads 종료 위해 대기
    for(auto& worker: workers){
        if(worker.joinable()){
            worker.join();
        }
    }
}

void PacketTaskThreadPool::workerLoop(){
    while(true){
        std::string packetData;
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            
            // 종료 요청이 오거나 일거리 들어올 때까지 worker 대기(wait)
            condition.wait(lock, [this](){
                return !running || !taskQueue.empty(); //서버 종료 or 일거리 O
            });

            // worker thread 바로 퇴근
            if(!running && taskQueue.empty()){ // 서버 종료 + 일거리 X
                return;
            }

            // packetData 들어오면 wake up

            // job queue에서 packetData 작업 하나 꺼내기
            packetData = taskQueue.front();
            taskQueue.pop();

        }

        // PacketProcessor.process(packetData)
        try{
            // lock을 풀고 처리하기 때문에, packet 처리 중에도 다른 worker의 queue 접근을 막지 않음
            ProcessResult result = packetProcessor.process(packetData);

            // std::cout << "Received: " << packetData << std::endl;
            // std::cout << "Status: " << (result.warning ? "WARNING" : "NORMAL") << std::endl;
        } catch(const std::exception& e){
            std::cerr << "Failed to process packet task: " << e.what() << std::endl;
        }

    // 다시 queue 대기
    }
}