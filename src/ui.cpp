#include "engine/include/utils.hpp"
#include "engine/include/engine.hpp"
#include "include/ui.hpp"
#include <SFML/Graphics.hpp>
#include <string>
#include <iostream>
#include <unordered_map>

static std::string currentWindowTitle;
static unsigned int currentWindowWidth;
static unsigned int currentWindowHeight;
static unsigned int rectangleWidth;
static float scale;


static sf::RenderWindow currentWindow;

static const sf::Color lightCaseColor = sf::Color(180, 136, 98);
static const sf::Color darkCaseColor = sf::Color(240, 217, 180);
static const sf::Color lightTargetCaseColor = sf::Color(99, 110, 64);
static const sf::Color darkTargetCaseColor = sf::Color(130, 151, 104);
static const sf::Color lightSelectedCaseColor = sf::Color(99, 110, 64);
static const sf::Color darkSelectedCaseColor = sf::Color(130, 151, 104);

namespace ui {

int init(const std::string windowTitle, const unsigned int boardRectangleWidth, const float scale) {
    ::currentWindowTitle = windowTitle;
    ::currentWindowWidth = (boardRectangleWidth * 8 + LEFT_BORDER_WIDTH + RIGHT_BORDER_WIDTH) * scale;
    ::currentWindowHeight = (boardRectangleWidth * 8 + TOP_BORDER_HEIGHT + BOTTOM_BORDER_HEIGHT) * scale;
    ::rectangleWidth = boardRectangleWidth;
    ::scale = scale;

    ::currentWindow.create(sf::VideoMode(currentWindowWidth, currentWindowHeight), windowTitle, sf::Style::Titlebar | sf::Style::Close);

    return ::currentWindow.isOpen();
}

int manageEvents() {
    if (!::currentWindow.isOpen()) {
        return ERROR_EVENT;
    }

    sf::Event event;

    if (::currentWindow.waitEvent(event)) {
        switch (event.type) {
            case sf::Event::Closed: {
                close();

                return ERROR_EVENT;
            }

            case sf::Event::MouseButtonPressed: {
                switch (event.mouseButton.button) {
                    case sf::Mouse::Left: {
                        unsigned int file = ((event.mouseButton.x / ::scale) - LEFT_BORDER_WIDTH) / ::rectangleWidth, rank = 7 - (unsigned int)(((event.mouseButton.y / ::scale) - TOP_BORDER_HEIGHT) / ::rectangleWidth);
                        
                        /*std::cout << "Mouse coordinates : x = " << event.mouseButton.x << " y = " << event.mouseButton.y << std::endl;
                        std::cout << "Scale = " << ::scale << " TOP_BORDER_HEIGHT = " << TOP_BORDER_HEIGHT << " rectangleWidth = " << ::rectangleWidth << std::endl;
                        std::cout << "1st step = " << (event.mouseButton.y / ::scale) << std::endl;
                        std::cout << "2nd step = " << (event.mouseButton.y / ::scale) - TOP_BORDER_HEIGHT << std::endl;
                        std::cout << "3rd step = " << ((event.mouseButton.y / ::scale) - TOP_BORDER_HEIGHT) / ::rectangleWidth << std::endl;
                        std::cout << "4th step = " << 7 - (((event.mouseButton.y / ::scale) - TOP_BORDER_HEIGHT) / ::rectangleWidth) << std::endl;*/
                        //std::cout << "Seleted case : file = " << file << " rank = " << rank << std::endl;
                        
                        return ID(file, rank);
                    }

                    case sf::Mouse::Right: {
                        return PMOVE_EVENT;
                    }

                    // todo

                    default:
                        break;
                }
            }


            // todo

            default:
                break;
        }
    }

    return INVALID_EVENT;
}

unsigned int handlePromotion(engine::Color pieceColor, unsigned int targetSquare) {
    if (!::currentWindow.isOpen()) {
        return -1;
    }

    unsigned int promotedPieceType = (M_PQUEEN >> 7);
    bool done = false;

    while (!done) {
        sf::Event event;

        engine::Piece promotedPiece = {
            (engine::PieceType)(engine::PieceType::Bishop + promotedPieceType),
            pieceColor,
        };
        renderSquare(targetSquare);
        renderPiece(engine::pieceSymbol(promotedPiece), targetSquare);
        show();

        if (::currentWindow.waitEvent(event)) {
            switch (event.type) {
                case sf::Event::MouseButtonPressed: {
                    switch (event.mouseButton.button) {
                        case sf::Mouse::Right: {
                            promotedPieceType += 1;
                            promotedPieceType %= 4;

                            break;
                        }

                        case sf::Mouse::Left: {
                            done = true;

                            break;
                        }

                        default:
                            break;
                    }

                    break;
                }

                default:
                    break;
            }
        }
    }

    return (promotedPieceType << 7);
}

int clear() {
    if (!::currentWindow.isOpen()) {
        return -1;
    }

    ::currentWindow.clear(sf::Color(130, 151, 104));

    return 0;
}

int renderSquare(unsigned int squareId) {
    if (!::currentWindow.isOpen()) {
        return -1;
    }

    size_t file = FILE(squareId), rank = RANK(squareId);
    sf::RectangleShape rectangle(sf::Vector2f(::rectangleWidth * ::scale, ::rectangleWidth * ::scale));

    rectangle.setPosition(sf::Vector2f((LEFT_BORDER_WIDTH + file * ::rectangleWidth) * ::scale, (TOP_BORDER_HEIGHT + (7 - rank) * ::rectangleWidth) * ::scale));

    if ((file + rank) % 2 == 1) {
        rectangle.setFillColor(::darkCaseColor);
    } else {
        rectangle.setFillColor(::lightCaseColor);
    }

    ::currentWindow.draw(rectangle);

    return 0;
}

int renderBoard() {
    if (!::currentWindow.isOpen()) {
        return -1;
    }

    for (unsigned int squareId = 0; squareId < 64; squareId++) {
        if (renderSquare(squareId) != 0) {
            return 1;
        }
    }
    
    return 0;
}

int renderSelectedCase(const unsigned int selectedCaseId) {
    if (!::currentWindow.isOpen()) {
        return -1;
    }

    size_t file = FILE(selectedCaseId), rank = RANK(selectedCaseId);

    sf::RectangleShape rectangle(sf::Vector2f(::rectangleWidth * ::scale, ::rectangleWidth * ::scale));
    rectangle.setPosition(sf::Vector2f((LEFT_BORDER_WIDTH + file * ::rectangleWidth) * ::scale, (TOP_BORDER_HEIGHT + (7 - rank) * ::rectangleWidth) * ::scale));

    if ((file + rank) % 2 == 1) {
        rectangle.setFillColor(::darkSelectedCaseColor);
    } else {
        rectangle.setFillColor(::lightSelectedCaseColor);
    }

    ::currentWindow.draw(rectangle);

    return 0;
}

int renderPiece(char pieceSymbol, unsigned int targetSquare) {
    if (!::currentWindow.isOpen()) {
        return -1;
    }

    size_t file = FILE(targetSquare), rank = RANK(targetSquare);
    sf::Texture currentPieceTexture;
    std::string fileName("src/assets/");
    fileName += pieceSymbol;
    fileName += ".png";

    if (!currentPieceTexture.loadFromFile(fileName)) {
        std::cerr << "[ERROR] Couldn't load texture from '" << fileName << "'" << std::endl;

        return 1;
    }

    currentPieceTexture.setSmooth(true);

    sf::Sprite currentPieceSprite;
    currentPieceSprite.setTexture(currentPieceTexture);
    currentPieceSprite.setPosition(sf::Vector2f((LEFT_BORDER_WIDTH + file * ::rectangleWidth) * ::scale, (TOP_BORDER_HEIGHT + (7 - rank) * ::rectangleWidth) * ::scale));
    currentPieceSprite.scale(sf::Vector2f(::scale, ::scale));   

    ::currentWindow.draw(currentPieceSprite);

    return 0;
}

int renderPosition(const std::string &fen) {
    if (!::currentWindow.isOpen()) {
        return -1;
    }

    std::string board = utils::split(fen)[0];
    size_t file = 0, rank = 7;

    for (char c : board) {
        if (c == '/') {
            rank--;
            file = 0;
        } else {
            if ('0' <= c && c <= '9') {
                file += c - '0';
            } else {
                if (renderPiece(c, ID(file, rank)) != 0) {
                    return 1;
                }

                file++;
            }
        }
    }

    return 0;
}

int renderMoves(std::vector<engine::Move> &moves) {
    if (!::currentWindow.isOpen()) {
        return -1;
    }

    for (engine::Move &move : moves) {
        // size_t startFile = FILE(move.first), startRank = RANK(move.first);
        size_t endFile = FILE(move.getTargetSquare()), endRank = RANK(move.getTargetSquare());
        float circleRadius = ((float)::rectangleWidth) / 6.f;

        sf::CircleShape circle(circleRadius * ::scale);
        circle.setPosition(sf::Vector2f((LEFT_BORDER_WIDTH + 2 * circleRadius + endFile * ::rectangleWidth) * ::scale, (TOP_BORDER_HEIGHT + 2 * circleRadius + (7 - endRank) * ::rectangleWidth) * ::scale));

        if ((endFile + endRank) % 2 == 1) {
            circle.setFillColor(::darkTargetCaseColor);
        } else {
            circle.setFillColor(::lightTargetCaseColor);
        }

        ::currentWindow.draw(circle);
    }

    return 0;
}

int renderCapturedPieces(int side, std::unordered_map<engine::PieceType, unsigned char> &capturedPieces) {
    if (!::currentWindow.isOpen()) {
        return -1;
    }

    float localScale = 0.5f;
    float x = LEFT_BORDER_WIDTH, y;

    for (size_t piece = 1; piece < 7; piece++) {
        float xOffset = 0.f;

        for (size_t n = 0; n < capturedPieces[(engine::PieceType)piece]; n++) {
            engine::Piece currentPiece = {(engine::PieceType)piece, (engine::Color)(engine::Color::White - side)};
            sf::Texture currentPieceTexture;
            std::string fileName("src/assets/");
            fileName += engine::pieceSymbol(currentPiece);
            fileName += ".png";

            if (!currentPieceTexture.loadFromFile(fileName)) {
                std::cerr << "[ERROR] Couldn't load texture from '" << fileName << "'" << std::endl;

                return 1;
            }

            currentPieceTexture.setSmooth(true);

            if (side == 0) {
                y = (TOP_BORDER_HEIGHT - (currentPieceTexture.getSize().y * localScale)) / 2.f;
            } else {
                y = (BOTTOM_BORDER_HEIGHT - (currentPieceTexture.getSize().y * localScale)) / 2.f;
                y += TOP_BORDER_HEIGHT + ::rectangleWidth * 8;
            }
            xOffset = (currentPieceTexture.getSize().x * localScale) / 2.f;

            sf::Sprite currentPieceSprite;
            currentPieceSprite.setTexture(currentPieceTexture);
            currentPieceSprite.setPosition(sf::Vector2f(x * ::scale, y * ::scale));
            currentPieceSprite.scale(sf::Vector2f(localScale * ::scale, localScale * ::scale));   

            ::currentWindow.draw(currentPieceSprite);

            x += xOffset;
        }

        x += xOffset;
    }

    return 0;
}

int show() {
    if (!::currentWindow.isOpen()) {
        return -1;
    }

    ::currentWindow.display();

    return 0;
}

int close() {
    if (!::currentWindow.isOpen()) {
        return -1;
    }

    ::currentWindow.close();

    return 0;
}

}