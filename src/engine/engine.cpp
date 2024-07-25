#include "include/movesgeneration.hpp"
#include "include/piece.hpp"
#include "include/utils.hpp"
#include "include/engine.hpp"
#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>

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

    this->history.push_back({this->hash, Move()});

    return 0;
}

Result Game::result(std::vector<Move> &legalMoves) {
    bool inCheck = this->isAttackedBy(this->getKingSquare(this->getActiveColor()), getOppositeColor(this->getActiveColor()));

    if (this->halfMoveNumber >= 100) {
        if (inCheck && legalMoves.size() == 0) {
            return Result::CheckMate;
        } else { // stalemate count as draw
            return Result::Draw;
        }
    }

    if (legalMoves.size() == 0) {
        if (inCheck) { 
            return Result::CheckMate;
        } else {
            return Result::Draw;
        }
    }

    if (this->hasRepeated()) {
        return Result::Draw;
    }

    return Result::Undecided;
}

bool Game::hasRepeated() {
    int repeats = 0;

    for (const std::pair<Key, Move> &previousPosition : this->history) {
        if (this->hash == previousPosition.first) {
            repeats++;
        }

        if (repeats == 2) {
            return true;
        }
    }

    return false;
}

bool Game::isAttackedBy(unsigned int squareId, Color color) {
    int pawnSide = (color == Color::Black) ? 1 : -1;
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
                testedSquare = mailbox10x12[mailbox8x8[testedSquare] + currentOffset]; // next square in the current direction

                if (testedSquare == XX) { // got out of the board
                    break;
                }

                Piece &currentTestedPiece = this->getPiece(testedSquare);

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
            if (move.getOriginSquare() == castlingRookSquareIds[selectedPiece.color][i].first) {
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
                if (move.getTargetSquare() == castlingRookSquareIds[targetSquarePiece.color][i].first) {
                    this->castle[targetSquarePiece.color][i] = false;
                }
            }
        }

        this->capturedPieces[this->activeColor][targetSquarePiece.pieceType]++;
    }

    if (move.isCastling()) {
        unsigned int castlingSide = move.getCastlingSide();
        unsigned int castlingOriginSquare = castlingRookSquareIds[this->activeColor][castlingSide].first;

        this->board[castlingRookSquareIds[this->activeColor][castlingSide].second] = this->board[castlingOriginSquare]; // move rook
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
    this->history.push_back({this->hash, move});

    return savedState;
}

void Game::undoMove(Move &move, MoveSaveState savedState) {
    this->history.pop_back();
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
        unsigned int castlingOriginSquare = castlingRookSquareIds[this->activeColor][castlingSide].first;

        this->board[castlingOriginSquare] = this->board[castlingRookSquareIds[this->activeColor][castlingSide].second]; // move rook
        this->board[castlingRookSquareIds[this->activeColor][castlingSide].second] = {PieceType::None, Color::Black}; // empty old rook position
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
        std::pair<unsigned int, unsigned int> rookSquares = castlingRookSquareIds[movedPiece.color][move.getCastlingSide()];

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
    generateLegalMoves(*this, legalMoves, originSquare);

    for (Move &currentMove : legalMoves) {
        if (currentMove.getTargetSquare() == targetSquare) {
            return currentMove;
        }
    }

    return Move();
}

}