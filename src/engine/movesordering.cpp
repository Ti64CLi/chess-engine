#include "include/engine.hpp"
#include "include/evaluation.hpp"
#include "include/piece.hpp"
#include "include/movesordering.hpp"
#include <algorithm>

namespace engine {


int guessScore(Game &game, Move &move) {
    int guessedScore = 0;

    PieceType movedPiece = game.getPiece(move.getOriginSquare()).pieceType;
    PieceType capturedPiece = move.getCapturedPiece().pieceType;

    if (move.isCapture()) { // capturing a piece of better value could be a good move
        guessedScore += 10 * pieceTypeValue[capturedPiece].first - pieceTypeValue[movedPiece].first;
    }

    if (move.isPromotion()) { // promoting could be a good move
        guessedScore += pieceTypeValue[move.getPromotedPiece()].first;
    }

    if (game.isAttackedBy(move.getTargetSquare(), getOppositeColor(game.getActiveColor()))) { // getting threatened should be avoided
        guessedScore -= 2 * pieceTypeValue[movedPiece].first;
    }

    if (game.isAttackedBy(game.getKingSquare(getOppositeColor(game.getActiveColor())), game.getActiveColor())) { // checking the king should be a good move
        guessedScore += pieceTypeValue[movedPiece].first;
    }

    return guessedScore;
}

void orderMoves(Game &game, std::vector<Move> &moves, std::vector<unsigned int> &orderedIndices) {
    std::vector<Move> orderedMoves;
    std::vector<int> guessedScores;

    for (unsigned int i = 0; i < moves.size(); i++) {
        orderedIndices.push_back(i);
        guessedScores.push_back(guessScore(game, moves[i]));
    }

    std::sort(orderedIndices.begin(), orderedIndices.end(), [&](unsigned int &i, unsigned int &j) {
        return guessedScores[i] > guessedScores[j];
    });
}


} // namespace engine