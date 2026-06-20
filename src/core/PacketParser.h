#pragma once

#include <string>
#include "Packet.h"

class PacketParser {
public :
    static Packet parser(const std::string& rawData);
};