#include "../includes/Game.hpp"
#include <cstdlib>
#include <ctime>

/**
 * @brief Constructeur de la classe Game.
 *
 * Initialise la fenêtre, le plateau de jeu, charge la police d'écriture,
 * génère les deux premiers Tetrominos (courant et suivant) et configure
 * les boutons du menu.
 *
 * @param width Largeur du plateau (en nombre de cases).
 * @param height Hauteur du plateau (en nombre de cases).
 * @param t Taille d'une case (en pixels).
 *
 * @throws std::runtime_error Si la police ne peut pas être chargée.
 */
Game::Game(int width, int height, int t)
    : window(sf::VideoMode(width*t, height*t), "Tetris SFML + Menu"),
      board(width, height), tileSize(t), timer(0), delay(0.5f),
      clearing(false), clearTimer(0.f), gameOver(false),
      state(GameState::MENU)
{
    srand(time(nullptr));
    if (!font.loadFromFile("/usr/share/fonts/truetype/noto/NotoSans-Regular.ttf")) {
        throw std::runtime_error("Impossible de charger la police Noto !");
    }

    current = std::make_unique<Tetromino>(TetrominoType(rand()%7), width/2);
    next = std::make_unique<Tetromino>(TetrominoType(rand()%7), width/2);

    setupMenuButtons();
}

/**
 * @brief Gère les clics sur les boutons du menu.
 *
 * @param mousePos Position actuelle de la souris dans la fenêtre.
 */
void Game::handleMenuClick(const sf::Vector2f& mousePos) {
    for (auto& btn : menuButtons) {
        if (btn.isMouseOver(mousePos)) {
            btn.onClick();
            break;
        }
    }
}

/**
 * @brief Gère tous les événements de la fenêtre (clavier et souris).
 *
 * - Ferme la fenêtre si l'événement est Closed.
 * - Gère les clics sur le menu.
 * - Retour au menu avec ESC depuis Aide ou À propos.
 * - Met le jeu en pause ou le reprend avec P.
 * - Déplace, fait tourner ou fait descendre le Tetromino courant.
 * - Effectue le "Hard Drop" avec la touche Espace.
 */
void Game::processEvents() {
    sf::Event e;
    while (window.pollEvent(e)) {
        if (e.type == sf::Event::Closed) window.close();

        if (state == GameState::MENU) {
            if (e.type == sf::Event::MouseButtonPressed &&
                e.mouseButton.button == sf::Mouse::Left) {
                sf::Vector2f mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
                handleMenuClick(mousePos);
            }
        }

        if ((state == GameState::HELP || state == GameState::ABOUT) &&
            e.type == sf::Event::KeyPressed &&
            e.key.code == sf::Keyboard::Escape) {
            state = GameState::MENU;
        }

        if (state == GameState::PLAYING || state == GameState::PAUSED) {
            if (e.type == sf::Event::KeyPressed && e.key.code == sf::Keyboard::P) {
                if (state == GameState::PLAYING)
                    state = GameState::PAUSED;
                else if (state == GameState::PAUSED)
                    state = GameState::PLAYING;
            }
        }

        if (state != GameState::PLAYING || gameOver) continue;

        // Gestion des mouvements et du hard drop
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
                    if (board.checkCollision(*current)) {
                        gameOver = true;
                        state = GameState::GAME_OVER;
                    }
                }
                timer = 0;
            }
        }
    }
}

/**
 * @brief Calcule la position du "Ghost Piece" (ombre du Tetromino).
 *
 * Le Ghost Piece est affiché en transparence pour indiquer où le Tetromino
 * courant atterrira s'il est lâché directement.
 *
 * @return Tetromino Copie du Tetromino courant positionné en bas.
 */
Tetromino Game::computeGhost() const {
    Tetromino ghost = *current;
    ghost.setColor(sf::Color(200,200,200,120));
    while (!board.checkCollision(ghost)) ghost.move(0,1);
    ghost.move(0,-1);
    return ghost;
}

