#include <iostream>
#include <exception>

#include "TcpServer.h"

int main(){
    try{
        TcpServer server(9000);
        server.start();
    } catch(const std::exception& e){
        std::cerr << "Server error : " << e.what() << std::endl;
        return 1;
    }

    return 0;
}