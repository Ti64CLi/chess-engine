#ifndef __ZOBRIST_HPP__
#define __ZOBRIST_HPP__

namespace engine {

typedef unsigned long long Key;

class Zobrist {
    private:
        // Pawn on each square, then bishop, knight, rook, queen, king : 2 * 6 * 64 = 2 * 384 = 768
        // then side to move : 1
        // then castling rights : 4
        // then file for valid en passant square = 8
        // Total = 768 + 1 + 4 + 8 = 781
        bool initialized;
        Key keys[781];
    
    public:
        Zobrist() = default;
        
        void init();
        Key getKey(unsigned int);
};

} // namespace engine

#endif
