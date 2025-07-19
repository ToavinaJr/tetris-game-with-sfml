#include "../includes/Game.hpp"
#include <cstdlib>
#include <ctime>

/**
 * @brief Constructeur de la classe Game.
 * 
 * Initialise la fenêtre SFML, la grille de jeu (Board), ainsi que les Tetrominos courant et suivant.
 * 
 * @param width Largeur du plateau en nombre de blocs.
 * @param height Hauteur du plateau en nombre de blocs.
 * @param t Taille (en pixels) d’un bloc (tile).
 */
Game::Game(int width, int height, int t)
    : window(sf::VideoMode(width*t, height*t), "Tetris POO + Ghost + Explosion"),
      board(width, height), tileSize(t), timer(0), delay(0.5f),
      clearing(false), clearTimer(0.f), gameOver(false)
{
    srand(time(nullptr));
    current = std::make_unique<Tetromino>(TetrominoType(rand()%7), width/2);
    next = std::make_unique<Tetromino>(TetrominoType(rand()%7), width/2);
}

/**
 * @brief Gère les entrées utilisateur et les événements SFML.
 * 
 * Détecte :
 * - Les mouvements gauche/droite (`Left`/`Right`)
 * - La rotation (`Up`)
 * - La descente rapide (`Down`)
 * - Le "hard drop" (descente instantanée avec `Space`)
 * - La fermeture de la fenêtre (`Closed`)
 */
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
                // Hard drop
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

/**
 * @brief Calcule et retourne le "ghost piece" du Tetromino actuel.
 * 
 * Le ghost est une version translucide du Tetromino qui indique
 * où il atterrira si on le laisse tomber.
 * 
 * @return Une copie du Tetromino positionnée à la dernière ligne libre.
 */
Tetromino Game::computeGhost() const {
    Tetromino ghost = *current;
    ghost.setColor(sf::Color(200,200,200,120));
    while (!board.checkCollision(ghost)) ghost.move(0,1);
    ghost.move(0,-1);
    return ghost;
}

/**
 * @brief Met à jour la logique du jeu.
 * 
 * - Gère le déplacement automatique des pièces.
 * - Fusionne un Tetromino lorsqu'il touche le bas ou un autre bloc.
 * - Lance l'animation d’effacement des lignes et gère la création du prochain Tetromino.
 * 
 * @param dt Temps écoulé depuis la dernière mise à jour (delta time).
 */
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

/**
 * @brief Dessine tous les éléments du jeu.
 * 
 * - Grille et blocs existants
 * - Tetromino courant
 * - Ghost du Tetromino
 * - Tetromino suivant
 * - Animation d’explosion des lignes effacées
 * - Message "Game Over" si la partie est terminée
 */
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

    // Affiche le prochain Tetromino dans un coin
    if (next) {
        Tetromino next_display = *next;
        next_display.move(board.getWidth() + 2, 2);
        next_display.draw(window, tileSize);
    }

    // Affiche le message Game Over
    if (gameOver) {
        sf::Text gameOverText;
        sf::Font font;
        if (!font.loadFromFile("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf")) {
            // Gérer l'erreur de chargement
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

/**
 * @brief Boucle principale du jeu.
 * 
 * Gère l'exécution continue du jeu :
 * - Lecture des entrées
 * - Mise à jour de la logique
 * - Rendu graphique
 */
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
