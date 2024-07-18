#include "include/move.hpp"

namespace engine {

Move::Move() {
    this->originSquare = 64;
    this->targetSquare = 64;
    this->flags = M_NONE;
    this->capturedPiece = Piece();
}

/*Move::Move(const Move &move) : originSquare(move.originSquare),
                               targetSquare(move.targetSquare),
                               flags(move.flags),
                               capturedPiece(move.capturedPiece) {}*/
                               
Move::Move(unsigned int originSquare, 
           unsigned int targetSquare, 
           unsigned int flags,
           Piece capturedPiece) : originSquare(originSquare),
                                 targetSquare(targetSquare),
                                 flags(flags),
                                 capturedPiece(capturedPiece) {}

void Move::setOriginSquare(unsigned int originSquare) {
    this->originSquare = originSquare;
}

void Move::setTargetSquare(unsigned int targetSquare) {
    this->targetSquare = targetSquare;
}

void Move::setCapturedPiece(Piece capturedPiece) {
    this->capturedPiece = capturedPiece;
}

unsigned int Move::getOriginSquare() {
    return this->originSquare;
}

unsigned int Move::getTargetSquare() {
    return this->targetSquare;
}

Piece &Move::getCapturedPiece() {
    return this->capturedPiece;
}

unsigned int Move::getFlags() {
    return this->flags;
}

void Move::setFlags(unsigned int flags) {
    this->flags |= flags;
}

void Move::clearFlags(unsigned int flags) {
    this->flags &= ~flags;
}

bool Move::isEnPassant() {
    return this->flags & M_ENPASSANT;
}

bool Move::isCapture() {
    return this->flags & M_CAPTURE;
}

bool Move::isCheck() {
    return this->flags & M_CHECK;
}

bool Move::isMate() {
    return this->flags & M_MATE;
}

bool Move::isPromotion() {
    return this->flags & M_PROMOTION;
}

bool Move::isCastling() {
    return this->flags & M_CASTLE;
}

unsigned int Move::getCastlingSide() {
    return (this->flags & M_QUEENSIDE) >> 5;
}

PieceType Move::getPromotedPiece() {
    PieceType promotedPieceType = (PieceType)(PieceType::Bishop + ((this->flags & M_PQUEEN) >> 7));

    return promotedPieceType;
}

} // namespace engine