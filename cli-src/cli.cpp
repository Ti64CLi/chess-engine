#include "../src/engine/include/engine.hpp"
#include "../src/engine/include/search.hpp"
#include "../src/engine/include/utils.hpp"
#include <algorithm>
#include <chrono>
#include <iostream>
#include <string>
#include <vector>

#define SEARCH_DEPTH    4
#define QUIESCE_DEPTH   2

typedef unsigned long long u64;

u64 perft(engine::Game &game, unsigned int maxDepth, unsigned int depth, std::vector<u64> &data, bool divide=true, bool infos=true) {
    if (depth == 0) {
        return 1ULL;
    }

    u64 nodes = 0;
    std::vector<engine::Move> pseudoLegalMoves;

    game.generateAllPseudoLegalMoves(pseudoLegalMoves);

    for (engine::Move &move : pseudoLegalMoves) {
        u64 currentLeafs = 0;
        std::vector<u64> tmpData(data.size(), 0);
        engine::Color currentColor = game.getActiveColor();
        engine::MoveSaveState savedState = game.doMove(move);

        if (!game.isAttackedBy(game.getKingSquare(currentColor), game.getActiveColor())) {
            if (depth == 1 && infos) {
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

            currentLeafs = perft(game, maxDepth, depth - 1, tmpData, false, false);
            nodes += currentLeafs;

            for (size_t i = 0; i < data.size() && infos; i++) {
                data[i] += tmpData[i];
            }
        }

        game.undoMove(move, savedState);

        if (divide) {
            for (size_t i = 0; i < (maxDepth - depth); i++) {
                std::cout << "\t";
            }

            std::cout << utils::caseNameFromId(move.getOriginSquare()) << utils::caseNameFromId(move.getTargetSquare()) << " : " << currentLeafs;

            for (size_t i = 0; i < data.size() && infos; i++) {
                std::cout << " " << tmpData[i];
            }

            std::cout << std::endl;
        }
    }

    return nodes;
}

u64 perft_legal(engine::Game &game, unsigned int maxDepth, unsigned int depth, std::vector<u64> &data, bool divide=true, bool infos=true) {
    if (depth == 0) {
        return 1ULL;
    }

    u64 nodes = 0;
    std::vector<engine::Move> legalMoves;

    game.generateAllLegalMoves(legalMoves);

    for (engine::Move &move : legalMoves) {
        u64 currentLeafs = 0;
        std::vector<u64> tmpData(data.size(), 0);
        engine::Color currentColor = game.getActiveColor();

        engine::MoveSaveState savedState = game.doMove(move);

        if (depth == 1 && infos) {
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

        currentLeafs = perft(game, maxDepth, depth - 1, tmpData, false, false);
        nodes += currentLeafs;

        game.undoMove(move, savedState);

        for (size_t i = 0; i < data.size() && infos; i++) {
            data[i] += tmpData[i];
        }

        if (divide) {
            for (size_t i = 0; i < (maxDepth - depth); i++) {
                std::cout << "\t";
            }

            std::cout << utils::caseNameFromId(move.getOriginSquare()) << utils::caseNameFromId(move.getTargetSquare()) << " : " << currentLeafs;

            for (size_t i = 0; i < data.size() && infos; i++) {
                std::cout << " " << tmpData[i];
            }

            std::cout << std::endl;
        }
    }

    return nodes;
}

char asciitolower(char in) {
    if (in <= 'Z' && in >= 'A')
        return in - ('Z' - 'z');
    return in;
}

void tolower(std::string data) {
    std::transform(data.begin(), data.end(), data.begin(), asciitolower);
}

std::string move2str(engine::Move &move) {
    std::string moveStr;

    moveStr += utils::caseNameFromId(move.getOriginSquare());
    moveStr += utils::caseNameFromId(move.getTargetSquare());

    return moveStr;
}

void showBoard(engine::Game &game) {
    std::vector<std::string> position = utils::split(game.getPositionFEN());
    size_t file = 0, rank = 8;

    std::cout << "  +";

    for (size_t i = 0; i < 8; i++) {
        std::cout << "---+";
    }

    std::cout << std::endl;

    for (const char &c : position[0]) {
        if (c == '/') {
            std::cout << "|\n  +";

            for (size_t i = 0; i < 8; i++) {
                std::cout << "---+";
            }

            std::cout << std::endl;

            rank--;
            file = 0;
        } else {
            if (file == 0) {
                std::cout << rank << " ";
            }

            if ('0' <= c && c <= '9') {
                for (size_t i = 0; i < (size_t)(c - '0'); i++) {
                    std::cout << "|   ";
                    file++;
                }
            } else {
                std::cout << "| " << c << " ";
                file++;
            }
        }
    }

    if (position[0].back() != '/') {
        std::cout << "|\n+";

        for (size_t i = 0; i < 8; i++) {
            std::cout << "---+";
        }

        std::cout << std::endl;
    }

    std::cout << "    ";

    for (size_t i = 0; i < 8; i++) {
        std::cout << (char)('a' + i) << "   ";
    }

    std::cout << "\n" << std::endl;

    std::cout << ((game.getActiveColor() == engine::Color::Black) ? "Black" : "White") << "'s turn\n" << std::endl;
}

int main() {
    engine::Game game;
    std::string cmd;
    std::vector<std::string> splitCmd;
    std::vector<engine::Move> moveStack;
    std::vector<engine::MoveSaveState> saveStateStack;

    std::cout << "Chess engine v" << ENGINE_VERSION << "\n" << std::endl;

    for(;;) {
        std::cout << "cmd > ";
        getline(std::cin, cmd);
        std::cout << std::endl;
        tolower(cmd);
        splitCmd = utils::split(cmd);

        if (splitCmd[0] == "show") {
            showBoard(game);
        } else if (splitCmd[0] == "do") {
            for (size_t m = 1; m < splitCmd.size(); m++) {
                if (utils::idFromCaseName(splitCmd[m].substr(0, 2)) == -1 || utils::idFromCaseName(splitCmd[m].substr(2)) == -1) {
                    std::cout << "Invalid move : " << splitCmd[m] << "\n" << std::endl;

                    break;
                }

                engine::Move move(game.str2move(splitCmd[m]));
                engine::MoveSaveState savedState = game.doMove(move);

                moveStack.push_back(move);
                saveStateStack.push_back(savedState);
            }
        } else if (splitCmd[0] == "undo") {
            unsigned int n = 1;

            if (splitCmd.size() > 1) {
                n = std::stoi(splitCmd[1]);
            }

            for (;n > 0 && moveStack.size() > 0; n--, moveStack.pop_back(), saveStateStack.pop_back()) {
                game.undoMove(moveStack.back(), saveStateStack.back());
            }
        } else if (splitCmd[0] == "moves") {
            if (splitCmd.size() == 1) {
                std::cout << "A square name should be provided\n" << std::endl;

                continue;
            }

            unsigned int originSquare = utils::idFromCaseName(splitCmd[1]);

            if (originSquare > 63) {
                std::cout << "Invalid square name : " << splitCmd[1] << "\n";
            } else {
                std::vector<engine::Move> legalMoves;
                game.generateLegalMoves(legalMoves, originSquare);

                for (engine::Move &move : legalMoves) {
                    std::cout << move2str(move) << std::endl;
                }
            }

            std::cout << std::endl;
        } else if (splitCmd[0] == "position") {
            std::string oldPosition = game.getPositionFEN();

            if (splitCmd.size() < 7) {
                std::cout << "current position : " << oldPosition << "\n" << std::endl;

                continue;
            }

            std::string position;
            
            for (size_t i = 1; i < 7; i++) {
                position += splitCmd[i];
                if (i != 6) {
                    position += " ";
                }
            }

            if (game.loadPosition(position) == -1) {
                game.loadPosition(oldPosition);
            }

            std::cout << "position loaded\n" << std::endl;
        } else if (splitCmd[0] == "perft") {
            unsigned int perftDepth = 0;
            bool divide = false, infos = false;

            if (splitCmd.size() > 1) {
                unsigned int offset = 0;

                if (splitCmd[1] == "divide") {
                    divide = true;
                    offset = 1;
                }

                if (splitCmd.size() > 1 + offset) {
                    perftDepth = std::stoi(splitCmd[1 + offset]);
                }

                if (splitCmd.size() > 2 + offset && splitCmd[2 + offset] == "infos") {
                    infos = true;
                }
            }

            std::vector<u64> data = {0, 0, 0, 0, 0};
            auto start = std::chrono::high_resolution_clock::now();

            u64 p = perft(game, perftDepth, perftDepth, data, divide, infos);

            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

            std::cout << "perft(" << perftDepth << ") = " << p;

            if (infos) {
                for (u64 &d : data) {
                    std::cout << " " << d;
                }
            }

            float s = (float)duration.count() / 1000.f;
            std::cout << "\n";
            std::cout << "Time : " << duration.count() << "ms => " << s << "s : " << (float)p / s << " N/s\n" << std::endl;
        } else if (splitCmd[0] == "perft_legal") {
            unsigned int perftDepth = 0;
            bool divide = false, infos = false;

            if (splitCmd.size() > 1) {
                unsigned int offset = 0;

                if (splitCmd[1] == "divide") {
                    divide = true;
                    offset = 1;
                }

                if (splitCmd.size() > 1 + offset) {
                    perftDepth = std::stoi(splitCmd[1 + offset]);
                }

                if (splitCmd.size() > 2 + offset && splitCmd[2 + offset] == "infos") {
                    infos = true;
                }
            }

            std::vector<u64> data = {0, 0, 0, 0, 0};
            auto start = std::chrono::high_resolution_clock::now();

            u64 p = perft_legal(game, perftDepth, perftDepth, data, divide, infos);

            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

            std::cout << "perft(" << perftDepth << ") = " << p;

            if (infos) {
                for (u64 &d : data) {
                    std::cout << " " << d;
                }
            }

            float s = (float)duration.count() / 1000.f;
            std::cout << "\n";
            std::cout << "Time : " << duration.count() << "ms => " << s << "s : " << (float)p / s << " N/s\n" << std::endl;
        } else if (splitCmd[0] == "search") {
            unsigned int searchDepth = SEARCH_DEPTH;

            if (splitCmd.size() > 1) {
                searchDepth = std::stoi(splitCmd[1]);
            }

            std::cout << "With move ordering :" << std::endl;
            {
                unsigned long long moveCount = 0;
                auto start = std::chrono::high_resolution_clock::now();

                engine::MoveValuation bestValuation = engine::negaMax(game, searchDepth, moveCount, true);

                auto end = std::chrono::high_resolution_clock::now();
                auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
                float s = (float)duration.count() / 1000.f;

                std::cout << "Visited " << moveCount << " nodes in " << duration.count() << "ms = " << s << "s => " << (float)moveCount / s << " N/s\n";
                std::cout << "Best move : " << move2str(bestValuation.first) << " (valuation = " << (float)bestValuation.second / 1000.f << ")\n" << std::endl;
            }
            /*std::cout << "Without move ordering :" << std::endl;
            {
                unsigned long long moveCount = 0;
                auto start = std::chrono::high_resolution_clock::now();

                engine::MoveValuation bestValuation = engine::negaMax(game, searchDepth, moveCount, false);

                auto end = std::chrono::high_resolution_clock::now();
                auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
                float s = (float)duration.count() / 1000.f;

                std::cout << "Visited " << moveCount << " nodes in " << duration.count() << "ms = " << s << "s => " << (float)moveCount / s << " N/s\n";
                std::cout << "Best move : " << move2str(bestValuation.first) << " (valuation = " << (float)bestValuation.second / 1000.f << ")\n" << std::endl;
            }*/
        } else if (splitCmd[0] == "exit") {
            break;
        } else if (splitCmd[0] == "help") {
            std::cout << "Available commands :\n";
            std::cout << "\tposition [<fen string>] : show current FEN (or load a position if provided)\n";
            std::cout << "\tshow : display the current board position and turn\n";
            std::cout << "\tdo <move> [<move> ...] : execute the given moves\n";
            std::cout << "\tundo [<n>] : undo <n> moves (1 by default)\n";
            std::cout << "\tmoves <square> : display the legal moves from the given <square>\n";
            std::cout << "\tsearch [<max depth>] : search the best move (<max depth> default is " << SEARCH_DEPTH << ")\n";
            std::cout << "\tperft [divide] [<max depth>] [infos] : execute perft(<max depth>) with [divide] or additional [infos] (<max depth> default is " << 0 << ")\n";
            std::cout << "\tperft_legal [divide] [<max depth>] [infos] : execute perft_legal(<max depth>) with [divide] or additional [infos] (<max depth> default is " << 0 << ")\n";
            std::cout << "\t\t\tThe difference between perft and perft_legal is that perft_legal generate legal moves\n";
            std::cout << "\t\t\twhich can be expensive since to check if a move is legal, the engine do the move,\n";
            std::cout << "\t\t\tthen check if it lefts the king in check, and then undo the move, which adds a second\n";
            std::cout << "\t\t\tlayer of do/undo\n" << std::endl;
        }
    }

    return 0;
}