#ifndef __MOVESGENERATION_HPP__
#define __MOVESGENERATION_HPP__

#include "engine.hpp"
#include "move.hpp"
#include <vector>

namespace engine {

void generatePseudoLegalMoves(Game &game, std::vector<Move> &pseudoLegalMoves, unsigned int selectedCaseId);
void generateAllPseudoLegalMoves(Game &game, std::vector<Move> &pseudoLegalMoves);
void generateLegalMoves(Game &game, std::vector<Move> &legalMoves, unsigned int selectedCaseId, bool capturesOnly = false);
void generateAllLegalMoves(Game &game, std::vector<Move> &legalMoves, bool capturesOnly = false);

} // namespace engine

#endif