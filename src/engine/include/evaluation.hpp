#ifndef __EVALUATION_HPP__
#define __EVALUATION_HPP__

#include "engine.hpp"

namespace engine{

// values from https://www.chessprogramming.org/PeSTO%27s_Evaluation_Function
static std::unordered_map<engine::PieceType, std::pair<unsigned int, unsigned int>> pieceTypeValue = {
    {engine::PieceType::Pawn, {82, 94}},
    {engine::PieceType::Knight, {337, 281}},
    {engine::PieceType::Bishop, {365, 297}},
    {engine::PieceType::Rook, {477, 512}},
    {engine::PieceType::Queen, {1025, 936}},
    {engine::PieceType::King, {0, 0}},
    };

int evaluate(Game &game);

} // namespace engine

#endif