#include <iostream>
#include <exception>

#include "MockDeviceSimulator.h"

int main(){
    try{
        MockDeviceSimulator simulator("127.0.0.1", 9000);

        // packetCnt, 지연(ms)
        simulator.run(10,1000);
    } catch(const std::exception& e){
        std::cerr << "Client error : " << e.what() << std::endl;
        return 1;
    }

    return 0;
}