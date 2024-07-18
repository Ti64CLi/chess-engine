#include "include/movesgeneration.hpp"
#include "include/piece.hpp"
#include "include/engine.hpp"
#include "include/utils.hpp"

/*static std::unordered_map<engine::PieceType, std::pair<std::vector<int>, bool>> pieceTypeOffsets = {
    {engine::PieceType::Bishop, {{-11, -9, 9, 11}, true}},
    {engine::PieceType::Knight, {{-21, -19, -12, -8, 8, 12, 19, 21}, false}},
    {engine::PieceType::Rook, {{-10, -1, 1, 10}, true}},
    {engine::PieceType::Queen, {{-11, -10, -9, -1, 1, 9, 10, 11}, true}},
    {engine::PieceType::King, {{-11, -10, -9, -1, 1, 9, 10, 11}, false}},
};*/

namespace engine {

void generatePseudoLegalMoves(Game &game, std::vector<Move> &pseudoLegalMoves, unsigned int selectedCaseId) {
    static std::vector<std::vector<int>> castlingOffsets = {
        {1, 2},
        {-1, -2, -3},
    };

    if (game.getPiece(selectedCaseId).color != game.getActiveColor() || game.getPiece(selectedCaseId).pieceType == PieceType::None) {
        return;
    }

    Piece &selectedPiece = game.getPiece(selectedCaseId);
    unsigned int activeColorLastRank = (game.getActiveColor() == Color::Black) ? 0 : 7;

    switch (selectedPiece.pieceType) {
        case PieceType::Pawn: {
            int side = (game.getActiveColor() == Color::White) ? 1 : -1;
            unsigned int targetSquare9 = mailbox10x12[mailbox8x8[selectedCaseId] + (9 * side)];
            unsigned int targetSquare10 = mailbox10x12[mailbox8x8[selectedCaseId] + (10 * side)];
            unsigned int targetSquare20 = mailbox10x12[mailbox8x8[selectedCaseId] + (20 * side)];
            unsigned int targetSquare11 = mailbox10x12[mailbox8x8[selectedCaseId] + (11 * side)];

            if (targetSquare9 != XX && // valid id
                (game.getEnPassantTargetSquare() == targetSquare9 || // en passant possible
                (game.getPiece(targetSquare9).pieceType != PieceType::None && 
                game.getPiece(targetSquare9).color != selectedPiece.color))) { // or opponent piece
                unsigned int flags = M_CAPTURE;
                Piece capturedPiece = game.getPiece(targetSquare9);

                if (game.getEnPassantTargetSquare() == targetSquare9) { // en passant
                    capturedPiece.pieceType = PieceType::Pawn;
                    capturedPiece.color = getOppositeColor(game.getActiveColor());
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
                (game.getEnPassantTargetSquare() == targetSquare11 || // en passant possible
                (game.getPiece(targetSquare11).pieceType != PieceType::None &&
                game.getPiece(targetSquare11).color != selectedPiece.color))) { // or opponent piece
                unsigned int flags = M_CAPTURE;
                Piece capturedPiece = game.getPiece(targetSquare11);

                if (game.getEnPassantTargetSquare() == targetSquare11) { // en passant
                    capturedPiece.pieceType = PieceType::Pawn;
                    capturedPiece.color = getOppositeColor(game.getActiveColor());
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
                game.getPiece(targetSquare10).pieceType == PieceType::None) { // no piece on target square
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
                    game.getPiece(targetSquare20).pieceType == PieceType::None) { // target square is empty
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
                    targetSquare = mailbox10x12[mailbox8x8[targetSquare] + offset]; // get next target square

                    if (targetSquare == XX) { // outside of the board
                        break;
                    }

                    Piece &targetSquarePiece = game.getPiece(targetSquare);

                    if (targetSquarePiece.pieceType != PieceType::None) { // blocked by a piece
                        if (targetSquarePiece.color != game.getActiveColor()) { // capture opponent piece
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
            bool possible = game.getCastlingRights(game.getActiveColor())[castlingSide] && !game.isAttackedBy(selectedCaseId, getOppositeColor(game.getActiveColor()));

            if (possible) {
                for (const auto &offset : castlingOffsets[castlingSide]) {
                    if (game.getPiece(selectedCaseId + offset).pieceType != PieceType::None || (game.isAttackedBy(selectedCaseId + offset, getOppositeColor(game.getActiveColor())) && offset != -3)) {
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

void generateAllPseudoLegalMoves(Game &game, std::vector<Move> &pseudoLegalMoves) {
    for (unsigned int caseId = 0; caseId < 64; caseId++) {
        if (game.getPiece(caseId).color != game.getActiveColor() || game.getPiece(caseId).pieceType == PieceType::None) {
            continue;
        }

        generatePseudoLegalMoves(game, pseudoLegalMoves, caseId);
    }
}

void generateLegalMoves(Game &game, std::vector<Move> &legalMoves, unsigned int selectedCaseId, bool capturesOnly) {
    if (game.getPiece(selectedCaseId).color != game.getActiveColor() || game.getPiece(selectedCaseId).pieceType == PieceType::None) {
        return;
    }

    std::vector<Move> pseudoLegalMoves;
    unsigned int kingSquare = game.getKingSquare(game.getActiveColor());

    generatePseudoLegalMoves(game, pseudoLegalMoves, selectedCaseId);

    for (Move move : pseudoLegalMoves) {
        if (capturesOnly && !move.isCapture()) {
            continue;
        }

        unsigned int currentKingSquare = kingSquare;

        if (move.getOriginSquare() == kingSquare) {
            currentKingSquare = move.getTargetSquare();
        }

        MoveSaveState savedState = game.doMove(move);

        if (!game.isAttackedBy(currentKingSquare, game.getActiveColor())) { // left our king in check ?
            legalMoves.push_back(move);
        }

        game.undoMove(move, savedState);
    }
}

void generateAllLegalMoves(Game &game, std::vector<Move> &legalMoves, bool capturesOnly) {
    for (unsigned int selectedCaseId = 0; selectedCaseId < 64; selectedCaseId++) {
        if (game.getPiece(selectedCaseId).color != game.getActiveColor() || game.getPiece(selectedCaseId).pieceType == PieceType::None) {
            continue;
        }

        generateLegalMoves(game, legalMoves, selectedCaseId, capturesOnly);
    }
}

} // namespace engine
