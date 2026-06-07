#pragma once

#include <string>

class FakeDeviceClient{
private :
    std::string serverIp; // 접속할 서버 IP
    int serverPort; // 접속할 서버 Port

public :
    // 생성자
    FakeDeviceClient(const std::string& serverIp, int serverPort);

    // 패킷 전송
    void sendPacket(const std::string& packetData);
};