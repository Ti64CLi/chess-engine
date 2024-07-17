#ifndef __ZOBRIST_HPP__
#define __ZOBRIST_HPP__

#include "engine.hpp"

namespace engine {

typedef unsigned long long Key;

class Zobrist {
    private:
        // Pawn on each square, then bishop, knight, rook, queen, king : 2 * 6 * 64 = 2 * 384 = 768
        // then side to move : 1
        // then castling rights : 4
        // then file for valid en passant square = 8
        // Total = 768 + 1 + 4 + 8 = 781
        static Key keys[781];
        static bool initialized;
    
    public:
        static void init();

        Key generate_key(Game &game);
        void key_do_move(Game &game, Key &key, Move &move, MoveSaveState &savedState);
        void key_undo_move(Game &game, Key &key, Move &move, MoveSaveState &savedState);
};

} // namespace engine

#endif
