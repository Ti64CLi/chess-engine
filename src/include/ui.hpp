#ifndef __UI_HPP__
#define __UI_HPP__

#include "../engine/include/engine.hpp"
#include <string>
#include <unordered_map>
#include <vector>

#define TOP_BORDER_HEIGHT       40
#define BOTTOM_BORDER_HEIGHT    40
#define LEFT_BORDER_WIDTH       0
#define RIGHT_BORDER_WIDTH      0

#define INVALID_EVENT   64
#define ERROR_EVENT     -1
#define PMOVE_EVENT     65

namespace ui {

int init(const std::string windowTitle, const unsigned int boardRectangleWidth, const float scale);
int manageEvents();
unsigned int handlePromotion(engine::Color pieceColor, unsigned int targetSquare);
int clear();
int renderSquare(unsigned int squareId);
int renderBoard();
int renderSelectedCase(const unsigned int selectedCaseId);
int renderPiece(char pieceSymbol, unsigned int targetSquare);
int renderPosition(const std::string &fen);
int renderMoves(std::vector<engine::Move> &moves);
int renderCapturedPieces(int side, std::unordered_map<engine::PieceType, unsigned char> &capturedPieces);
int show();
int close();

}

#endif