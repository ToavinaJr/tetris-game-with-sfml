#ifndef TETROMINO_HPP
#define TETROMINO_HPP

#include <array>
#include <SFML/Graphics.hpp>

enum class TetrominoType { I, O, T, S, Z, J, L };

class Tetromino {
public:
    Tetromino(TetrominoType type, int startX);

    void move(int dx, int dy);
    void rotate();
    void draw(sf::RenderWindow& window, int tileSize);
    std::array<sf::Vector2i, 4> getBlocks() const { return blocks; }
    void setBlocks(const std::array<sf::Vector2i, 4>& b) { blocks = b; }
    inline void setColor(sf::Color c) { color = c; }
    inline sf::Color getColor() const { return color; }

private:
    TetrominoType type;
    std::array<sf::Vector2i, 4> blocks;
    sf::Color color;
};

#endif // TETROMINO_HPP