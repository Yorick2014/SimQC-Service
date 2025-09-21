#include <vector>
#include "iprotocol.hpp"
#include "pulse.hpp"

class BB84 : public IProtocol {
    std::vector<int> aliceBits, bobBits;
    std::vector<Basis> aliceBases, bobBases;
};
