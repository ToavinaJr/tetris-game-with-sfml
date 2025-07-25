#ifndef GAME_HPP
#define GAME_HPP

#include <SFML/Graphics.hpp>
#include <memory>
#include <vector>
#include <functional>
#include "Board.hpp"
#include "Tetromino.hpp"

enum class GameState {
    MENU,
    PLAYING,
    PAUSED,
    HELP,
    ABOUT,
    GAME_OVER
};

// Structure pour gérer un bouton simple
struct Button {
    sf::RectangleShape shape;
    sf::Text text;
    std::function<void()> onClick; ///< Action à exécuter si cliqué

    bool isMouseOver(const sf::Vector2f& mousePos) const {
        return shape.getGlobalBounds().contains(mousePos);
    }
};

class Game {
public:
    Game(int width, int height, int tileSize);
    void run();

private:
    void processEvents();
    void update(float dt);
    void render();

    void setupMenuButtons();
    void setupPauseButtons();

    void handleMenuClick(const sf::Vector2f& mousePos);
    void handlePauseClick(const sf::Vector2f& mousePos);

    void drawMenu();
    void drawHelp();
    void drawAbout();
    void drawPause();
    void drawNextPiece();

    void resetGame();
    void loadBestScore();
    void saveBestScore();
    void updateScore(int linesCleared);
    void drawScore();

    Tetromino computeGhost() const;

    sf::RenderWindow window;
    Board board;
    int tileSize;

    std::unique_ptr<Tetromino> current;
    std::unique_ptr<Tetromino> next;

    int score = 0;
    int level = 1;
    int totalLinesCleared = 0;
    int bestScore = 0;

    float timer;
    float delay;

    bool clearing;
    float clearTimer;
    bool gameOver;

    GameState state;
    sf::Font font;

    // Boutons dans le jeu
    std::vector<Button> menuButtons;
    std::vector<Button> pauseButtons;
};

#endif // GAME_HPP
