#ifndef BOARD_HPP
#define BOARD_HPP

#include <SFML/Graphics.hpp>
#include <vector>
#include <memory>
#include "Tetromino.hpp"

class Board {
public:
    Board(int w, int h);
    bool checkCollision(const Tetromino& tetro) const;
    void mergeTetromino(const Tetromino& tetro);
    void detectLinesToClear();
    void performClearLines();
    void draw(sf::RenderWindow& window, int tileSize);
    void drawGrid(sf::RenderWindow& window, int tileSize);
    void drawExplosion(sf::RenderWindow& window, int tileSize, float animTime);

    int getWidth() const { return width; }
    int getHeight() const { return height; }
    bool isClearing() const { return !linesToClear.empty(); }
    const std::vector<int>& getLinesToClear() const { return linesToClear; }

private:
    int width;
    int height;
    std::vector<std::vector<sf::Color>> grid;
    std::vector<int> linesToClear;
};

#endif // BOARD_HPP