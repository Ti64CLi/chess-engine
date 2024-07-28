#ifndef __BITBOARDS_HPP__
#define __BITBOARDS_HPP__

#include <cassert>

namespace engine {

typedef unsigned long long bitboard_t;

const bitboard_t fileABB = 0x0101010101010101ULL;
const bitboard_t fileBBB = fileABB << 1;
const bitboard_t fileCBB = fileABB << 2;
const bitboard_t fileDBB = fileABB << 3;
const bitboard_t fileEBB = fileABB << 4;
const bitboard_t fileFBB = fileABB << 5;
const bitboard_t fileGBB = fileABB << 6;
const bitboard_t fileHBB = fileABB << 7;

const bitboard_t rank1BB = 0xFFULL;
const bitboard_t rank2BB = rank1BB << (1 << 3);
const bitboard_t rank3BB = rank1BB << (2 << 3);
const bitboard_t rank4BB = rank1BB << (3 << 3);
const bitboard_t rank5BB = rank1BB << (4 << 3);
const bitboard_t rank6BB = rank1BB << (5 << 3);
const bitboard_t rank7BB = rank1BB << (6 << 3);
const bitboard_t rank8BB = rank1BB << (7 << 3);

enum Square {
    A1, A2, A3, A4, A5, A6, A7, A8,
    B1, B2, B3, B4, B5, B6, B7, B8,
    C1, C2, C3, C4, C5, C6, C7, C8,
    D1, D2, D3, D4, D5, D6, D7, D8,
    E1, E2, E3, E4, E5, E6, E7, E8,
    F1, F2, F3, F4, F5, F6, F7, F8,
    G1, G2, G3, G4, G5, G6, G7, G8,
    H1, H2, H3, H4, H5, H6, H7, H8,
    None,
};

enum File {
    A, B, C, D, E, F, G, H,
};

enum Rank {
    R1, R2, R3, R4, R5, R6, R7, R8,
};

bitboard_t operator&(bitboard_t bitboard, Square square);
bitboard_t operator|(bitboard_t bitboard, Square square);
bitboard_t operator^(bitboard_t bitboard, Square square);
bitboard_t &operator&=(bitboard_t &bitboard, Square square);
bitboard_t &operator|=(bitboard_t &bitboard, Square square);
bitboard_t &operator^=(bitboard_t &bitboard, Square square);

} // namespace engine

#endif