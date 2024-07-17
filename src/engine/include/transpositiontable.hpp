#ifndef __TRANSPOSITION_TABLE_HPP__
#define __TRANSPOSITION_TABLE_HPP__

#include "engine.hpp"
#include "zobrist.hpp"

#define TTABLE_SIZE 1000000

namespace engine {

enum TTEntryType {
    Lower,
    Upper,
    Exact,
};

struct TTEntry {
    Move move;
    unsigned int depth;
    TTEntryType entryType;
    int valuation;
    Key hash;
};

class TTable {
    private:
        TTEntry table[TTABLE_SIZE];
    
    public:
        TTable() = default;

        TTEntry getEntry(Key &key);
        void addEntry(Key &key, Move &move, unsigned int depth, int valuation, int alpha, int beta);
};

} // namespace engine

#endif