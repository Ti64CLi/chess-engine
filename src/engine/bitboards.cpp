#include "include/bitboards.hpp"

namespace engine {

bitboard_t operator&(bitboard_t bitboard, Square square) {
    return bitboard & (1 << square);
}

bitboard_t operator|(bitboard_t bitboard, Square square) {
    return bitboard | (1 << square);
}

bitboard_t operator^(bitboard_t bitboard, Square square) {
    return bitboard ^ (1 << square);
}

bitboard_t &operator&=(bitboard_t &bitboard, Square square) {
    return (bitboard &= 1 << square);
}

bitboard_t &operator|=(bitboard_t &bitboard, Square square) {
    return (bitboard |= 1 << square);
}

bitboard_t &operator^=(bitboard_t &bitboard, Square square) {
    return (bitboard ^= 1 << square);
}

} // namespace engine