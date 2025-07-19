#include "../includes/Game.hpp"
#include <cstdlib>
#include <ctime>

Game::Game(int width, int height, int t)
    : window(sf::VideoMode(width*t, height*t), "Tetris POO + Ghost + Explosion"),
      board(width, height), tileSize(t), timer(0), delay(0.5f),
      clearing(false), clearTimer(0.f), gameOver(false)
{
    srand(time(nullptr));
    current = std::make_unique<Tetromino>(TetrominoType(rand()%7), width/2);
    next = std::make_unique<Tetromino>(TetrominoType(rand()%7), width/2);
}

void Game::processEvents() {
    sf::Event e;
    while (window.pollEvent(e)) {
        if (e.type == sf::Event::Closed) window.close();
        if (!clearing && e.type == sf::Event::KeyPressed) {
            if (e.key.code == sf::Keyboard::Left) {
                current->move(-1,0);
                if (board.checkCollision(*current)) current->move(1,0);
            }
            else if (e.key.code == sf::Keyboard::Right) {
                current->move(1,0);
                if (board.checkCollision(*current)) current->move(-1,0);
            }
            else if (e.key.code == sf::Keyboard::Up) {
                auto backup = current->getBlocks();
                current->rotate();
                if (board.checkCollision(*current)) current->setBlocks(backup);
            }
            else if (e.key.code == sf::Keyboard::Down) delay = 0.05f;
            else if (e.key.code == sf::Keyboard::Space) {
                while (!board.checkCollision(*current)) current->move(0,1);
                current->move(0,-1);
                board.mergeTetromino(*current);
                board.detectLinesToClear();
                if (board.isClearing()) {
                    clearing = true;
                    clearTimer = 0;
                } else {
                    current = std::move(next);
                    next = std::make_unique<Tetromino>(TetrominoType(rand()%7), board.getWidth()/2);
                    if (board.checkCollision(*current)) gameOver = true;
                }
                timer = 0;
            }
        }
    }
}

Tetromino Game::computeGhost() const {
    Tetromino ghost = *current;
    ghost.setColor(sf::Color(200,200,200,120));
    while (!board.checkCollision(ghost)) ghost.move(0,1);
    ghost.move(0,-1);
    return ghost;
}

void Game::update(float dt) {
    if (gameOver) return;

    if (clearing) {
        clearTimer += dt;
        if (clearTimer > 0.3f) {
            board.performClearLines();
            clearing = false;
            clearTimer = 0;
            current = std::move(next);
            next = std::make_unique<Tetromino>(TetrominoType(rand()%7), board.getWidth()/2);
            if (board.checkCollision(*current)) gameOver = true;
        }
        return;
    }

    timer += dt;
    if (timer > delay) {
        current->move(0,1);

        if (board.checkCollision(*current)) {
            current->move(0,-1);

            board.mergeTetromino(*current);

            board.detectLinesToClear();
            if (board.isClearing()) {
                clearing = true;
                clearTimer = 0;
            } else {
                current = std::move(next);
                next = std::make_unique<Tetromino>(TetrominoType(rand()%7), board.getWidth()/2);
                if (board.checkCollision(*current)) gameOver = true;
            }
        }
        timer = 0;
    }
}

void Game::render() {
    window.clear(sf::Color::Black);

    board.drawGrid(window, tileSize);
    board.draw(window, tileSize);

    if (!clearing) {
        auto ghost = computeGhost();
        ghost.draw(window, tileSize);
        current->draw(window, tileSize);
    } else {
        board.drawExplosion(window, tileSize, clearTimer);
    }

    // Dessin du next Tetromino (peut être ajusté pour sa position)
    if (next) {
        Tetromino next_display = *next;
        next_display.move(board.getWidth() + 2, 2);
        next_display.draw(window, tileSize);
    }

    if (gameOver) {
        sf::Text gameOverText;
        sf::Font font;
        if (!font.loadFromFile("arial.ttf")) {
        }
        gameOverText.setFont(font);
        gameOverText.setString("Game Over!");
        gameOverText.setCharacterSize(50);
        gameOverText.setFillColor(sf::Color::Red);
        sf::FloatRect textRect = gameOverText.getLocalBounds();
        gameOverText.setOrigin(textRect.left + textRect.width/2.0f,
                               textRect.top  + textRect.height/2.0f);
        gameOverText.setPosition(window.getSize().x/2.0f, window.getSize().y/2.0f);
        window.draw(gameOverText);
    }

    window.display();
}

void Game::run() {
    sf::Clock clock;
    while (window.isOpen() && !gameOver) {
        float dt = clock.restart().asSeconds();
        processEvents();
        update(dt);
        delay = 0.5f;
        render();
    }
}