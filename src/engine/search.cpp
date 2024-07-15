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

    std::vector<engine::Move> legalMoves;
    std::vector<unsigned int> orderedIndices;

    game.generateAllLegalMoves(legalMoves, true);

    // DON'T DO THAT, IT CAN BE EMPTY SINCE IT'S ONLY CAPTURES
    /*if (legalMoves.empty()) { // mate
        if (game.isAttackedBy(game.getKingSquare(game.getActiveColor()), engine::getOppositeColor(game.getActiveColor()))) { // checkmate
            return MIN_SCORE;
        }

        return NULL_SCORE; // stalemate
    }*/

    if (orderMoves) {
        game.orderMoves(legalMoves, orderedIndices);
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

        alpha = std::max(score, alpha);
    }

    return alpha;
}

int negaMax(engine::Game &game, unsigned int depth, int alpha, int beta, unsigned long long &moveCount, bool orderMoves) {
    if (depth == 0) {
        return quiesceSearch(game, alpha, beta, moveCount, orderMoves);
    }

    std::vector<Move> legalMoves;

    game.generateAllLegalMoves(legalMoves);

    if (legalMoves.empty()) { // mate
        if (game.isAttackedBy(game.getKingSquare(game.getActiveColor()), engine::getOppositeColor(game.getActiveColor()))) { // checkmate
            return MIN_SCORE;
        }
        
        return NULL_SCORE; // stalemate 
    }

    std::vector<unsigned int> orderedIndices;

    if (orderMoves) {
        game.orderMoves(legalMoves, orderedIndices);
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
    std::vector<Move> legalMoves;

    game.generateAllLegalMoves(legalMoves);

    if (legalMoves.empty()) { // mate
        if (!game.isAttackedBy(game.getKingSquare(game.getActiveColor()), getOppositeColor(game.getActiveColor()))) { // stalemate
            bestMoveValuation.second = 0;
        } // else checkmate

        return bestMoveValuation;
    }

    std::vector<unsigned int> orderedIndices;

    if (orderMoves) {
        game.orderMoves(legalMoves, orderedIndices);
    }

    for (unsigned int i = 0; i < legalMoves.size(); i++) {
        Move &currentMove = legalMoves[i];

        moveCount++;

        if (orderMoves) {
            currentMove = legalMoves[orderedIndices[i]];
        }

        // std::cout << "Current move evaluated : " << utils::caseNameFromId(currentMove.getOriginSquare()) << utils::caseNameFromId(currentMove.getTargetSquare()) << " (valuation = ";

        engine::MoveSaveState savedState = game.doMove(currentMove);
        int moveScore = -negaMax(game, depth - 1, -32000, 32000, moveCount, orderMoves);
        game.undoMove(currentMove, savedState);

        // std::cout << moveScore << "/ best = " << bestMoveValuation.second << ")" << std::endl;

        if (moveScore > bestMoveValuation.second) {
            bestMoveValuation = {currentMove, moveScore};
        }
    }

    return bestMoveValuation;
}

}