#include "include/search.hpp"
#include "include/engine.hpp"
#include "include/evaluation.hpp"
#include "include/movesgeneration.hpp"
#include "include/movesordering.hpp"
#include "include/transpositiontable.hpp"
#include <algorithm>
#include <vector>

static engine::TTable ttable;

namespace engine {

int quiesceSearch(Game &game, int alpha, int beta, unsigned long long &moveCount, bool orderingMoves) {
    int stand_pat = evaluate(game);

    if (stand_pat >= beta) {
        return beta;
    }
    if (alpha < stand_pat) {
        alpha = stand_pat;
    }

    std::vector<engine::Move> legalMoves;
    std::vector<unsigned int> orderedIndices;

    generateAllLegalMoves(game, legalMoves, true);

    if (orderingMoves) {
        orderMoves(game, legalMoves, orderedIndices);
    }

    for (unsigned int i = 0; i < legalMoves.size(); i++) {
        Move &currentMove = legalMoves[i];

        moveCount++;

        if (orderingMoves) {
            currentMove = legalMoves[orderedIndices[i]];
        }

        engine::MoveSaveState savedState = game.doMove(currentMove);
        int score = -quiesceSearch(game, -beta, -alpha, moveCount, orderingMoves);
        game.undoMove(currentMove, savedState);

        if (score >= beta) {
            return beta;
        }

        alpha = std::max(score, alpha);
    }

    return alpha;
}

int alphabeta(engine::Game &game, unsigned int maxDepth, unsigned int depth, int alpha, int beta, unsigned long long &moveCount, bool orderingMoves) {
    if (depth == 0) {
        return quiesceSearch(game, alpha, beta, moveCount, orderingMoves);
    }

    int originalAlpha = alpha;

    TTEntry entry = ::ttable.getEntry(game.getHash());

    if (game.getHash() == entry.hash && entry.depth >= depth) {
        if (entry.entryType == TTEntryType::Exact) {
            return entry.valuation;
        } else if (entry.entryType == TTEntryType::Lower) {
            alpha = std::max(alpha, entry.valuation);
        } else if (entry.entryType == TTEntryType::Upper) {
            beta = std::min(beta, entry.valuation);
        }

        if (alpha >= beta) {
            return entry.valuation;
        }
    }

    std::vector<Move> legalMoves;

    generateAllLegalMoves(game, legalMoves);

    engine::Result result = game.result(legalMoves);

    if (result != engine::Result::Undecided) {
        int evaluation = 0;

        switch (result) {
            case engine::Result::CheckMate: {
                evaluation = MIN_SCORE;
                evaluation += maxDepth - depth;

                break;
            }
            default:
                break;
        }

        return evaluation;
    }

    std::vector<unsigned int> orderedIndices;

    if (orderingMoves) {
        orderMoves(game, legalMoves, orderedIndices);
    }

    MoveValuation bestMoveValuation = {Move(), MIN_SCORE};

    for (unsigned int i  = 0; i < legalMoves.size(); i++) {
        Move &currentMove = legalMoves[i];

        moveCount++;

        if (orderingMoves) {
            currentMove = legalMoves[orderedIndices[i]];
        }

        engine::MoveSaveState savedState = game.doMove(currentMove);
        int evaluation = -alphabeta(game, maxDepth, depth - 1, -beta, -alpha, moveCount, orderingMoves);
        game.undoMove(currentMove, savedState);

        if (evaluation >= bestMoveValuation.second) {
            bestMoveValuation.second = evaluation;
            bestMoveValuation.first = currentMove;
        }

        alpha = std::max(alpha, evaluation);

        if (alpha >= beta) {
            break;
        }
    }

    ::ttable.addEntry(game.getHash(), bestMoveValuation.first, depth, bestMoveValuation.second, originalAlpha, beta);

    return bestMoveValuation.second;
}

MoveValuation negaMax(Game &game, unsigned int maxDepth, unsigned int depth, unsigned long long &moveCount, bool orderingMoves) {
    MoveValuation bestMoveValuation = {Move(), MIN_SCORE};
    std::vector<Move> legalMoves;

    generateAllLegalMoves(game, legalMoves);

    engine::Result result = game.result(legalMoves);

    if (result != engine::Result::Undecided) {
        switch (result) {
            case engine::Result::Draw: {
                bestMoveValuation.second = 0;
                bestMoveValuation.second += (game.getHash() & 0x2) - 1;

                break;
            }
            case engine::Result::CheckMate: {
                bestMoveValuation.second = MIN_SCORE;
                bestMoveValuation.second += (maxDepth - depth); // distance to mate
                // DTM should always be 0 here, but I'm keeping it for clarity sake

                break;
            }
            default:
                break;
        }

        return bestMoveValuation;
    }

    std::vector<unsigned int> orderedIndices;

    if (orderingMoves) {
        orderMoves(game, legalMoves, orderedIndices);
    }

    for (unsigned int i = 0; i < legalMoves.size(); i++) {
        Move &currentMove = legalMoves[i];

        moveCount++;

        if (orderingMoves) {
            currentMove = legalMoves[orderedIndices[i]];
        }

        // std::cout << "Current move evaluated : " << utils::caseNameFromId(currentMove.getOriginSquare()) << utils::caseNameFromId(currentMove.getTargetSquare()) << " (valuation = ";

        engine::MoveSaveState savedState = game.doMove(currentMove);
        int moveScore = -alphabeta(game, maxDepth, depth - 1, -32000, 32000, moveCount, orderingMoves);
        game.undoMove(currentMove, savedState);

        // std::cout << moveScore << "/ best = " << bestMoveValuation.second << ")" << std::endl;

        if (moveScore >= bestMoveValuation.second) {
            bestMoveValuation = {currentMove, moveScore};
        }
    }

    return bestMoveValuation;
}

}