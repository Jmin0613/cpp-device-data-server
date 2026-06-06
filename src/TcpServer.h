#pragma once

class TcpServer{
private :
    int port;

public :
    // 생성자
    explicit TcpServer(int port);
    
    // 서버 구동
    void start();
};