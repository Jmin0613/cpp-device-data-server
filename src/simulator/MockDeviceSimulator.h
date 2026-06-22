#pragma once

#include <string>

class MockDeviceSimulator{
private :
    std::string serverIp;
    int serverPort;

public :
    // 생성자
    MockDeviceSimulator(const std::string& serverIp, int serverPort);

    std::string generatePacket(); //packet 생성

    // legacy
    void sendPacket(const std::string& packet); //생성된 packet, server로 전송
    // legacy
    void run(int packetCount, int delayMs); // 지연시간, 전송패킷 수 지정하여 실행

    // 단일 client
    void runContinuous(int packetCount, int delayMs); // 패킷 다중 전송 실행

    // 테스트용 (다중 client)
    void runContinuous(int clienCount, int packetCount, int delayMs);
};