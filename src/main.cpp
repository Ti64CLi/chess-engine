#include "engine/include/evaluation.hpp"
#include "engine/include/piece.hpp"
#include "engine/include/utils.hpp"
#include "include/ui.hpp"
#include "engine/include/search.hpp"
#include "engine/include/engine.hpp"
#include "engine/include/movesgeneration.hpp"
#include <iostream>
#include <string>
#include <vector>

#define BOARD_RECTANGLE_WIDTH 45
#define SEARCH_DEPTH 5

int main() {
    std::string title("Chess engine v");
    title += ENGINE_VERSION;

    std::string fen;

    std::cout << "Enter position : ";
    getline(std::cin, fen);
    
    if (fen.size() == 0) {
        fen = engine::startPosition;
    }

    bool inGame = true, selected = false;
    unsigned int selectedCaseId = 64;
    engine::Game game(fen);
    std::vector<engine::MoveSaveState> savedStates;
    std::vector<engine::Move> savedMoves;
    engine::MoveValuation bestMoveValuation = {engine::Move(), 0xc0ffee};

    ui::init(title, BOARD_RECTANGLE_WIDTH, 3.0f);

    std::vector<engine::Move> moves;

    while (inGame) {
        engine::generateAllLegalMoves(game, moves);
        engine::Result result = game.result(moves);

        if (result == engine::Result::Draw) {
            std::cout << "Draw !" << std::endl;

            inGame = false;
            break;
        } else if (result == engine::Result::CheckMate) {
            std::cout << "Check mate ! " << (game.getActiveColor() == engine::Color::White ? "Black" : "White") << " wins !" << std::endl;

            inGame = false;
            break;
        }

        ui::clear();
        ui::renderBoard();

        if (selected) {
            ui::renderSelectedCase(selectedCaseId);
        }

        ui::renderPosition(game.getPositionFEN());
        //ui::renderMoves(moves);
        ui::renderCapturedPieces(0, game.getCapturedPieces(engine::Color::Black));
        ui::renderCapturedPieces(1, game.getCapturedPieces(engine::Color::White));
        ui::show();

        if (bestMoveValuation.second == 0xc0ffee) {
            unsigned long long moveCount = 0;
            bestMoveValuation = engine::negaMax(game, SEARCH_DEPTH, SEARCH_DEPTH, moveCount, true);
            std::cout << "AI move : " << game.move2str(bestMoveValuation.first) << " with valuation " << bestMoveValuation.second << std::endl;
            std::cout << "AI move : " << utils::caseNameFromId(bestMoveValuation.first.getOriginSquare()) << utils::caseNameFromId(bestMoveValuation.first.getTargetSquare()) << std::endl;
            if (abs(bestMoveValuation.second) >= engine::MAX_SCORE - 256) {
                std::cout << "Check mate in " << (engine::MAX_SCORE - abs(bestMoveValuation.second)) << " moves" << std::endl;
            }
        }
        //if (game.getActiveColor() == engine::Color::Black) { // AI turn
            bestMoveValuation.second = 0xc0ffee;

            savedMoves.push_back(bestMoveValuation.first);
            savedStates.push_back(game.doMove(bestMoveValuation.first));

            std::cout << "New position : " << game.getPositionFEN() << " with valuation : " << engine::evaluate(game) << std::endl;
        /*} else {
            int event = 64;

            if ((event = ui::manageEvents()) == ERROR_EVENT) {
                inGame = false;
            } else if (event < INVALID_EVENT && event >= 0) {
                if (selected && ((unsigned int)event == selectedCaseId)) {
                    moves.clear();
                    selected = false;
                    // todo
                } else {
                    selectedCaseId = event;
                    selected = true;

                    for (engine::Move &move : moves) {
                        if (move.getTargetSquare() == selectedCaseId) {
                            bestMoveValuation.second = 0xc0ffee;
                            selected = false;

                            if (move.isPromotion()) {
                                unsigned int promotedPieceFlag = ui::handlePromotion(game.getActiveColor(), move.getTargetSquare());

                                move.clearFlags(M_PQUEEN);
                                move.setFlags(promotedPieceFlag);
                            }

                            savedMoves.push_back(move);
                            savedStates.push_back(game.doMove(move));

                            std::cout << "New position : " << game.getPositionFEN() << " with valuation : " << engine::evaluate(game) << std::endl;

                            break;
                        }
                    }

                    if (selected) {
                        generateLegalMoves(game, moves, selectedCaseId);
                        std::cout << "Moves allowed :" << std::endl;

                        for (engine::Move &move : moves) {
                            std::cout << "\t" << game.move2str(move) << std::endl;
                        }
                    } else {
                        moves.clear();
                    }
                }
            } else if (event == PMOVE_EVENT && savedStates.size() > 0) {
                selected = false;
                moves.clear();

                game.undoMove(savedMoves.back(), savedStates.back());

                savedMoves.pop_back();
                savedStates.pop_back();

                game.undoMove(savedMoves.back(), savedStates.back());

                savedMoves.pop_back();
                savedStates.pop_back();

                std::cout << "New position : " << game.getPositionFEN() << std::endl;
            }
        }*/
    }

    ui::close();

    std::cout << "Final position : " << game.getPositionFEN() << std::endl;

    return 0;
}