#include "../includes/Tetromino.hpp"
#include <SFML/Graphics.hpp>

namespace {
constexpr std::array<std::array<int,4>,7> SHAPES {{
    {1,3,5,7},
    {2,3,4,5},
    {3,5,4,6},
    {3,4,5,6},
    {2,4,5,7},
    {2,3,5,7},
    {3,5,4,7}
}};
std::array<sf::Color,7> COLORS {{
    sf::Color::Cyan, sf::Color::Yellow, sf::Color::Magenta,
    sf::Color::Green, sf::Color::Red, sf::Color::Blue, sf::Color(255,165,0)
}};
}

Tetromino::Tetromino(TetrominoType t, int startX) : type(t) {
    int n = static_cast<int>(t);
    color = COLORS[n];
    for (int i=0; i<4; i++) {
        blocks[i].x = SHAPES[n][i] % 2 + startX;
        blocks[i].y = SHAPES[n][i] / 2;
    }
}

void Tetromino::move(int dx, int dy) {
    for (auto& b : blocks) {
        b.x += dx;
        b.y += dy;
    }
}

void Tetromino::rotate() {
    if (type == TetrominoType::O) return;
    auto pivot = blocks[1];
    for (auto& b : blocks) {
        int x = b.y - pivot.y;
        int y = b.x - pivot.x;
        b.x = pivot.x - x;
        b.y = pivot.y + y;
    }
}

void Tetromino::draw(sf::RenderWindow& window, int tileSize) {
    sf::RectangleShape block(sf::Vector2f(tileSize-1, tileSize-1));
    block.setFillColor(color);
    for (auto& b : blocks) {
        block.setPosition(b.x * tileSize, b.y * tileSize);
        window.draw(block);
    }
}