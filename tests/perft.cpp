#include "../src/engine/include/engine.hpp"
#include "../src/engine/include/utils.hpp"
#include "../src/engine/include/movesgeneration.hpp"
#include <iostream>

typedef unsigned long long u64;

u64 perft(engine::Game &game, unsigned int maxDepth, unsigned int depth, std::vector<u64> &data, bool print=true) {
    if (depth == 0) {
        return 1ULL;
    }

    u64 nodes = 0;
    std::vector<engine::Move> pseudoLegalMoves;

    generateAllPseudoLegalMoves(game, pseudoLegalMoves);

    for (engine::Move &move : pseudoLegalMoves) {
        u64 currentLeafs = 0;
        std::vector<u64> tmpData(data.size(), 0);
        engine::Color currentColor = game.getActiveColor();
        engine::MoveSaveState savedState = game.doMove(move);

        if (!game.isAttackedBy(game.getKingSquare(currentColor), game.getActiveColor())) {
            if (depth == 1) {
                if (move.isCapture()) {
                    data[0] += 1;
                }
                if (move.getTargetSquare() == savedState.enPassantTargetSquare && game.getPiece(move.getTargetSquare()).pieceType == engine::PieceType::Pawn) {
                    data[1] += 1;
                }
                if (move.isCastling()) {
                    data[2] += 1;
                }
                if (move.isPromotion()) {
                    data[3] += 1;
                }
                if (game.isAttackedBy(game.getKingSquare(game.getActiveColor()), currentColor)) {
                    data[4] += 1;
                }
            }

            currentLeafs = perft(game, maxDepth, depth - 1, tmpData, false);
            nodes += currentLeafs;
            for (size_t i = 0; i < data.size(); i++) {
                data[i] += tmpData[i];
            }
        }

        game.undoMove(move, savedState);

        if (print) {
            for (size_t i = 0; i < (maxDepth - depth); i++) {
                std::cout << "\t";
            }

            std::cout << utils::caseNameFromId(move.getOriginSquare()) << utils::caseNameFromId(move.getTargetSquare()) << " : " << currentLeafs;

            for (size_t i = 0; i < data.size(); i++) {
                std::cout << " " << tmpData[i];
            }

            std::cout << std::endl;
        }
    }

    return nodes;
}

u64 perft_unopt(engine::Game &game, unsigned int maxDepth, unsigned int depth, std::vector<u64> &data, bool print=true) {
    if (depth == 0) {
        return 1ULL;
    }

    u64 nodes = 0;
    std::vector<engine::Move> legalMoves;

    generateAllLegalMoves(game, legalMoves);

    for (engine::Move &move : legalMoves) {
        u64 currentLeafs = 0;
        std::vector<u64> tmpData(data.size(), 0);
        engine::Color currentColor = game.getActiveColor();
        engine::MoveSaveState savedState = game.doMove(move);

        if (depth == 1) {
            if (move.isCapture()) {
                data[0] += 1;
            }
            if (move.getTargetSquare() == savedState.enPassantTargetSquare && game.getPiece(move.getTargetSquare()).pieceType == engine::PieceType::Pawn) {
                data[1] += 1;
            }
            if (move.isCastling()) {
                data[2] += 1;
            }
            if (move.isPromotion()) {
                data[3] += 1;
            }
            if (game.isAttackedBy(game.getKingSquare(game.getActiveColor()), currentColor)) {
                data[4] += 1;
            }
        }

        currentLeafs = perft(game, maxDepth, depth - 1, tmpData, false);
        nodes += currentLeafs;
        for (size_t i = 0; i < data.size(); i++) {
            data[i] += tmpData[i];
        }

        game.undoMove(move, savedState);

        if (print) {
            for (size_t i = 0; i < (maxDepth - depth); i++) {
                std::cout << "\t";
            }

            std::cout << utils::caseNameFromId(move.getOriginSquare()) << utils::caseNameFromId(move.getTargetSquare()) << " : " << currentLeafs;

            for (size_t i = 0; i < data.size(); i++) {
                std::cout << " " << tmpData[i];
            }

            std::cout << std::endl;
        }
    }

    return nodes;
}

int main() {
    std::string fen, perftDepth;
    engine::Game game;
    unsigned int depth;

    std::cout << "Enter position : ";
    getline(std::cin, fen);
    
    if (fen.size() == 0) {
        fen = engine::startPosition;
    }

    if (game.loadPosition(fen) == -1) {
        return -1;
    }

    std::cout << "Perft depth : ";
    getline(std::cin, perftDepth);

    if (perftDepth.size() == 0) {
        depth = 1;
    } else {
        depth = std::stoi(perftDepth);
    }

    std::vector<u64> datas = {0, 0, 0, 0, 0}; // captures enpassants castles promotions checks
    u64 p = perft(game, depth,  depth, datas);
    
    std::cout << "perft(" << depth << ") = " << p;

    for (u64 &data : datas) {
        std::cout << " " << data;
    }

    std::cout << std::endl;

    std::vector<u64> datas_unopt = {0, 0, 0, 0, 0}; // captures enpassants castles promotions checks
    u64 p_unopt = perft_unopt(game, depth,  depth, datas_unopt);
    
    std::cout << "perft(" << depth << ") = " << p_unopt;

    for (u64 &data : datas_unopt) {
        std::cout << " " << data;
    }

    std::cout << std::endl;

    return 0;
}