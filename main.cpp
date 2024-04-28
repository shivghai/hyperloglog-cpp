#include "hyperloglog.hpp"

#include <iostream>
#include <cstdlib> // for rand()
#include "external/murmur3.h" // for the hash fn


int main() {
    auto hll = HLL::HyperLogLog<std::string>(4,
        [](std::string const& val) -> uint32_t {
                uint32_t hash;
                MurmurHash3_x86_32(val.data(), val.size(), rand(), (void*) &hash);
                return hash;
            }
        );

    for (size_t i = 0; i < 5355; ++i) {
        auto tmp = std::to_string(i);
        hll.add(tmp);
    }
    std::cout << "Cardinality: " << hll.get_cardinality() << std::endl;
    return 0;
}