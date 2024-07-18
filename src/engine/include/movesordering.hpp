#ifndef __MOVESORDERING_HPP__
#define __MOVESORDERING_HPP__

#include "engine.hpp"
#include "move.hpp"

namespace engine {

int guessScore(Game &game, Move &move);
void orderMoves(Game &game, std::vector<Move> &moves, std::vector<unsigned int> &orderedIndices);

} // namespace engine

#endif