/**
 * @brief Met à jour la logique du jeu (mouvement automatique, fusion des Tetrominos, lignes à effacer).
 *
 * @param dt Temps écoulé depuis la dernière frame (en secondes).
 */
void Game::update(float dt) {
    if (state != GameState::PLAYING || gameOver) return;

    if (clearing) {
        clearTimer += dt;
        if (clearTimer > 0.3f) {
            board.performClearLines();
            clearing = false;
            clearTimer = 0;
            current = std::move(next);
            next = std::make_unique<Tetromino>(TetrominoType(rand()%7), board.getWidth()/2);
            if (board.checkCollision(*current)) {
                gameOver = true;
                state = GameState::GAME_OVER;
            }
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
                if (board.checkCollision(*current)) {
                    gameOver = true;
                    state = GameState::GAME_OVER;
                }
            }
        }
        timer = 0;
    }
}

/**
 * @brief Dessine tous les éléments en fonction de l'état du jeu.
 *
 * - Menu, Aide, À propos
 * - Plateau de jeu, Tetrominos, lignes effacées
 * - Écran de pause ou de fin
 */
void Game::render() {
    window.clear(sf::Color::Black);

    if (state == GameState::MENU) {
        drawMenu();
    }
    else if (state == GameState::HELP) {
        drawHelp();
    }
    else if (state == GameState::ABOUT) {
        drawAbout();
    }
    else if (state == GameState::PLAYING || state == GameState::GAME_OVER || state == GameState::PAUSED) {
        board.drawGrid(window, tileSize);
        board.draw(window, tileSize);

        if (!clearing) {
            auto ghost = computeGhost();
            ghost.draw(window, tileSize);
            current->draw(window, tileSize);
        } else {
            board.drawExplosion(window, tileSize, clearTimer);
        }

        if (next) {
            Tetromino next_display = *next;
            next_display.move(board.getWidth() + 2, 2);
            next_display.draw(window, tileSize);
        }

        if (gameOver) {
            sf::Text gameOverText("Game Over!\nAppuyez sur ESC pour quitter", font, 40);
            gameOverText.setFillColor(sf::Color::Red);
            gameOverText.setPosition(50, 200);
            window.draw(gameOverText);
        }

        if (state == GameState::PAUSED) {
            drawPause();
        }
    }

    window.display();
}

/**
 * @brief Affiche le menu principal avec les boutons.
 */
void Game::drawMenu() {
    sf::Text title("=== TETRIS SFML ===", font, 40);
    title.setFillColor(sf::Color::Cyan);

    sf::FloatRect titleBounds = title.getLocalBounds();
    title.setOrigin(titleBounds.left + titleBounds.width / 2.0f,
                    titleBounds.top + titleBounds.height / 2.0f);
    title.setPosition(window.getSize().x / 2.0f, 80.f);

    window.draw(title);

    sf::Vector2f mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
    for (auto& btn : menuButtons) {
        if (btn.isMouseOver(mousePos)) {
            btn.shape.setFillColor(sf::Color(150, 150, 150));
        } else {
            btn.shape.setFillColor(sf::Color(100, 100, 100));
        }

        window.draw(btn.shape);
        window.draw(btn.text);
    }
}

/**
 * @brief Affiche l'écran d'aide avec les commandes.
 */
void Game::drawHelp() {
    sf::Text help(
        "=== Aide ===\n"
        "Fleche Gauche/Droite : Deplacer\n"
        "Fleche Haut : Rotation\n"
        "Fleche Bas : Descente rapide\n"
        "Espace : Hard drop\n\n"
        "ESC : Retour au menu", font, 18);
    help.setFillColor(sf::Color::Yellow);

    sf::FloatRect bounds = help.getLocalBounds();
    help.setOrigin(bounds.left + bounds.width / 2.f, bounds.top + bounds.height / 2.f);
    help.setPosition(window.getSize().x / 2.f, window.getSize().y / 2.f - 50);

    window.draw(help);
}

