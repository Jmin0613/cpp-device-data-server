#pragma once

#include "Packet.h"

class WarningDetector {
public :
    static bool isWarning(const Packet& packet);
};