#ifndef GAME_HPP
#define GAME_HPP

#include <SFML/Graphics.hpp>
#include <memory>
#include "Board.hpp"
#include "Tetromino.hpp"

class Game {
public:
    Game(int width, int height, int tileSize);
    void run();

private:
    void processEvents();
    void update(float dt);
    void render();
    Tetromino computeGhost() const;

    sf::RenderWindow window;
    Board board;
    int tileSize;

    std::unique_ptr<Tetromino> current;
    std::unique_ptr<Tetromino> next; // Pièce suivante

    float timer;
    float delay;

    bool clearing;
    float clearTimer;

    bool gameOver;
};

#endif // GAME_HPP