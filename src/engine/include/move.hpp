#ifndef __MOVE_HPP__
#define __MOVE_HPP__

#include "piece.hpp"

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

namespace engine {

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
        Piece &getCapturedPiece();
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

} // namespace engine

#endif