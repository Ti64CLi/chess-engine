#include "include/search.hpp"
#include "include/engine.hpp"
#include <vector>

namespace engine {

int quiesceSearch(Game &game, int alpha, int beta, unsigned long long &moveCount, bool orderMoves) {
    int stand_pat = game.evaluate();

    if (stand_pat >= beta) {
        return beta;
    }
    if (alpha < stand_pat) {
        alpha = stand_pat;
    }

    std::vector<engine::Move> legalMoves = game.generateAllLegalMoves(true);

    if (legalMoves.empty()) { // mate
        if (game.isAttackedBy(game.getKingSquare(game.getActiveColor()), engine::getOppositeColor(game.getActiveColor()))) { // checkmate
            return MIN_SCORE;
        }

        return NULL_SCORE; // stalemate
    }

    std::vector<unsigned int> orderedIndices;

    if (orderMoves) {
        orderedIndices = game.orderMoves(legalMoves);
    }

    for (unsigned int i = 0; i < legalMoves.size(); i++) {
        Move &currentMove = legalMoves[i];

        moveCount++;

        if (orderMoves) {
            currentMove = legalMoves[orderedIndices[i]];
        }

        engine::MoveSaveState savedState = game.doMove(currentMove);
        int score = -quiesceSearch(game, -beta, -alpha, moveCount, orderMoves);
        game.undoMove(currentMove, savedState);

        if (score >= beta) {
            return beta;
        }
        if (score > alpha) {
            alpha = score;
        }
    }

    return alpha;
}

int negaMax(engine::Game &game, unsigned int depth, int alpha, int beta, unsigned long long &moveCount, bool orderMoves) {
    if (depth == 0) {
        return quiesceSearch(game, alpha, beta, moveCount, orderMoves);
    }

    std::vector<Move> legalMoves = game.generateAllLegalMoves();

    if (legalMoves.empty()) { // mate
        if (game.isAttackedBy(game.getKingSquare(game.getActiveColor()), engine::getOppositeColor(game.getActiveColor()))) { // checkmate
            return MIN_SCORE;
        }
        
        return NULL_SCORE; // stalemate
    }

    std::vector<unsigned int> orderedIndices;

    if (orderMoves) {
        orderedIndices = game.orderMoves(legalMoves);
    }

    for (unsigned int i  = 0; i < legalMoves.size(); i++) {
        Move &currentMove = legalMoves[i];

        moveCount++;

        if (orderMoves) {
            currentMove = legalMoves[orderedIndices[i]];
        }

        engine::MoveSaveState savedState = game.doMove(currentMove);
        int evaluation = -negaMax(game, depth - 1, -beta, -alpha, moveCount, orderMoves);
        game.undoMove(currentMove, savedState);

        if (evaluation >= beta) {
            return beta;
        }

        alpha = std::max(alpha, evaluation);
    }

    return alpha;
}

MoveValuation negaMax(Game &game, unsigned int depth, unsigned long long &moveCount, bool orderMoves) {
    MoveValuation bestMoveValuation = {Move(), MIN_SCORE};
    std::vector<Move> legalMoves =  game.generateAllLegalMoves();

    if (legalMoves.empty()) { // mate
        if (!game.isAttackedBy(game.getKingSquare(game.getActiveColor()), getOppositeColor(game.getActiveColor()))) { // stalemate
            bestMoveValuation.second = 0;
        } // else checkmate

        return bestMoveValuation;
    }

    std::vector<unsigned int> orderedIndices;

    if (orderMoves) {
        orderedIndices = game.orderMoves(legalMoves);
    }

    for (unsigned int i = 0; i < legalMoves.size(); i++) {
        Move &currentMove = legalMoves[i];

        moveCount++;

        if (orderMoves) {
            currentMove = legalMoves[orderedIndices[i]];
        }

        engine::MoveSaveState savedState = game.doMove(currentMove);
        int moveScore = -negaMax(game, depth - 1, -32000, 32000, moveCount, orderMoves);
        game.undoMove(currentMove, savedState);

        if (moveScore > bestMoveValuation.second) {
            bestMoveValuation.first = currentMove;
            bestMoveValuation.second = moveScore;
        }
    }

    return bestMoveValuation;
}

}