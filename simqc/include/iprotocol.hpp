#pragma once
#include "pulse.hpp"

class IProtocol {
public:
    virtual ~IProtocol() = default;
    virtual Pulse preparePhoton() = 0;
    virtual Bit measurePhoton(const Pulse& photon) = 0;
    virtual void siftKeys() = 0;
};
