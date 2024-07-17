#include "include/utils.hpp"
#include "include/engine.hpp"
#include <algorithm>
#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>

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

// values from https://www.chessprogramming.org/PeSTO%27s_Evaluation_Function
static std::unordered_map<engine::PieceType, std::pair<unsigned int, unsigned int>> pieceTypeValue = {
    {engine::PieceType::Pawn, {82, 94}},
    {engine::PieceType::Knight, {337, 281}},
    {engine::PieceType::Bishop, {365, 297}},
    {engine::PieceType::Rook, {477, 512}},
    {engine::PieceType::Queen, {1025, 936}},
    {engine::PieceType::King, {0, 0}},
};

namespace engine {

//const std::string startPosition = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

char fileSymbol(unsigned char file) {
    return 'a' + file;
}

char rankSymbol(unsigned char rank) {
    return '0' + rank;
}

char colorSymbol(const Color &color) {
    return (color == Color::Black) ? 'b' : 'w';
}

char pieceTypeSymbol(const PieceType &pieceType) {
    switch (pieceType) {
        case PieceType::Pawn:
            return 'P';
        case PieceType::Bishop:
            return 'B';
        case PieceType::Knight:
            return 'N';
        case PieceType::Rook:
            return 'R';
        case PieceType::Queen:
            return 'Q';
        case PieceType::King:
        default:
            return 'K';
    }
}

char pieceSymbol(Piece &piece) {
    char symbol = pieceTypeSymbol(piece.pieceType);

    if (piece.color == Color::Black) {
        symbol = utils::toLowerCase(symbol);
    }

    return symbol;
}

char promotionSymbol(PieceType promotedPiece) {
    switch (promotedPiece) {
        case PieceType::Bishop:
            return 'B';
        case PieceType::Knight:
            return 'N';
        case PieceType::Rook:
            return 'R';
        case PieceType::Queen:
        default:
            return 'Q';
    }
}

Color getOppositeColor(Color color) {
    switch (color) {
        case Color::Black:
            return Color::White;
        case Color::White:
        default:
            return Color::Black;
    }
}

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

Piece Move::getCapturedPiece() {
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

Game::Game() {
    this->zobristKeys.init();
    this->loadPosition(startPosition);
}
Game::Game(const std::string fen) {
    this->zobristKeys.init();
    this->loadPosition(fen);
}

int Game::loadPosition(const std::string fen) {
    std::unordered_map<char, PieceType> pieceTypeFromSymbol = {
        {'p', PieceType::Pawn},
        {'b', PieceType::Bishop},
        {'n', PieceType::Knight},
        {'r', PieceType::Rook},
        {'q', PieceType::Queen},
        {'k', PieceType::King},
    };

    this->capturedPieces = {
        {Color::Black, {
            {PieceType::Pawn, 8},
            {PieceType::Bishop, 2},
            {PieceType::Knight, 2},
            {PieceType::Rook, 2},
            {PieceType::Queen, 1},
            {PieceType::King, 1},
        }},
        {Color::White, {
            {PieceType::Pawn, 8},
            {PieceType::Bishop, 2},
            {PieceType::Knight, 2},
            {PieceType::Rook, 2},
            {PieceType::Queen, 1},
            {PieceType::King, 1},
        }},
    };

    this->board.clear();
    for (size_t i = 0; i < 64; i++) {
        this->board.push_back({PieceType::None, Color::Black});
    }
    
    std::vector<std::string> splitFEN = utils::split(fen);

    if (splitFEN.size() != 6) {
        std::cerr << "[ERROR] Invalid FEN : " << fen << std::endl;
        
        return -1;
    }

    size_t file = 0, rank = 7;
    for (auto c : splitFEN[0]) {
        if (c == '/') {
            rank--;
            file = 0;
        } else {
            if ('0' <= c && c <= '9') {
                file += c - '0';
            } else {
                Color color;

                if ('A' < c && c < 'Z') {
                    color = Color::White;
                } else if ('a' < c && c < 'z') {
                    color = Color::Black;
                } else {
                    std::cerr << "[ERROR] Invalid FEN '" << fen << "' : Wrong piece descriptor" << std::endl;

                    return -1;
                }

                if (c == 'k') {
                    this->kingSquare[Color::Black] = ID(file, rank);
                } else if (c == 'K') {
                    this->kingSquare[Color::White] = ID(file, rank);
                }

                if (this->capturedPieces[getOppositeColor(color)][pieceTypeFromSymbol[utils::toLowerCase(c)]] == 0 && utils::toLowerCase(c) != 'p') {
                    this->capturedPieces[getOppositeColor(color)][PieceType::Pawn]--;
                } else {
                    this->capturedPieces[getOppositeColor(color)][pieceTypeFromSymbol[utils::toLowerCase(c)]]--;
                }

                this->board[ID(file, rank)] = {pieceTypeFromSymbol[utils::toLowerCase(c)], color};
                file++;
            }
        }
    }

    if (this->capturedPieces[Color::Black][PieceType::Pawn] > 8 || this->capturedPieces[Color::White][PieceType::Pawn] > 8) {
        std::cerr << "[ERROR] Invalid FEN '" << fen << "' : Too many pieces" << std::endl;

        return -1;
    }

    if (splitFEN[1] == "w") {
        this->activeColor = Color::White;
    } else if (splitFEN[1] == "b") {
        this->activeColor = Color::Black;
    } else {
        std::cerr << "[ERROR] Invalid FEN '" << fen << "' : Malformed active color" << std::endl;

        return -1;
    }

    this->castle = {
        {Color::Black, {false, false}},
        {Color::White, {false, false}},
    };

    if (splitFEN[2] != "-") {
        for (char c : splitFEN[2]) {
            switch (c) {
                case 'K': {
                    this->castle[Color::White][0] = true;
                    break;
                }
                case 'Q': {
                    this->castle[Color::White][1] = true;
                    break;
                }
                case 'k': {
                    this->castle[Color::Black][0] = true;
                    break;
                }
                case 'q': {
                    this->castle[Color::Black][1] = true;
                    break;
                }
                default: {
                    std::cerr << "[ERROR] Invalid FEN '" << fen << "' : Malformed castling availability" << std::endl;

                    return -1;
                }
            }
        }
    }

    this->enPassantTargetSquare = 64;

    if (splitFEN[3] != "-") {
        int enPassantTargetSquare = utils::idFromCaseName(splitFEN[3]);

        if (enPassantTargetSquare == -1) {
            std::cerr << "[ERROR] Invalid FEN '" << fen << "' : Malformed en passant target square" << std::endl;

            return -1;
        }

        this->enPassantTargetSquare = enPassantTargetSquare;
    }

    this->halfMoveNumber = std::stoi(splitFEN[4]);
    this->fullMoveNumber = std::stoi(splitFEN[5]); // TODO : should handle potential exception

    this->generate_hash();

    return 0;
}

MoveSaveState Game::saveState() {
    return {
        {
            {Color::Black, this->castle[Color::Black]},
            {Color::White, this->castle[Color::White]}
        },
        this->enPassantTargetSquare,
        this->halfMoveNumber,
        this->fullMoveNumber,
        {
            {Color::Black, this->kingSquare[Color::Black]},
            {Color::White, this->kingSquare[Color::White]}
        },
    };
}

void Game::restoreState(MoveSaveState savedState) {
    this->castle[Color::Black] = savedState.castle[Color::Black];
    this->castle[Color::White] = savedState.castle[Color::White];
    this->enPassantTargetSquare = savedState.enPassantTargetSquare;
    this->halfMoveNumber = savedState.halfMoveNumber;
    this->fullMoveNumber = savedState.fullMoveNumber;
    this->kingSquare[Color::Black] = savedState.kingSquare[Color::Black];
    this->kingSquare[Color::White] = savedState.kingSquare[Color::White];
}

void Game::generatePseudoLegalMoves(std::vector<Move> &pseudoLegalMoves, unsigned int selectedCaseId) {
    static std::unordered_map<PieceType, std::pair<std::vector<int>, bool>> pieceTypeOffsets = {
        {PieceType::Bishop, {{-11, -9, 9, 11}, true}},
        {PieceType::Knight, {{-21, -19, -12, -8, 8, 12, 19, 21}, false}},
        {PieceType::Rook, {{-10, -1, 1, 10}, true}},
        {PieceType::Queen, {{-11, -10, -9, -1, 1, 9, 10, 11}, true}},
        {PieceType::King, {{-11, -10, -9, -1, 1, 9, 10, 11}, false}},
    };

    static std::vector<std::vector<int>> castlingOffsets = {
        {1, 2},
        {-1, -2, -3},
    };

    if (this->board[selectedCaseId].color != this->activeColor || this->board[selectedCaseId].pieceType == PieceType::None) {
        return;
    }

    Piece &selectedPiece = this->board[selectedCaseId];
    unsigned int activeColorLastRank = (this->activeColor == Color::Black) ? 0 : 7;

    switch (selectedPiece.pieceType) {
        case PieceType::Pawn: {
            int side = (this->activeColor == Color::White) ? 1 : -1;
            unsigned int targetSquare9 = ::mailbox10x12[::mailbox8x8[selectedCaseId] + (9 * side)];
            unsigned int targetSquare10 = ::mailbox10x12[::mailbox8x8[selectedCaseId] + (10 * side)];
            unsigned int targetSquare20 = ::mailbox10x12[::mailbox8x8[selectedCaseId] + (20 * side)];
            unsigned int targetSquare11 = ::mailbox10x12[::mailbox8x8[selectedCaseId] + (11 * side)];

            if (targetSquare9 != XX && // valid id
                (this->enPassantTargetSquare == targetSquare9 || // en passant possible
                (this->board[targetSquare9].pieceType != PieceType::None && 
                this->board[targetSquare9].color != selectedPiece.color))) { // or opponent piece
                unsigned int flags = M_CAPTURE;
                Piece capturedPiece = this->board[targetSquare9];

                if (this->enPassantTargetSquare == targetSquare9) { // en passant
                    capturedPiece.pieceType = PieceType::Pawn;
                    capturedPiece.color = getOppositeColor(this->activeColor);
                }
                
                if (RANK(targetSquare9) == activeColorLastRank) {
                    flags |= M_PROMOTION;

                    for (unsigned int promotionFlag = 0; promotionFlag < 4; promotionFlag++) {
                        pseudoLegalMoves.push_back(Move(selectedCaseId, targetSquare9, flags | (promotionFlag << 7), capturedPiece));
                    }
                } else {
                    pseudoLegalMoves.push_back(Move(selectedCaseId, targetSquare9, flags, capturedPiece));
                }
            }

            if (targetSquare11 != XX && // valid id
                (this->enPassantTargetSquare == targetSquare11 || // en passant possible
                (this->board[targetSquare11].pieceType != PieceType::None &&
                this->board[targetSquare11].color != selectedPiece.color))) { // or opponent piece
                unsigned int flags = M_CAPTURE;
                Piece capturedPiece = this->board[targetSquare11];

                if (this->enPassantTargetSquare == targetSquare11) { // en passant
                    capturedPiece.pieceType = PieceType::Pawn;
                    capturedPiece.color = getOppositeColor(this->activeColor);
                }

                if (RANK(targetSquare11) == activeColorLastRank) {
                    flags |= M_PROMOTION;

                    for (unsigned int promotionFlag = 0; promotionFlag < 4; promotionFlag++) {
                        pseudoLegalMoves.push_back(Move(selectedCaseId, targetSquare11, flags | (promotionFlag << 7), capturedPiece));
                    }
                } else {
                    pseudoLegalMoves.push_back(Move(selectedCaseId, targetSquare11, flags, capturedPiece));
                }
            }

            if (targetSquare10 != XX && // valid id
                this->board[targetSquare10].pieceType == PieceType::None) { // no piece on target square
                if (RANK(targetSquare10) == activeColorLastRank) {
                    unsigned int flags = M_PROMOTION;
                
                    for (unsigned int promotionFlag = 0; promotionFlag < 4; promotionFlag++) {
                        pseudoLegalMoves.push_back(Move(selectedCaseId, targetSquare10, flags | (promotionFlag << 7), {PieceType::None, Color::Black}));
                    }
                } else {
                    pseudoLegalMoves.push_back(Move(selectedCaseId, targetSquare10, M_NONE, {PieceType::None, Color::Black}));
                }

                if (targetSquare20 != XX && // valid id
                    RANK(selectedCaseId) == ((unsigned int)(7 + side) % 7) && // second rank for each side
                    this->board[targetSquare20].pieceType == PieceType::None) { // target square is empty
                    pseudoLegalMoves.push_back(Move(selectedCaseId, targetSquare20, M_ENPASSANT, {PieceType::None, Color::Black}));
                }
            }

            break;
        }

        case PieceType::Bishop:
        case PieceType::Knight:
        case PieceType::Rook:
        case PieceType::Queen:
        case PieceType::King: {
            std::pair<std::vector<int>, bool> &currentPieceAllowedMoves = pieceTypeOffsets[selectedPiece.pieceType];
            std::vector<int> &currentPieceOffsets = currentPieceAllowedMoves.first;

            for (const int &offset : currentPieceOffsets) { // check all directions
                for (unsigned int targetSquare = selectedCaseId;;) { // start from selected square
                    targetSquare = ::mailbox10x12[::mailbox8x8[targetSquare] + offset]; // get next target square

                    if (targetSquare == XX) { // outside of the board
                        break;
                    }

                    Piece &targetSquarePiece = this->board[targetSquare];

                    if (targetSquarePiece.pieceType != PieceType::None) { // blocked by a piece
                        if (targetSquarePiece.color != this->activeColor) { // capture opponent piece
                            pseudoLegalMoves.push_back(Move(selectedCaseId, targetSquare, M_CAPTURE, targetSquarePiece));
                        }

                        break; // can't continue in this direction
                    }

                    pseudoLegalMoves.push_back(Move(selectedCaseId, targetSquare, M_NONE, {PieceType::None, Color::Black}));

                    if (!currentPieceAllowedMoves.second) { // can't slide
                        break;
                    }
                }
            }

            break;
        }

        default:
            break;
    }

    // check for castling
    if (selectedPiece.pieceType == PieceType::King) {
        for (size_t castlingSide = 0; castlingSide < 2; castlingSide++) { // check each side
            bool possible = this->castle[this->activeColor][castlingSide] && !this->isAttackedBy(selectedCaseId, getOppositeColor(this->activeColor));

            if (possible) {
                for (const auto &offset : castlingOffsets[castlingSide]) {
                    if (this->board[selectedCaseId + offset].pieceType != PieceType::None || (this->isAttackedBy(selectedCaseId + offset, getOppositeColor(this->activeColor)) && offset != -3)) {
                        possible = false;

                        break;
                    }
                }

                if (possible) {
                    pseudoLegalMoves.push_back(Move(selectedCaseId, selectedCaseId + castlingOffsets[castlingSide][0] * 2, M_CASTLE | (M_KINGSIDE << castlingSide), {PieceType::None, Color::Black}));
                }
            }
        }
    }
}

void Game::generateAllPseudoLegalMoves(std::vector<Move> &pseudoLegalMoves) {
    for (unsigned int caseId = 0; caseId < 64; caseId++) {
        if (this->board[caseId].color != this->activeColor || this->board[caseId].pieceType == PieceType::None) {
            continue;
        }

        this->generatePseudoLegalMoves(pseudoLegalMoves, caseId);
    }
}

bool Game::isAttackedBy(unsigned int squareId, Color color) {
    int pawnSide = (color == Color::Black) ? 1 : -1;

    static std::vector<std::pair<PieceType, std::pair<std::vector<int>, bool>>> pieceTypeOffsets = {
        {PieceType::Pawn, {{9, 11}, false}},
        {PieceType::Bishop, {{-11, -9, 9, 11}, true}},
        {PieceType::Knight, {{-21, -19, -12, -8, 8, 12, 19, 21}, false}},
        {PieceType::Rook, {{-10, -1, 1, 10}, true}},
        {PieceType::Queen, {{-11, -10, -9, -1, 1, 9, 10, 11}, true}},
        {PieceType::King, {{-11, -10, -9, -1, 1, 9, 10, 11}, false}},
    };

    bool attacked = false;

    for (const auto &currentPieceTypeOffsets : pieceTypeOffsets) {
        if (attacked) {
            break;
        }

        const auto &allowedOffsets = currentPieceTypeOffsets.second;

        for (const auto &offset : allowedOffsets.first) { // test each direction
            if (attacked) {
                break;
            }

            int currentOffset = offset;

            if (currentPieceTypeOffsets.first == PieceType::Pawn) {
                currentOffset *= pawnSide;
            }

            for (unsigned int testedSquare = squareId;;) { // go in the current direction, starting from the selected square
                testedSquare = ::mailbox10x12[::mailbox8x8[testedSquare] + currentOffset]; // next square in the current direction

                if (testedSquare == XX) { // got out of the board
                    break;
                }

                Piece &currentTestedPiece = this->board[testedSquare];

                if (currentTestedPiece.pieceType == PieceType::None && allowedOffsets.second) { // can continue in this direction only if sliding piece
                    continue;
                }

                if (currentTestedPiece.pieceType == currentPieceTypeOffsets.first && currentTestedPiece.color == color) { // opposing piece
                    attacked = true;
                }

                break; // break anyway since there is a piece
            }
        }
    }

    return attacked;
}

void Game::generateLegalMoves(std::vector<Move> &legalMoves, unsigned int selectedCaseId, bool capturesOnly) {
    if (this->board[selectedCaseId].color != this->activeColor || this->board[selectedCaseId].pieceType == PieceType::None) {
        return;
    }

    std::vector<Move> pseudoLegalMoves;
    unsigned int kingSquare = this->getKingSquare(this->activeColor);

    this->generatePseudoLegalMoves(pseudoLegalMoves, selectedCaseId);

    for (Move move : pseudoLegalMoves) {
        if (capturesOnly && !move.isCapture()) {
            continue;
        }

        unsigned int currentKingSquare = kingSquare;

        if (move.getOriginSquare() == kingSquare) {
            currentKingSquare = move.getTargetSquare();
        }

        MoveSaveState savedState = this->doMove(move);

        if (!this->isAttackedBy(currentKingSquare, this->activeColor)) { // left our king in check ?
            legalMoves.push_back(move);
        }

        this->undoMove(move, savedState);
    }
}

void Game::generateAllLegalMoves(std::vector<Move> &legalMoves, bool capturesOnly) {
    for (unsigned int selectedCaseId = 0; selectedCaseId < 64; selectedCaseId++) {
        if (this->board[selectedCaseId].color != this->activeColor || this->board[selectedCaseId].pieceType == PieceType::None) {
            continue;
        }

        this->generateLegalMoves(legalMoves, selectedCaseId, capturesOnly);
    }
}

int Game::guessScore(Move &move) {
    int guessedScore = 0;

    PieceType movedPiece = this->board[move.getOriginSquare()].pieceType;
    PieceType capturedPiece = move.getCapturedPiece().pieceType;

    if (move.isCapture()) {
        guessedScore += 10 * ::pieceTypeValue[capturedPiece].first - ::pieceTypeValue[movedPiece].first;
    }

    if (move.isPromotion()) {
        guessedScore += ::pieceTypeValue[move.getPromotedPiece()].first;
    }

    if (this->isAttackedBy(move.getTargetSquare(), getOppositeColor(this->activeColor))) {
        guessedScore -= ::pieceTypeValue[movedPiece].first;
    }

    return guessedScore;
}

void Game::orderMoves(std::vector<Move> &moves, std::vector<unsigned int> &orderedIndices) {
    std::vector<Move> orderedMoves;
    std::vector<int> guessedScores;

    for (unsigned int i = 0; i < moves.size(); i++) {
        orderedIndices.push_back(i);
        guessedScores.push_back(this->guessScore(moves[i]));
    }

    std::sort(orderedIndices.begin(), orderedIndices.end(), [&](unsigned int &i, unsigned int &j) {
        return guessedScores[i] > guessedScores[j];
    });
}

MoveSaveState Game::doMove(Move &move) {
    MoveSaveState savedState = this->saveState(); // save current state

    Piece selectedPiece = this->board[move.getOriginSquare()];
    Piece targetSquarePiece = move.getCapturedPiece();

    this->board[move.getTargetSquare()] = selectedPiece; // move piece to target square
    this->board[move.getOriginSquare()] = {PieceType::None, Color::Black}; // remove piece from original square

    if (selectedPiece.pieceType == PieceType::King) { // can't castle anymore if king moves
        this->castle[this->activeColor] = {false, false};
        this->kingSquare[this->activeColor] = move.getTargetSquare();
    }
    if (selectedPiece.pieceType == PieceType::Rook) { // can't castle on the side of the rook
        for (size_t i = 0; i < 2; i++) {
            if (move.getOriginSquare() == ::castlingRookSquareIds[selectedPiece.color][i].first) {
                this->castle[selectedPiece.color][i] = false;
            }
        }
    }

    // handle flags
    if (move.isCapture()) {
        if (this->enPassantTargetSquare == move.getTargetSquare()) {
            unsigned int offset = (this->activeColor == Color::Black) ? 8 : -8;
            this->board[this->enPassantTargetSquare + offset] = {PieceType::None, Color::Black}; 
        }

        if (targetSquarePiece.pieceType == PieceType::Rook) {
            for (size_t i = 0; i < 2; i++) {
                if (move.getTargetSquare() == ::castlingRookSquareIds[targetSquarePiece.color][i].first) {
                    this->castle[targetSquarePiece.color][i] = false;
                }
            }
        }

        this->capturedPieces[this->activeColor][targetSquarePiece.pieceType]++;
    }

    if (move.isCastling()) {
        unsigned int castlingSide = move.getCastlingSide();
        unsigned int castlingOriginSquare = ::castlingRookSquareIds[this->activeColor][castlingSide].first;

        this->board[::castlingRookSquareIds[this->activeColor][castlingSide].second] = this->board[castlingOriginSquare]; // move rook
        this->board[castlingOriginSquare] = {PieceType::None, Color::Black}; // empty old rook position
        this->castle[this->activeColor] = {false, false};
    }
    
    if (move.isPromotion()) {
        PieceType promotedPieceType = move.getPromotedPiece();

        this->board[move.getTargetSquare()] = {promotedPieceType, this->activeColor};
    }

    if (move.isEnPassant()) {
        unsigned int offset = (this->activeColor == Color::Black) ? 8 : -8;
        this->enPassantTargetSquare = move.getTargetSquare() + offset;
    } else {
        this->enPassantTargetSquare = 64;
    }

    // handle clocks
    this->halfMoveNumber++;

    if (this->activeColor == Color::White) {
        this->fullMoveNumber++;
    }

    if (move.isCapture() || this->board[move.getTargetSquare()].pieceType == PieceType::Pawn) {
        this->halfMoveNumber = 0;
    }

    this->switchActiveColor();

    this->update_hash(move, savedState);

    return savedState;
}

void Game::undoMove(Move &move, MoveSaveState savedState) {
    this->update_hash(move, savedState);

    this->switchActiveColor();
    this->restoreState(savedState);

    Piece &movedPiece = this->board[move.getTargetSquare()];

    if (move.isPromotion()) {
        movedPiece.pieceType = PieceType::Pawn;
    }

    this->board[move.getOriginSquare()] = movedPiece;
    this->board[move.getTargetSquare()] = {PieceType::None, Color::Black};

    if (move.isCapture()) {
        unsigned int capturedPieceSquare = move.getTargetSquare();

        if (this->enPassantTargetSquare == move.getTargetSquare()) {
            unsigned int offset = (this->activeColor == Color::Black) ? 8 : -8;
            capturedPieceSquare = this->enPassantTargetSquare + offset;
        }

        this->board[capturedPieceSquare] = move.getCapturedPiece();
        
        this->capturedPieces[this->activeColor][move.getCapturedPiece().pieceType]--;
    }

    if (move.isCastling()) {
        unsigned int castlingSide = move.getCastlingSide();
        unsigned int castlingOriginSquare = ::castlingRookSquareIds[this->activeColor][castlingSide].first;

        this->board[castlingOriginSquare] = this->board[::castlingRookSquareIds[this->activeColor][castlingSide].second]; // move rook
        this->board[::castlingRookSquareIds[this->activeColor][castlingSide].second] = {PieceType::None, Color::Black}; // empty old rook position
    }
}

void Game::generate_hash() {
    this->hash = 0;

    for (unsigned int square = 0; square < 64; square++) {
        Piece &piece = this->getPiece(square);
        if (piece.pieceType != PieceType::None) {
            this->hash^= this->zobristKeys.getKey(piece.color * 384 + (piece.pieceType - PieceType::Pawn) * 64 + square);
        }
    }

    if (this->getActiveColor() == Color::Black) {
        this->hash^= this->zobristKeys.getKey(768);
    }

    std::vector<bool> blackCastlingRights = this->getCastlingRights(Color::Black);
    std::vector<bool> whiteCastlingRights = this->getCastlingRights(Color::White);
    for (unsigned int castlingSide = 0; castlingSide < 2; castlingSide++) {
        if (blackCastlingRights[castlingSide]) {
            this->hash^= this->zobristKeys.getKey(769 + castlingSide);
        }
        if (whiteCastlingRights[castlingSide]) {
            this->hash^= this->zobristKeys.getKey(771 + castlingSide);
        }
    }

    unsigned int enPassantTargetSquare = this->getEnPassantTargetSquare();

    if (enPassantTargetSquare < 64) {
        this->hash^= this->zobristKeys.getKey(773 + (enPassantTargetSquare % 8));
    }
}

void Game::update_hash(Move &move, MoveSaveState &savedState) {
    Piece movedPiece = this->getPiece(move.getTargetSquare());
    PieceType promotedPieceType = movedPiece.pieceType;
    Piece capturedPiece = move.getCapturedPiece();
    unsigned int capturedPieceSquare = move.getTargetSquare();

    if (move.isPromotion()) {
        movedPiece.pieceType = PieceType::Pawn;
    }

    this->hash ^= this->zobristKeys.getKey(movedPiece.color * 384 + (movedPiece.pieceType - PieceType::Pawn) * 64 + move.getOriginSquare()); // Remove piece from origin square
    this->hash ^= this->zobristKeys.getKey(movedPiece.color * 384 + (promotedPieceType - PieceType::Pawn) * 64 + move.getTargetSquare()); // put new piece on target square

    if (move.isCapture()) {
        if (capturedPieceSquare == savedState.enPassantTargetSquare) { // take care of en passant offset
            capturedPieceSquare += movedPiece.color == Color::Black ? 8 : -8;
        }

        this->hash ^= this->zobristKeys.getKey(capturedPiece.color * 384 + (capturedPiece.pieceType - PieceType::Pawn) * 64 + capturedPieceSquare); // remove captured piece
    }

    if (move.isCastling()) {
        std::pair<unsigned int, unsigned int> rookSquares = ::castlingRookSquareIds[movedPiece.color][move.getCastlingSide()];

        this->hash ^= this->zobristKeys.getKey(movedPiece.color * 384 + (PieceType::Rook - PieceType::Pawn) * 64 + rookSquares.first); // remove rook from castle origin square
        this->hash ^= this->zobristKeys.getKey(movedPiece.color * 384 + (PieceType::Rook - PieceType::Pawn) * 64 + rookSquares.second); // put rook on castle target square
    }

    // handle castling rights
    std::vector<bool> previousBlackCastlingRights = savedState.castle[Color::Black];
    std::vector<bool> previousWhiteCastlingRights = savedState.castle[Color::White];
    std::vector<bool> blackCastlingRights = this->getCastlingRights(Color::Black);
    std::vector<bool> whiteCastlingRights = this->getCastlingRights(Color::White);

    for (unsigned int castlingSide = 0; castlingSide < 2; castlingSide++) {
        if (blackCastlingRights[castlingSide] != previousBlackCastlingRights[castlingSide]) {
            this->hash ^= this->zobristKeys.getKey(769 + castlingSide);
        }
        if (whiteCastlingRights[castlingSide] != previousWhiteCastlingRights[castlingSide]) {
            this->hash ^= this->zobristKeys.getKey(771 + castlingSide);
        }
    }

    this->hash ^= this->zobristKeys.getKey(768); // change color side

    // handle en passant
    if (savedState.enPassantTargetSquare < 64) {
        this->hash ^= this->zobristKeys.getKey(773 + (savedState.enPassantTargetSquare % 8));
    }

    if (this->getEnPassantTargetSquare() < 64) {
        this->hash ^= this->zobristKeys.getKey(773 + (this->getEnPassantTargetSquare() % 8));
    }
}

// not needed since XOR is it's own inverse
/*void Game::hash_undo_move(Move &move, MoveSaveState &savedState) {
    Piece movedPiece = this->getPiece(move.getTargetSquare());
    Piece capturedPiece = move.getCapturedPiece();
    unsigned int capturedPieceSquare = move.getTargetSquare();

    this->hash ^= this->zobristKeys.getKey(movedPiece.color * 384 + (movedPiece.pieceType - PieceType::Pawn) * 64 + move.getTargetSquare()); // remove piece on target square

    if (move.isPromotion()) {
        movedPiece.pieceType = PieceType::Pawn;
    }

    this->hash ^= this->zobristKeys.getKey(movedPiece.color * 384 + (movedPiece.pieceType - PieceType::Pawn) * 64 + move.getOriginSquare()); // put piece back on origin square

    if (move.isCapture()) {
        if (move.getTargetSquare() == savedState.enPassantTargetSquare) {
            capturedPieceSquare = movedPiece.color == Color::Black ? 8 : -8;
        }

        this->hash ^= this->zobristKeys.getKey(capturedPiece.color * 384 + (capturedPiece.pieceType - PieceType::Pawn) * 64 + capturedPieceSquare); // put captured piece back
    }

    if (move.isCastling()) {
        std::pair<unsigned int, unsigned int> rookSquares = ::castlingRookSquareIds[movedPiece.color][move.getCastlingSide()];

        this->hash ^= this->zobristKeys.getKey(movedPiece.color * 384 + (PieceType::Rook - PieceType::Pawn) * 64 + rookSquares.second); // remove rook from castle target square
        this->hash ^= this->zobristKeys.getKey(movedPiece.color * 384 + (PieceType::Rook - PieceType::Pawn) * 64 + rookSquares.first); // put rook on castle origin square back
    }

    // handle castling rights
    std::vector<bool> previousBlackCastlingRights = savedState.castle[Color::Black];
    std::vector<bool> previousWhiteCastlingRights = savedState.castle[Color::White];
    std::vector<bool> blackCastlingRights = this->getCastlingRights(Color::Black);
    std::vector<bool> whiteCastlingRights = this->getCastlingRights(Color::White);

    for (unsigned int castlingSide = 0; castlingSide < 2; castlingSide++) {
        if (blackCastlingRights[castlingSide] != previousBlackCastlingRights[castlingSide]) {
            this->hash ^= this->zobristKeys.getKey(769 + castlingSide);
        }
        if (whiteCastlingRights[castlingSide] != previousWhiteCastlingRights[castlingSide]) {
            this->hash ^= this->zobristKeys.getKey(771 + castlingSide);
        }
    }

    this->hash ^= this->zobristKeys.getKey(768); // change color side

    // handle en passant
    if (savedState.enPassantTargetSquare < 64) {
        this->hash ^= this->zobristKeys.getKey(773 + (savedState.enPassantTargetSquare % 8));
    }

    if (this->getEnPassantTargetSquare() < 64) {
        this->hash ^= this->zobristKeys.getKey(773 + (this->getEnPassantTargetSquare() % 8));
    }
}*/

void Game::switchActiveColor() {
    if (this->activeColor == Color::Black) { // next player
        this->activeColor = Color::White;
    } else {
        this->activeColor = Color::Black;
    }
}

bool Game::isEndGame() {
    bool endGame = false;

    return endGame;
}

// todo later
int Game::evaluate() {
    // from https://www.chessprogramming.org/PeSTO%27s_Evaluation_Function
    static const int pawnMiddleGame[] = {
        0,   0,   0,   0,   0,   0,  0,   0,
        98, 134,  61,  95,  68, 126, 34, -11,
        -6,   7,  26,  31,  65,  56, 25, -20,
        -14,  13,   6,  21,  23,  12, 17, -23,
        -27,  -2,  -5,  12,  17,   6, 10, -25,
        -26,  -4,  -4, -10,   3,   3, 33, -12,
        -35,  -1, -20, -23, -15,  24, 38, -22,
        0,   0,   0,   0,   0,   0,  0,   0,
    };

    static const int pawnEndGame[] = {
        0,   0,   0,   0,   0,   0,   0,   0,
        178, 173, 158, 134, 147, 132, 165, 187,
        94, 100,  85,  67,  56,  53,  82,  84,
        32,  24,  13,   5,  -2,   4,  17,  17,
        13,   9,  -3,  -7,  -7,  -8,   3,  -1,
        4,   7,  -6,   1,   0,  -5,  -1,  -8,
        13,   8,   8,  10,  13,   0,   2,  -7,
        0,   0,   0,   0,   0,   0,   0,   0,
    };

    static const int knightMiddleGame[] = {
        -167, -89, -34, -49,  61, -97, -15, -107,
        -73, -41,  72,  36,  23,  62,   7,  -17,
        -47,  60,  37,  65,  84, 129,  73,   44,
        -9,  17,  19,  53,  37,  69,  18,   22,
        -13,   4,  16,  13,  28,  19,  21,   -8,
        -23,  -9,  12,  10,  19,  17,  25,  -16,
        -29, -53, -12,  -3,  -1,  18, -14,  -19,
        -105, -21, -58, -33, -17, -28, -19,  -23,
    };

    static const int knightEndGame[] = {
        -58, -38, -13, -28, -31, -27, -63, -99,
        -25,  -8, -25,  -2,  -9, -25, -24, -52,
        -24, -20,  10,   9,  -1,  -9, -19, -41,
        -17,   3,  22,  22,  22,  11,   8, -18,
        -18,  -6,  16,  25,  16,  17,   4, -18,
        -23,  -3,  -1,  15,  10,  -3, -20, -22,
        -42, -20, -10,  -5,  -2, -20, -23, -44,
        -29, -51, -23, -15, -22, -18, -50, -64,
    };

    static const int bishopMiddleGame[] = {
        -29,   4, -82, -37, -25, -42,   7,  -8,
        -26,  16, -18, -13,  30,  59,  18, -47,
        -16,  37,  43,  40,  35,  50,  37,  -2,
        -4,   5,  19,  50,  37,  37,   7,  -2,
        -6,  13,  13,  26,  34,  12,  10,   4,
        0,  15,  15,  15,  14,  27,  18,  10,
        4,  15,  16,   0,   7,  21,  33,   1,
        -33,  -3, -14, -21, -13, -12, -39, -21,
    };

    static const int bishopEndGame[] = {
        -14, -21, -11,  -8, -7,  -9, -17, -24,
        -8,  -4,   7, -12, -3, -13,  -4, -14,
        2,  -8,   0,  -1, -2,   6,   0,   4,
        -3,   9,  12,   9, 14,  10,   3,   2,
        -6,   3,  13,  19,  7,  10,  -3,  -9,
        -12,  -3,   8,  10, 13,   3,  -7, -15,
        -14, -18,  -7,  -1,  4,  -9, -15, -27,
        -23,  -9, -23,  -5, -9, -16,  -5, -17,
    };

    static const int rookMiddleGame[] = {
        32,  42,  32,  51, 63,  9,  31,  43,
        27,  32,  58,  62, 80, 67,  26,  44,
        -5,  19,  26,  36, 17, 45,  61,  16,
        -24, -11,   7,  26, 24, 35,  -8, -20,
        -36, -26, -12,  -1,  9, -7,   6, -23,
        -45, -25, -16, -17,  3,  0,  -5, -33,
        -44, -16, -20,  -9, -1, 11,  -6, -71,
        -19, -13,   1,  17, 16,  7, -37, -26,
    };

    static const int rookEndGame[] = {
        13, 10, 18, 15, 12,  12,   8,   5,
        11, 13, 13, 11, -3,   3,   8,   3,
        7,  7,  7,  5,  4,  -3,  -5,  -3,
        4,  3, 13,  1,  2,   1,  -1,   2,
        3,  5,  8,  4, -5,  -6,  -8, -11,
        -4,  0, -5, -1, -7, -12,  -8, -16,
        -6, -6,  0,  2, -9,  -9, -11,  -3,
        -9,  2,  3, -1, -5, -13,   4, -20,
    };

    static const int queenMiddleGame[] = {
        -28,   0,  29,  12,  59,  44,  43,  45,
        -24, -39,  -5,   1, -16,  57,  28,  54,
        -13, -17,   7,   8,  29,  56,  47,  57,
        -27, -27, -16, -16,  -1,  17,  -2,   1,
        -9, -26,  -9, -10,  -2,  -4,   3,  -3,
        -14,   2, -11,  -2,  -5,   2,  14,   5,
        -35,  -8,  11,   2,   8,  15,  -3,   1,
        -1, -18,  -9,  10, -15, -25, -31, -50,
    };

    static const int queenEndGame[] = {
        -9,  22,  22,  27,  27,  19,  10,  20,
        -17,  20,  32,  41,  58,  25,  30,   0,
        -20,   6,   9,  49,  47,  35,  19,   9,
        3,  22,  24,  45,  57,  40,  57,  36,
        -18,  28,  19,  47,  31,  34,  39,  23,
        -16, -27,  15,   6,   9,  17,  10,   5,
        -22, -23, -30, -16, -16, -23, -36, -32,
        -33, -28, -22, -43,  -5, -32, -20, -41,
    };

    static const int kingMiddleGame[] = {
        -65,  23,  16, -15, -56, -34,   2,  13,
        29,  -1, -20,  -7,  -8,  -4, -38, -29,
        -9,  24,   2, -16, -20,   6,  22, -22,
        -17, -20, -12, -27, -30, -25, -14, -36,
        -49,  -1, -27, -39, -46, -44, -33, -51,
        -14, -14, -22, -46, -44, -30, -15, -27,
        1,   7,  -8, -64, -43, -16,   9,   8,
        -15,  36,  12, -54,   8, -28,  24,  14,
    };

    static const int kingEndGame[] = {
        -74, -35, -18, -18, -11,  15,   4, -17,
        -12,  17,  14,  17,  17,  38,  23,  11,
        10,  17,  23,  15,  20,  45,  44,  13,
        -8,  22,  24,  27,  26,  33,  26,   3,
        -18,  -4,  21,  24,  27,  23,   9, -11,
        -19,  -3,  11,  21,  23,  16,   7,  -9,
        -27, -11,   4,  13,  14,   4,  -5, -17,
        -53, -34, -21, -11, -28, -14, -24, -43
    };

    static std::unordered_map<PieceType, const int *> positionBonusMiddleGame = {
        {PieceType::Pawn, pawnMiddleGame},
        {PieceType::Bishop, bishopMiddleGame},
        {PieceType::Knight, knightMiddleGame},
        {PieceType::Rook, rookMiddleGame},
        {PieceType::Queen, queenMiddleGame},
        {PieceType::King, kingMiddleGame},
    };

    static std::unordered_map<PieceType, const int *> positionBonusEndGame = {
        {PieceType::Pawn, pawnEndGame},
        {PieceType::Bishop, bishopEndGame},
        {PieceType::Knight, knightEndGame},
        {PieceType::Rook, rookEndGame},
        {PieceType::Queen, queenEndGame},
        {PieceType::King, kingEndGame},
    };

    static int piecePhase[PieceType::PTNumber] = {0, 1, 1, 2, 4, 0};
    
    static unsigned int totalPhase = piecePhase[PieceType::Pawn - 1] * 16 +
                                     piecePhase[PieceType::Knight - 1] * 4 +
                                     piecePhase[PieceType::Bishop - 1] * 4 +
                                     piecePhase[PieceType::Rook - 1] * 4 +
                                     piecePhase[PieceType::Queen - 1] * 2 +
                                     piecePhase[PieceType::King - 1] * 2;

    std::unordered_map<Color, std::pair<int, int>> positionScore = {
        {Color::Black, {0, 0}},
        {Color::White, {0, 0}},
    };
    
    int phase = totalPhase;

    for (size_t rank = 0; rank < 8; rank++) {
        for (size_t file = 0; file < 8; file++) {
            Piece &piece = this->getPiece(ID(file, rank));

            unsigned int relativeRank = rank;

            if (piece.color == Color::White) { // table are oriented as if viewed from white position
                relativeRank = 7 - rank;
            }

            if (piece.pieceType != PieceType::None) {
                phase -= piecePhase[piece.pieceType - 1];

                positionScore[piece.color].first += ::pieceTypeValue[piece.pieceType].first + // opening value
                                                    positionBonusMiddleGame[piece.pieceType][ID(file, relativeRank)];
                positionScore[piece.color].second += ::pieceTypeValue[piece.pieceType].second + // endgame value
                                                    positionBonusEndGame[piece.pieceType][ID(file, relativeRank)];
                    
            }
        }
    }

    phase = (phase * 256 + (totalPhase / 2)) / totalPhase;

    int openingScore = positionScore[Color::Black].first - positionScore[Color::White].first;
    int endgameScore = positionScore[Color::Black].second - positionScore[Color::White].second;
    
    if (this->activeColor == Color::White) {
        openingScore = -openingScore;
        endgameScore = -endgameScore;
    }

    int score = ((openingScore * (256 - phase)) + (endgameScore * phase)) / 256;

    /*std::cout << "Current evaluation (phase = " << phase << "/" << totalPhase << ") : " << score << "\n";
    std::cout << "\t\tBlack | White:\n"
                 "\topening : " << positionScore[Color::Black].first << " | " << positionScore[Color::White].first;
    std::cout << "\n\tending : " << positionScore[Color::Black].second << " | " << positionScore[Color::White].second;
    std::cout << "\nOpening score : " << openingScore << " => " << (openingScore * (256 - phase)) << "\n";
    std::cout << "Endgame score : " << endgameScore << " => " << (endgameScore * phase) << "\n";
    std::cout << std::endl;*/

    return score;
}

Key &Game::getHash() {
    return this->hash;
}

std::vector<bool> Game::getCastlingRights(Color color) {
    return this->castle[color];
}

unsigned int Game::getEnPassantTargetSquare() {
    return this->enPassantTargetSquare;
}

unsigned int Game::getKingSquare(Color color) {
    return this->kingSquare[color];
}

std::unordered_map<PieceType, unsigned char> &Game::getCapturedPieces(Color color) {
    return this->capturedPieces[color];
}

Color Game::getActiveColor() {
    return this->activeColor;
}

std::string Game::getPositionFEN() {
    std::unordered_map<PieceType, char> pieceTypeSymbol = {
        {PieceType::Pawn, 'P'},
        {PieceType::Bishop,'B'},
        {PieceType::Knight, 'N'},
        {PieceType::Rook, 'R'},
        {PieceType::Queen, 'Q'},
        {PieceType::King, 'K'},
    };

    std::string positionFEN;

    for (size_t r = 0; r < 8; r++) {
        size_t rank = 7 - r;
        size_t skipFiles = 0;

        for (size_t file = 0; file < 8; file++) {
            const Piece &currentPiece = this->board[ID(file, rank)];

            if (currentPiece.pieceType == PieceType::None) {
                skipFiles++;
                continue;
            }

            char currentPieceSymbol = pieceTypeSymbol[currentPiece.pieceType];

            if (currentPiece.color == Color::Black) {
                currentPieceSymbol = utils::toLowerCase(currentPieceSymbol);
            }

            if (skipFiles != 0) {
                positionFEN += '0' + skipFiles;
                skipFiles = 0;
            }

            positionFEN += currentPieceSymbol;
        }


        if (skipFiles != 0) {
            positionFEN += '0' + skipFiles;
        }

        positionFEN += "/";
    }

    positionFEN += " ";
    positionFEN += colorSymbol(this->activeColor);
    positionFEN += " ";

    std::string allowedCastles = "";

    if (this->castle[Color::White][0]) {
        allowedCastles += "K";
    }
    if (this->castle[Color::White][1]) {
        allowedCastles += "Q";
    }
    if (this->castle[Color::Black][0]) {
        allowedCastles += "k";
    }
    if (this->castle[Color::Black][1]) {
        allowedCastles += "q";
    }

    if (allowedCastles.size() == 0) {
        allowedCastles = "-";
    }

    positionFEN += allowedCastles;
    positionFEN += " ";

    if (this->enPassantTargetSquare < 64) {
        positionFEN += utils::caseNameFromId(this->enPassantTargetSquare);
    } else {
        positionFEN += "-";
    }

    positionFEN += " ";
    positionFEN += std::to_string(this->halfMoveNumber);
    positionFEN += " ";
    positionFEN += std::to_string(this->fullMoveNumber);

    return positionFEN;
}

Piece &Game::getPiece(unsigned int squareId) {
    return this->board[squareId];
}

const std::string Game::move2str(Move &move) {
    if (move.isCastling()) {
        if (move.getCastlingSide() == 1) {
            return "O-O-O";
        }

        return "O-O";
    }

    std::string moveStr = "";
    Piece &currentPiece = this->board[move.getOriginSquare()];
    unsigned char originFile = FILE(move.getOriginSquare())/*, originRank = RANK(move.getOriginSquare())*/;
    // unsigned char targetFile = FILE(move.getTargetSquare()), targetRank = RANK(move.getTargetSquare());

    if (currentPiece.pieceType == PieceType::Pawn && move.isCapture()) {
        moveStr += fileSymbol(originFile);
    }

    if (currentPiece.pieceType != PieceType::Pawn) {
        moveStr += pieceTypeSymbol(currentPiece.pieceType);
    }

    if (move.isCapture()) {
        moveStr += 'x';
    }

    moveStr += utils::caseNameFromId(move.getTargetSquare());

    if (move.isPromotion()) {
        moveStr += "=";
        moveStr += promotionSymbol(move.getPromotedPiece());
    }

    if (move.isCheck()) {
        if (move.isMate()) {
            moveStr += "#";
        } else {
            moveStr += "+";
        }
    } else if (move.isMate()) {
        moveStr += "="; // stale mate
    }

    return moveStr;
}

Move Game::str2move(const std::string &move) {
    unsigned int originSquare = utils::idFromCaseName(move.substr(0, 2)), targetSquare = utils::idFromCaseName(move.substr(2));
    std::vector<Move> legalMoves;
    this->generateLegalMoves(legalMoves, originSquare);

    for (Move &currentMove : legalMoves) {
        if (currentMove.getTargetSquare() == targetSquare) {
            return currentMove;
        }
    }

    return Move();
}

}