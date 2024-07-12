#include "include/ai.hpp"
#include "include/engine.hpp"
#include <limits>
#include <vector>

namespace ai {

AI::AI(engine::Game &game) : game(game) {}

int AI::quiesceSearch(int alpha, int beta, unsigned int maxDepth) {
    int stand_pat = this->game.evaluate();

    if (maxDepth == 0) {
        return stand_pat;
    }

    if (stand_pat >= beta) {
        return beta;
    }
    if (alpha < stand_pat) {
        alpha = stand_pat;
    }

    std::vector<engine::Move> legalMoves = this->game.generateAllLegalMovesBeforeMate();

    for (engine::Move &move : legalMoves) {
        if (!move.isCapture()) {
            continue;
        }

        engine::MoveSaveState savedState = this->game.doMove(move);
        int score = -this->quiesceSearch(-beta, -alpha, maxDepth - 1);
        this->game.undoMove(move, savedState);

        if (score >= beta) {
            return beta;
        }
        if (score > alpha) {
            alpha = score;
        }
    }

    return alpha;
}

int AI::alphaBeta(int alpha, int beta, unsigned int depth) {
    if (depth == 0) {
        return this->quiesceSearch(alpha, beta, 2);
    }

    std::vector<engine::Move> legalMoves = this->game.generateAllLegalMovesBeforeMate();

    for (engine::Move &move : legalMoves) {
        engine::MoveSaveState savedState = this->game.doMove(move);
        int score = -this->alphaBeta(-beta, -alpha, depth - 1);
        this->game.undoMove(move, savedState);

        if (score >= beta) {
            return score;
        }
        if (score > alpha) {
            alpha = score;
        }
    }

    return alpha;
}

MoveValuation AI::negaMax(unsigned int depth) {
    if (depth == 0) {
        return {engine::Move(), this->game.evaluate()};
    }

    engine::Move bestMove;
    int maxScore = std::numeric_limits<int>::min();
    std::vector<engine::Move> legalMoves = this->game.generateAllLegalMovesBeforeMate();

    for (engine::Move &move : legalMoves) {
        engine::MoveSaveState savedState = this->game.doMove(move);
        int score = -this->alphaBeta(std::numeric_limits<int>::min(), std::numeric_limits<int>::max(), depth - 1);
        this->game.undoMove(move, savedState);

        if (score > maxScore) {
            maxScore = score;
            bestMove.setOriginSquare(move.getOriginSquare());
            bestMove.setTargetSquare(move.getTargetSquare());
            bestMove.setCapturedPiece(move.getCapturedPiece());
            bestMove.setFlags(move.getFlags());
        }
    }

    return {bestMove, maxScore};
}

}