#include "include/zobrist.hpp"
#include "include/engine.hpp"
#include <random>

static std::unordered_map<engine::Color, std::vector<std::pair<unsigned int, unsigned int>>> castlingRookSquareIds = {
    {engine::Color::Black, {{63, 61}, {56, 59}}},
    {engine::Color::White, {{7, 5}, {0, 3}}},
};

namespace engine {

bool Zobrist::initialized = false;

void Zobrist::init() {
    if (Zobrist::initialized) {
        return;
    }

    std::mt19937_64 mt;

    for (unsigned int i = 0; i < 781; i++) {
        Zobrist::keys[i] = mt();
    }

    Zobrist::initialized = true;
}

Key Zobrist::generate_key(Game &game) {
    Key key = 0;

    for (unsigned int square = 0; square < 64; square++) {
        Piece &piece = game.getPiece(square);

        key ^= Zobrist::keys[piece.color * 384 + (piece.pieceType - PieceType::Pawn) * 64 + square];
    }

    if (game.getActiveColor() == Color::Black) {
        key ^= Zobrist::keys[768];
    }

    std::vector<bool> blackCastlingRights = game.getCastlingRights(Color::Black);
    std::vector<bool> whiteCastlingRights = game.getCastlingRights(Color::White);
    for (unsigned int castlingSide = 0; castlingSide < 2; castlingSide++) {
        if (blackCastlingRights[castlingSide]) {
            key ^= Zobrist::keys[769 + castlingSide];
        }
        if (whiteCastlingRights[castlingSide]) {
            key ^= Zobrist::keys[771 + castlingSide];
        }
    }

    unsigned int enPassantTargetSquare = game.getEnPassantTargetSquare();

    if (enPassantTargetSquare < 64) {
        key ^= Zobrist::keys[773 + (enPassantTargetSquare % 8)];
    }

    return key;
}

void Zobrist::key_do_move(Game &game, Key &key, Move &move, MoveSaveState &savedState) {
    Piece movedPiece = game.getPiece(move.getOriginSquare());
    PieceType promotedPieceType = movedPiece.pieceType;
    Piece capturedPiece = move.getCapturedPiece();
    unsigned int capturedPieceSquare = move.getTargetSquare();

    // Remove piece from origin square
    key ^= Zobrist::keys[movedPiece.color * 384 + (movedPiece.pieceType - PieceType::Pawn) * 64 + move.getOriginSquare()];

    if (move.isPromotion()) {
        promotedPieceType = move.getPromotedPiece();
    }

    key ^= Zobrist::keys[movedPiece.color * 384 + (promotedPieceType - PieceType::Pawn) * 64 + move.getTargetSquare()]; // put new piece on target square

    if (move.isCapture()) {
        if (capturedPieceSquare == savedState.enPassantTargetSquare) { // take care of en passant offset
            capturedPieceSquare += movedPiece.color == Color::Black ? 8 : -8;
        }

        key ^= Zobrist::keys[(1 - movedPiece.color) * 384 + (capturedPiece.pieceType - PieceType::Pawn) * 64 + capturedPieceSquare]; // remove captured piece
    }

    if (move.isCastling()) {
        std::pair<unsigned int, unsigned int> rookSquares = ::castlingRookSquareIds[movedPiece.color][move.getCastlingSide()];

        key ^= Zobrist::keys[movedPiece.color * 384 + (PieceType::Rook - PieceType::Pawn) * 64 + rookSquares.first]; // remove rook from castle origin square
        key ^= Zobrist::keys[movedPiece.color * 384 + (PieceType::Rook - PieceType::Pawn) * 64 + rookSquares.second]; // put rook on castle target square
    }

    // handle castling rights
    std::vector<bool> previousBlackCastlingRights = savedState.castle[Color::Black];
    std::vector<bool> previousWhiteCastlingRights = savedState.castle[Color::White];
    std::vector<bool> blackCastlingRights = game.getCastlingRights(Color::Black);
    std::vector<bool> whiteCastlingRights = game.getCastlingRights(Color::White);

    for (unsigned int castlingSide = 0; castlingSide < 2; castlingSide++) {
        if (blackCastlingRights[castlingSide] != previousBlackCastlingRights[castlingSide]) {
            key ^= Zobrist::keys[769 + castlingSide];
        }
        if (whiteCastlingRights[castlingSide] != previousWhiteCastlingRights[castlingSide]) {
            key ^= Zobrist::keys[771 + castlingSide];
        }
    }

    key ^= Zobrist::keys[768]; // change color side

    // handle en passant
    if (savedState.enPassantTargetSquare < 64) {
        key ^= Zobrist::keys[773 + (savedState.enPassantTargetSquare % 8)];
    }

    if (game.getEnPassantTargetSquare() < 64) {
        key ^= Zobrist::keys[773 + (game.getEnPassantTargetSquare() % 8)];
    }
}

void Zobrist::key_undo_move(Game &game, Key &key, Move &move, MoveSaveState &savedState) {
    Piece movedPiece = game.getPiece(move.getTargetSquare());
    Piece capturedPiece = move.getCapturedPiece();
    unsigned int capturedPieceSquare = move.getTargetSquare();

    key ^= Zobrist::keys[movedPiece.color * 384 + (movedPiece.pieceType - PieceType::Pawn) * 64 + move.getTargetSquare()]; // remove piece on target square

    if (move.isPromotion()) {
        movedPiece.pieceType = PieceType::Pawn;
    }

    key ^= Zobrist::keys[movedPiece.color * 384 + (movedPiece.pieceType - PieceType::Pawn) * 64 + move.getOriginSquare()]; // put piece back on origin square

    if (move.isCapture()) {
        if (move.getTargetSquare() == savedState.enPassantTargetSquare) {
            capturedPieceSquare = movedPiece.color == Color::Black ? 8 : -8;
        }

        key ^= Zobrist::keys[(1 - movedPiece.color) * 384 + (capturedPiece.pieceType - PieceType::Pawn) * 64 + capturedPieceSquare]; // put captured piece back
    }

    if (move.isCastling()) {
        std::pair<unsigned int, unsigned int> rookSquares = ::castlingRookSquareIds[movedPiece.color][move.getCastlingSide()];

        key ^= Zobrist::keys[movedPiece.color * 384 + (PieceType::Rook - PieceType::Pawn) * 64 + rookSquares.second]; // remove rook from castle target square
        key ^= Zobrist::keys[movedPiece.color * 384 + (PieceType::Rook - PieceType::Pawn) * 64 + rookSquares.first]; // put rook on castle origin square back
    }

    // handle castling rights
    std::vector<bool> previousBlackCastlingRights = savedState.castle[Color::Black];
    std::vector<bool> previousWhiteCastlingRights = savedState.castle[Color::White];
    std::vector<bool> blackCastlingRights = game.getCastlingRights(Color::Black);
    std::vector<bool> whiteCastlingRights = game.getCastlingRights(Color::White);

    for (unsigned int castlingSide = 0; castlingSide < 2; castlingSide++) {
        if (blackCastlingRights[castlingSide] != previousBlackCastlingRights[castlingSide]) {
            key ^= Zobrist::keys[769 + castlingSide];
        }
        if (whiteCastlingRights[castlingSide] != previousWhiteCastlingRights[castlingSide]) {
            key ^= Zobrist::keys[771 + castlingSide];
        }
    }

    key ^= Zobrist::keys[768]; // change color side

    // handle en passant
    if (savedState.enPassantTargetSquare < 64) {
        key ^= Zobrist::keys[773 + (savedState.enPassantTargetSquare % 8)];
    }

    if (game.getEnPassantTargetSquare() < 64) {
        key ^= Zobrist::keys[773 + (game.getEnPassantTargetSquare() % 8)];
    }
}

} // namespace engine
