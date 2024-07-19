#ifndef __ENGINE_HPP__
#define __ENGINE_HPP__

#include "move.hpp"
#include "piece.hpp"
#include "zobrist.hpp"
#include <unordered_map>
#include <vector>
#include <string>


#define ENGINE_VERSION "0.1"

#define XX 64

namespace engine {

static const unsigned int mailbox10x12[120] = {
    XX, XX, XX, XX, XX, XX, XX, XX, XX, XX,
    XX, XX, XX, XX, XX, XX, XX, XX, XX, XX,
    XX,  0,  1,  2,  3,  4,  5,  6,  7, XX,
    XX,  8,  9, 10, 11, 12, 13, 14, 15, XX,
    XX, 16, 17, 18, 19, 20, 21, 22, 23, XX,
    XX, 24, 25, 26, 27, 28, 29, 30, 31, XX,
    XX, 32, 33, 34, 35, 36, 37, 38, 39, XX,
    XX, 40, 41, 42, 43, 44, 45, 46, 47, XX,
    XX, 48, 49, 50, 51, 52, 53, 54, 55, XX,
    XX, 56, 57, 58, 59, 60, 61, 62, 63, XX,
    XX, XX, XX, XX, XX, XX, XX, XX, XX, XX,
    XX, XX, XX, XX, XX, XX, XX, XX, XX, XX,
};

static const unsigned int mailbox8x8[64] = {
    21, 22, 23, 24, 25, 26, 27, 28,
    31, 32, 33, 34, 35, 36, 37, 38,
    41, 42, 43, 44, 45, 46, 47, 48,
    51, 52, 53, 54, 55, 56, 57, 58,
    61, 62, 63, 64, 65, 66, 67, 68,
    71, 72, 73, 74, 75, 76, 77, 78,
    81, 82, 83, 84, 85, 86, 87, 88,
    91, 92, 93, 94, 95, 96, 97, 98,
};

static std::unordered_map<engine::Color, std::vector<std::pair<unsigned int, unsigned int>>> castlingRookSquareIds = {
    {engine::Color::Black, {{63, 61}, {56, 59}}},
    {engine::Color::White, {{7, 5}, {0, 3}}},
};

const std::string startPosition = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

char pieceSymbol(Piece &piece);

struct MoveSaveState {
    std::unordered_map<Color, std::vector<bool>> castle;
    unsigned int enPassantTargetSquare;
    unsigned int halfMoveNumber;
    unsigned int fullMoveNumber;
    std::unordered_map<Color, unsigned int> kingSquare;
};

enum Result {
    Draw,
    CheckMate,
    StaleMate,
    Undecided,
};

class Game {
    private:
        std::vector<Piece> board;
        Zobrist zobristKeys;
        Key hash;
        //std::unordered_map<Color, std::vector<unsigned int>> attacks;
        // std::vector<Move> legalMoves;

        std::unordered_map<Color, std::vector<bool>> castle;
        unsigned int enPassantTargetSquare; // en passant target square (64 if none)
        unsigned int halfMoveNumber;
        unsigned int fullMoveNumber;
        Color activeColor;

        std::unordered_map<Color, unsigned int> kingSquare;
        std::unordered_map<Color, std::unordered_map<PieceType, unsigned char>> capturedPieces;

    public:
        Game();
        Game(const std::string fen);

        int loadPosition(const std::string fen);
        void generate_hash();

        Result result(std::vector<Move> &legalMoves);
        bool isAttackedBy(unsigned int squareId, Color color);

        MoveSaveState saveState();
        void restoreState(MoveSaveState savedState);

        MoveSaveState doMove(Move &move);
        void undoMove(Move &move, MoveSaveState savedState);

        void update_hash(Move &move, MoveSaveState &savedState);
        // void hash_undo_move(Move &move, MoveSaveState &savedState);

        void updateIrreversibles(Move &move);
        void switchActiveColor();

        Key &getHash();
        std::vector<bool> getCastlingRights(Color color);
        unsigned int getEnPassantTargetSquare();
        unsigned int getKingSquare(Color color);
        std::unordered_map<PieceType, unsigned char> &getCapturedPieces(Color color);
        Color getActiveColor();
        std::string getPositionFEN();
        Piece &getPiece(unsigned int squareId);
        const std::string move2str(Move &move);
        Move str2move(const std::string &move);
};

} // namespace engine

#endif