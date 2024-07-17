#include "include/zobrist.hpp"
#include <random>

namespace engine {

bool Zobrist::initialized = false;

void Zobrist::init() {
    if (Zobrist::initialized) {
        return;
    }

    std::mt19937_64 mt;

    for (unsigned int i = 0; i < 781; i++) {
        Zobrist::keys[i] = mt();
    }

    Zobrist::initialized = true;
}

} // namespace engine
