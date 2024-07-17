#ifndef __ENGINE_HPP__
#define __ENGINE_HPP__

#include "zobrist.hpp"
#include <unordered_map>
#include <vector>
#include <string>


#define ENGINE_VERSION "0.1"

#define M_NONE      0
#define M_ENPASSANT (1 << 0)
#define M_CAPTURE   (1 << 1)
#define M_CHECK     (1 << 2)
#define M_CASTLE    (1 << 3)
#define M_KINGSIDE  (1 << 4)
#define M_QUEENSIDE (1 << 5)
#define M_PROMOTION (1 << 6)
#define M_PKNIGHT   (1 << 7)
#define M_PROOK     (2 << 7)
#define M_PQUEEN    (3 << 7)
#define M_MATE      (1 << 9)

#define XX 64

namespace engine {

const std::string startPosition = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

enum PieceType {
    None = 0,
    Pawn,
    Bishop,
    Knight,
    Rook,
    Queen,
    King,
    Invalid,
};

enum Color {
    Black,
    White,
};

Color getOppositeColor(Color color);

struct Piece {
    PieceType pieceType;
    Color color;
};

char pieceSymbol(Piece &piece);

class Move {
    private:
        unsigned int originSquare;
        unsigned int targetSquare;
        unsigned int flags;
        Piece capturedPiece;
    
    public:
        Move();
        Move(unsigned int originSquare, unsigned int targetSquare, unsigned int flags, Piece capturedPiece);

        void setOriginSquare(unsigned int originSquare);
        void setTargetSquare(unsigned int targetSquare);
        void setCapturedPiece(Piece capturedPiece);
        void setFlags(unsigned int flags);
        void clearFlags(unsigned int flags);
        unsigned int getOriginSquare();
        unsigned int getTargetSquare();
        Piece getCapturedPiece();
        unsigned int getFlags();
        bool isEnPassant();
        bool isCapture();
        bool isPromotion();
        bool isCastling();
        bool isCheck();
        bool isMate();
        unsigned int getCastlingSide();
        PieceType getPromotedPiece();
};

struct MoveSaveState {
    std::unordered_map<Color, std::vector<bool>> castle;
    unsigned int enPassantTargetSquare;
    unsigned int halfMoveNumber;
    unsigned int fullMoveNumber;
    std::unordered_map<Color, unsigned int> kingSquare;
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

        MoveSaveState saveState();
        void restoreState(MoveSaveState savedState);

        bool isAttackedBy(unsigned int squareId, Color color);
        void generatePseudoLegalMoves(std::vector<Move> &pseudoLegalMoves, unsigned int selectedCaseId);
        void generateAllPseudoLegalMoves(std::vector<Move> &pseudoLegalMoves);
        void generateLegalMoves(std::vector<Move> &legalMoves, unsigned int selectedCaseId, bool capturesOnly = false);
        void generateAllLegalMoves(std::vector<Move> &legalMoves, bool capturesOnly = false);
        int guessScore(Move &move);
        void orderMoves(std::vector<Move> &moves, std::vector<unsigned int> &orderedIndices);

        MoveSaveState doMove(Move &move);
        void undoMove(Move &move, MoveSaveState savedState);

        void update_hash(Move &move, MoveSaveState &savedState);
        // void hash_undo_move(Move &move, MoveSaveState &savedState);

        void updateIrreversibles(Move &move);
        void switchActiveColor();

        int evaluate();

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