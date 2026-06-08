#pragma once

#include "PacketProcessor.h"

class ClientSession{
private :
    int clientSocket;
    PacketProcessor& packetProcessor;

public :
    // 생성자
    ClientSession(int clientSocket, PacketProcessor& packetProcessor);
    ~ClientSession();

    // 복사 막기
    ClientSession(const ClientSession&) = delete;
    ClientSession& operator=(const ClientSession&) = delete;

    // handleClient
    void handle();
};