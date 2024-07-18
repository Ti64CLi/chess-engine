#ifndef __PIECE_HPP__
#define __PIECE_HPP__

#include <unordered_map>
#include <vector>

namespace engine {

enum PieceType {
    None = 0,
    Pawn,
    Bishop,
    Knight,
    Rook,
    Queen,
    King,
    Invalid,
    PTNumber = 6,
};

enum Color {
    Black,
    White,
};

struct Piece {
    PieceType pieceType;
    Color color;
};

Color getOppositeColor(Color color);

static std::unordered_map<PieceType, std::pair<std::vector<int>, bool>> pieceTypeOffsets = {
    {PieceType::Pawn, {{9, 11}, false}},
    {PieceType::Bishop, {{-11, -9, 9, 11}, true}},
    {PieceType::Knight, {{-21, -19, -12, -8, 8, 12, 19, 21}, false}},
    {PieceType::Rook, {{-10, -1, 1, 10}, true}},
    {PieceType::Queen, {{-11, -10, -9, -1, 1, 9, 10, 11}, true}},
    {PieceType::King, {{-11, -10, -9, -1, 1, 9, 10, 11}, false}},
};

} // namespace engine

#endif