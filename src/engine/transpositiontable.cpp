#include "include/transpositiontable.hpp"
#include "include/zobrist.hpp"

namespace engine {

TTEntry TTable::getEntry(Key &key) {
    return this->table[key % TTABLE_SIZE];
}

void TTable::addEntry(Key &key, Move &move, unsigned int depth, int valuation, int alpha, int beta) {
    TTEntry entry;

    entry.move = move;
    entry.depth = depth;
    entry.valuation = valuation;
    entry.hash = key;

    if (valuation >= beta) {
        entry.entryType = TTEntryType::Lower;
    } else if (valuation <= alpha) {
        entry.entryType = TTEntryType::Upper;
    } else {
        entry.entryType = TTEntryType::Exact;
    }

    this->table[key % TTABLE_SIZE] = entry;
}

} // namespace engine