/**
 * @brief Affiche les informations "À propos".
 */
void Game::drawAbout() {
    sf::Text about(
        "=== A propos ===\n"
        "Tetris en C++23 & SFML\n"
        "Auteur : Toavina Sylvianno\n"
        "2025\n\n"
        "ECHAP : Retour au menu", font, 18);
    about.setFillColor(sf::Color::Green);

    sf::FloatRect bounds = about.getLocalBounds();
    about.setOrigin(bounds.left + bounds.width / 2.f, bounds.top + bounds.height / 2.f);
    about.setPosition(window.getSize().x / 2.f, window.getSize().y / 2.f - 50);

    window.draw(about);
}

/**
 * @brief Affiche l'écran de pause.
 */
void Game::drawPause() {
    sf::Text pauseText("PAUSE", font, 50);
    pauseText.setFillColor(sf::Color::Yellow);

    sf::FloatRect bounds = pauseText.getLocalBounds();
    pauseText.setOrigin(bounds.left + bounds.width / 2.f, bounds.top + bounds.height / 2.f);
    pauseText.setPosition(window.getSize().x / 2.f, window.getSize().y / 2.f);

    window.draw(pauseText);

    sf::Text infoText("Appuyez sur P pour reprendre", font, 20);
    infoText.setFillColor(sf::Color::White);

    bounds = infoText.getLocalBounds();
    infoText.setOrigin(bounds.left + bounds.width / 2.f, bounds.top + bounds.height / 2.f);
    infoText.setPosition(window.getSize().x / 2.f, window.getSize().y / 2.f + 50);

    window.draw(infoText);
}

/**
 * @brief Configure les boutons du menu (Jouer, Aide, À propos, Quitter).
 */
void Game::setupMenuButtons() {
    std::vector<std::string> labels = {"Jouer", "Aide", "A propos", "Quitter"};

    float width = 200.f;
    float height = 50.f;
    float spacing = 70.f;

    float totalHeight = labels.size() * height + (labels.size() - 1) * (spacing - height);
    float startY = (window.getSize().y - totalHeight) / 2.0f;
    float centerX = window.getSize().x / 2.0f;

    menuButtons.clear();
    for (size_t i = 0; i < labels.size(); i++) {
        Button btn;
        btn.shape.setSize(sf::Vector2f(width, height));

        float posX = centerX - width / 2.0f;
        float posY = startY + i * spacing;

        btn.shape.setPosition(posX, posY);
        btn.shape.setFillColor(sf::Color(100, 100, 100));
        btn.shape.setOutlineColor(sf::Color::White);
        btn.shape.setOutlineThickness(2);

        btn.text.setFont(font);
        btn.text.setString(labels[i]);
        btn.text.setCharacterSize(24);
        btn.text.setFillColor(sf::Color::White);

        // Centrage du texte à l'intérieur du bouton
        sf::FloatRect textBounds = btn.text.getLocalBounds();
        btn.text.setOrigin(textBounds.left + textBounds.width / 2.0f,
                           textBounds.top + textBounds.height / 2.0f);
        btn.text.setPosition(centerX, posY + height / 2.0f);

        // Actions
        if (labels[i] == "Jouer") {
            btn.onClick = [this]() { state = GameState::PLAYING; };
        } else if (labels[i] == "Aide") {
            btn.onClick = [this]() { state = GameState::HELP; };
        } else if (labels[i] == "A propos") {
            btn.onClick = [this]() { state = GameState::ABOUT; };
        } else if (labels[i] == "Quitter") {
            btn.onClick = [this]() { window.close(); };
        }

        menuButtons.push_back(btn);
    }
}

/**
 * @brief Boucle principale du jeu.
 *
 * Gère les événements, met à jour la logique et dessine à chaque frame.
 */
void Game::run() {
    sf::Clock clock;
    while (window.isOpen()) {
        float dt = clock.restart().asSeconds();
        processEvents();
        update(dt);
        if (state == GameState::PLAYING) delay = 0.5f;
        render();
    }
}
