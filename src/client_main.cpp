#include <iostream>
#include <exception>

#include "FakeDeviceClient.h"

int main(){
    try{
        FakeDeviceClient client("127.0.0.1", 9000);

        std::string packetData = "SAT-003|2026-06-07T17:53:17|TEMP|64.2";

        client.sendPacket(packetData);
    } catch(const std::exception& e){
        std::cerr << "Client error : " << e.what() << std::endl;
        return 1;
    }

    return 0;
}