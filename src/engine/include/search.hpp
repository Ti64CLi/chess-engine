#ifndef __SEARCH_HPP__
#define __SEARCH_HPP__

#include "engine.hpp"

namespace engine {

const int MIN_SCORE = -32000;
const int MAX_SCORE = +32000;
const int NULL_SCORE = 0;

typedef std::pair<Move, int> MoveValuation;

int quiesceSearch(Game &game, int alpha, int beta, unsigned long long &moveCount, bool orderingMoves = true);
int alphabeta(Game &game, unsigned int maxDepth, unsigned int depth, int alpha, int beta, unsigned long long &moveCount, bool orderingMoves = true);
MoveValuation negaMax(Game &game, unsigned int maxDepth, unsigned int depth, unsigned long long &moveCount, bool orderingMoves = true);

} // namespace engine

#endif