#include "include/zobrist.hpp"
#include <random>

namespace engine {


void Zobrist::init() {
    if (this->initialized) {
        return;
    }

    std::mt19937_64 mt;

    for (unsigned int i = 0; i < 781; i++) {
        this->keys[i] = mt();
    }

    this->initialized = true;

}

Key Zobrist::getKey(unsigned int offset) {
    return this->keys[offset];
}

} // namespace engine
