#include <iostream>
#include <unistd.h>
#include <exception>

#include "ThreadPool.h"
#include "ClientSession.h"

ThreadPool::ThreadPool(int workerCount, size_t maxQueueSize, PacketProcessor& packetProcessor)
: packetProcessor(packetProcessor), maxQueueSize(maxQueueSize), running(true) {
    if(workerCount <= 0){
        throw std::invalid_argument("workerCount must be positive");
    }

    if(maxQueueSize == 0) {
        throw std::invalid_argument("maxQueueSize must be positive");
    }
    
    workers.reserve(workerCount);

    for(int i=0; i< workerCount; ++i){
        workers.emplace_back(&ThreadPool::workerLoop, this);
    }
}

ThreadPool::~ThreadPool() {
    shutdown();
}

bool ThreadPool::enqueue(int clientSocket){
    {
        std::lock_guard<std::mutex> lock(queueMutex);

        // 서버 종료 중이면 즉시 접속 거절
        if(!running){
            return false;
        }

        // std::cout << "[ThreadPool] job queue size: "
        // << clientQueue.size() << "/"
        // << maxQueueSize << std::endl;

        // client queue is_full
        if(clientQueue.size() >= maxQueueSize){
            return false; // 접속 거절. TcpServer로 false 반환하여 clientSocket close.
        }

        clientQueue.push(clientSocket); //클라이언트 소켓 번호 넘기기

        // std::cout << "[ThreadPool] enqueue success. job queue size: " 
        // << clientQueue.size() << "/"
        // << maxQueueSize << std::endl;
    }

    condition.notify_one();
    return true;
}

void ThreadPool::workerLoop(){
    // worker thread 생성 후 바로 while 무한 루프 진입하여 대기.
    while(true){
        // 처리할 client socket 번호
        int clientSocket = -1;

        {
            std::unique_lock<std::mutex> lock(queueMutex);
            
            // 종료 요청이 오거나 일거리 들어올 때까지 worker 대기(wait)
            condition.wait(lock, [this](){
                return !running || !clientQueue.empty(); //서버 종료 or 일거리 O
            });

            // worker thread 바로 퇴근
            if(!running && clientQueue.empty()){ // 서버 종료 + 일거리 X
                return;
            }

            // job queue에서 clientSocket 작업 하나 꺼내기
            clientSocket = clientQueue.front();
            clientQueue.pop();
        }

        try{
            // ClientSession 만들어, 통신(recv/process/close) 시작
            // lock 풀고하기 때문에, 이 worker가 recv()에서 멈춰있어도 다른 worker들이나 소멸자가 방해받지 x
            ClientSession session(clientSocket, packetProcessor);
            session.handle();
        } catch(const std::exception& e){
            std::cerr << "Worker thread error: " << e.what() << std::endl;
            // close(clientSocket); → session에서 close처리.
        }
    }
}

void ThreadPool::shutdown(){
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