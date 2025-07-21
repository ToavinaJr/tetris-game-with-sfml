#include "../includes/Game.hpp"
#include <cstdlib>
#include <ctime>
#include <fstream>

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
    : window(sf::VideoMode(width*t + 200, height*t), "Tetris SFML"),
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
    setupPauseButtons();

    loadBestScore();
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
 * @brief Gère les clics sur les boutons du menu.
 *
 * @param mousePos Position actuelle de la souris dans la fenêtre.
 */
void Game::handlePauseClick(const sf::Vector2f& mousePos) {
    for (auto& btn : pauseButtons) {
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

        // --- Gestion du Game Over ---
        if (state == GameState::GAME_OVER) {
            if (e.type == sf::Event::KeyPressed && e.key.code == sf::Keyboard::R) {
                resetGame();
            }
            continue; // On ne fait rien d'autre si on est en Game Over
        }

        // --- Gestion du Menu ---
        if (state == GameState::MENU) {
            if (e.type == sf::Event::MouseButtonPressed &&
                e.mouseButton.button == sf::Mouse::Left) {
                sf::Vector2f mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
                handleMenuClick(mousePos);
            }
            continue;
        }

        // --- Aide et A propos ---
        if ((state == GameState::HELP || state == GameState::ABOUT) &&
            e.type == sf::Event::KeyPressed &&
            e.key.code == sf::Keyboard::Escape) {
            state = GameState::MENU;
            continue;
        }

        // --- Pause ---
        if (state == GameState::PAUSED) {
            // Gestion des clics sur les boutons de pause
            if (e.type == sf::Event::MouseButtonPressed &&
                e.mouseButton.button == sf::Mouse::Left) {
                sf::Vector2f mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
                handlePauseClick(mousePos);
            }

            // Reprise rapide avec P
            if (e.type == sf::Event::KeyPressed && e.key.code == sf::Keyboard::P) {
                state = GameState::PLAYING;
            }

            continue; // Ne pas traiter la logique du jeu si on est en pause
        }

        // --- Touche Pause depuis le jeu ---
        if (state == GameState::PLAYING) {
            if (e.type == sf::Event::KeyPressed && e.key.code == sf::Keyboard::P) {
                state = GameState::PAUSED;
                continue;
            }
        }

        // --- Ne pas continuer si Game Over pendant le jeu ---
        if (gameOver) continue;

        // --- Gestion des mouvements et du Hard Drop ---
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
            int cleared = board.getLinesToClear().size(); // nombre de lignes supprimées
            board.performClearLines();

            // ✅ Mise à jour du score et du niveau
            if (cleared > 0) {
                score += cleared * 100;        // 100 points par ligne
                level = 1 + score / 1000;      // +1 niveau tous les 1000 points
                if (score > bestScore) bestScore = score;

                // ✅ Ajuste la vitesse du jeu en fonction du niveau
                delay = std::max(0.1f, 0.5f - (level - 1) * 0.05f);
            }

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
 * @brief Dessine l'aperçu de la prochaine pièce (Tetromino) dans un panneau latéral.
 *
 * Cette fonction affiche un petit cadre à droite de la grille de jeu
 * indiquant au joueur quel sera le prochain Tetromino à apparaître.
 *
 * @details
 * - Affiche le titre "Next:" au-dessus du cadre.
 * - Dessine un cadre (rectangle) servant de zone d'aperçu.
 * - Copie la pièce suivante (`next`), normalise sa position
 *   pour qu'elle soit centrée dans le cadre, puis dessine chaque bloc.
 *
 * @note
 * - La fonction retourne immédiatement si aucun Tetromino suivant n'est défini (`next == nullptr`).
 * - Les coordonnées des blocs sont recalculées pour qu'ils s'affichent proprement
 *   dans un espace restreint de 4x5 cases.
 *
 * @warning
 * La fonction suppose que `next` est une pièce valide (pas de vérification approfondie
 * sur l'intégrité des données).
 *
 * @see Game::render() Pour l'endroit où cette fonction est appelée.
 */
void Game::drawNextPiece() {
    if (!next) return;

    float panelX = board.getWidth() * tileSize + 20.f;
    float panelY = 150.f;

    // Titre
    sf::Text nextLabel("Next:", font, 20);
    nextLabel.setFillColor(sf::Color::White);
    nextLabel.setPosition(panelX, panelY - 30.f);
    window.draw(nextLabel);

    // Cadre
    sf::RectangleShape box(sf::Vector2f(tileSize * 4, tileSize * 5));
    box.setPosition(panelX, panelY);
    box.setFillColor(sf::Color(30,30,30));
    box.setOutlineColor(sf::Color::White);
    box.setOutlineThickness(2);
    window.draw(box);

    // Copie de la pièce suivante pour la dessiner dans le cadre
    Tetromino preview = *next;

    // On récupère ses blocs pour les repositionner
    auto blocks = preview.getBlocks();

    // Trouver les minX et minY pour "normaliser" les coordonnées
    int minX = blocks[0].x, minY = blocks[0].y;
    for (auto &b : blocks) {
        if (b.x < minX) minX = b.x;
        if (b.y < minY) minY = b.y;
    }

    // Décaler et dessiner manuellement les blocs
    sf::RectangleShape rect(sf::Vector2f(tileSize - 1, tileSize - 1));
    rect.setFillColor(preview.getColor());

    for (auto &b : blocks) {
        float drawX = panelX + ((b.x - minX) + 1) * tileSize;
        float drawY = panelY + ((b.y - minY) + 1) * tileSize;
        rect.setPosition(drawX, drawY);
        window.draw(rect);
    }
}


/**
 * @brief Affiche les informations du jeu (Score, Meilleur score, Niveau).
 *
 * Cette fonction dessine les informations sur le côté droit de la fenêtre,
 * à l'extérieur de la grille du jeu.
 *
 * @details
 * - Le score actuel est affiché en blanc.
 * - Le meilleur score (Best) est affiché en jaune.
 * - Le niveau actuel est affiché en cyan.
 *
 * @note Les informations se positionnent en fonction de la largeur du plateau (`board.getWidth()`).
 */
void Game::drawScore() {
    float infoX = board.getWidth() * tileSize + 20.f;

    sf::Text scoreText("Score: " + std::to_string(score), font, 20);
    scoreText.setFillColor(sf::Color::White);
    scoreText.setPosition(infoX, 20);
    window.draw(scoreText);

    sf::Text bestText("Best: " + std::to_string(bestScore), font, 18);
    bestText.setFillColor(sf::Color::Yellow);
    bestText.setPosition(infoX, 50);
    window.draw(bestText);

    sf::Text levelText("Level: " + std::to_string(level), font, 18);
    levelText.setFillColor(sf::Color::Cyan);
    levelText.setPosition(infoX, 80);
    window.draw(levelText);
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
            // Efface tout avec un fond noir
            window.clear(sf::Color::Black);

            // === Titre Game Over ===
            sf::Text gameOverText("=== GAME OVER ===", font, 50);
            gameOverText.setFillColor(sf::Color::Red);
            sf::FloatRect bounds = gameOverText.getLocalBounds();
            gameOverText.setOrigin(bounds.width / 2.f, bounds.height / 2.f);
            gameOverText.setPosition(window.getSize().x / 2.f, window.getSize().y / 2.f - 80);
            window.draw(gameOverText);

            // === Affichage du Score ===
            sf::Text scoreText("Score : " + std::to_string(score), font, 30);
            scoreText.setFillColor(sf::Color::Yellow);
            bounds = scoreText.getLocalBounds();
            scoreText.setOrigin(bounds.width / 2.f, bounds.height / 2.f);
            scoreText.setPosition(window.getSize().x / 2.f, window.getSize().y / 2.f - 20);
            window.draw(scoreText);

            // === Meilleur Score ===
            sf::Text bestText("Best : " + std::to_string(bestScore), font, 25);
            bestText.setFillColor(sf::Color::Cyan);
            bounds = bestText.getLocalBounds();
            bestText.setOrigin(bounds.width / 2.f, bounds.height / 2.f);
            bestText.setPosition(window.getSize().x / 2.f, window.getSize().y / 2.f + 20);
            window.draw(bestText);

            // === Instructions pour rejouer ===
            sf::Text infoText("Appuyez sur R pour rejouer", font, 22);
            infoText.setFillColor(sf::Color::White);
            bounds = infoText.getLocalBounds();
            infoText.setOrigin(bounds.width / 2.f, bounds.height / 2.f);
            infoText.setPosition(window.getSize().x / 2.f, window.getSize().y / 2.f + 80);
            window.draw(infoText);

            window.display();
            return; // Empêche d'afficher le reste du jeu
        }

        if (state == GameState::PAUSED) {
            drawPause();
        }
    }

    if (state == GameState::PLAYING || state == GameState::GAME_OVER || state == GameState::PAUSED) {
        drawScore();
        drawNextPiece();
    }

    window.display();
}


/**
 * @brief Réinitialise complètement la partie pour recommencer.
 *
 * Cette fonction est appelée lorsqu’on appuie sur la touche **R** après un Game Over
 * ou lorsque l’on souhaite recommencer une nouvelle partie.
 *
 * @details
 * - Réinitialise la grille (`Board`).
 * - Réinitialise le score, le niveau, le timer et les états (`clearing`, `gameOver`, etc.).
 * - Génère un nouveau Tetromino courant et un prochain Tetromino aléatoire.
 *
 * @note Le meilleur score n’est pas remis à zéro (il est conservé entre les parties).
 */
void Game::resetGame() {
    board = Board(board.getWidth(), board.getHeight());

    score = 0;
    level = 1;
    timer = 0;
    delay = 0.5f;
    clearing = false;
    clearTimer = 0;
    gameOver = false;
    state = GameState::PLAYING;

    current = std::make_unique<Tetromino>(TetrominoType(rand()%7), board.getWidth()/2);
    next = std::make_unique<Tetromino>(TetrominoType(rand()%7), board.getWidth()/2);
}


/**
 * @brief Charge le meilleur score sauvegardé depuis un fichier.
 *
 * Lit le fichier `scores.txt` (s'il existe) et récupère la valeur du meilleur score.
 *
 * @warning Si le fichier n’existe pas ou n’est pas lisible, le meilleur score reste inchangé.
 */
void Game::loadBestScore() {
    std::ifstream file("scores.txt");
    if (file.is_open()) {
        file >> bestScore;
        file.close();
    }
}


/**
 * @brief Sauvegarde le meilleur score dans un fichier.
 *
 * Écrit le meilleur score dans le fichier `scores.txt`.
 *
 * @warning Si le fichier n’est pas accessible en écriture, la sauvegarde échoue silencieusement.
 */
void Game::saveBestScore() {
    if (score > bestScore) {
        bestScore = score;
        std::ofstream file("scores.txt");
        if (file.is_open()) {
            file << bestScore;
            file.close();
        }
    }
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
 * @brief Affiche le menu de pause avec ses boutons.
 */
void Game::drawPause() {
    // Fond semi-transparent sur toute la fenêtre
    sf::RectangleShape overlay(sf::Vector2f(window.getSize().x, window.getSize().y));
    overlay.setFillColor(sf::Color(0, 0, 0, 0));
    window.draw(overlay);

    // === Titre "PAUSE" centré uniquement dans la grille ===
    float gridWidth = board.getWidth() * tileSize;
    sf::Text pauseTitle("=== PAUSE ===", font, 40);
    pauseTitle.setFillColor(sf::Color::Yellow);
    sf::FloatRect titleBounds = pauseTitle.getLocalBounds();
    pauseTitle.setOrigin(titleBounds.width / 2.f, titleBounds.height / 2.f);
    pauseTitle.setPosition(gridWidth / 2.f, 100.f); // à ~100px du haut de la grille
    window.draw(pauseTitle);

    // === Boutons ===
    sf::Vector2f mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
    for (auto &btn : pauseButtons) {
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
 * @brief Met à jour le score, le niveau et la vitesse du jeu après l'effacement de lignes.
 *
 * Cette fonction applique un système de points basé sur le nombre de lignes effacées en une seule fois,
 * met à jour le nombre total de lignes effacées et augmente progressivement le niveau et la vitesse du jeu.
 *
 * @param linesCleared Nombre de lignes effacées simultanément (valeur attendue entre 0 et 4).
 *
 * @details
 * - Barème des points attribués :
 *   - 0 ligne : 0 point
 *   - 1 ligne : 100 points
 *   - 2 lignes : 300 points
 *   - 3 lignes : 500 points
 *   - 4 lignes : 800 points (Tetris)
 * - Tous les 10 lignes effacées (`totalLinesCleared / 10 >= level`), le niveau augmente de 1.
 * - La vitesse de descente (`delay`) diminue progressivement (multipliée par 0.95 à chaque niveau),
 *   mais ne descend jamais en dessous de **0.1 seconde**.
 *
 * @note Cette fonction ne sauvegarde pas le score maximum, elle ne fait qu’actualiser l’état actuel de la partie.
 *
 * @warning Assurez-vous que `linesCleared` est compris entre 0 et 4 pour éviter tout accès hors limites dans `comboPoints`.
 */
void Game::updateScore(int linesCleared) {
    static const std::array<int,5> comboPoints = {0,100,300,500,800};
    score += comboPoints[linesCleared];

    totalLinesCleared += linesCleared;
    if (totalLinesCleared / 10 >= level) {
        level++;
        delay = std::max(0.1f, delay * 0.95f); // accélération progressive
    }
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
 * @brief Configure les boutons du menu pause (Reprendre, Aller au menu, Quitter).
 */
void Game::setupPauseButtons() {
    std::vector<std::string> labels = {"Reprendre", "Aller au menu", "Quitter"};

    float width = 220.f;
    float height = 50.f;
    float spacing = 70.f;

    float gridWidth = board.getWidth() * tileSize;
    float gridHeight = board.getHeight() * tileSize;

    float totalHeight = labels.size() * height + (labels.size() - 1) * (spacing - height);
    float startY = (gridHeight - totalHeight) / 2.f;
    float centerX = gridWidth / 2.f;

    pauseButtons.clear();
    for (size_t i = 0; i < labels.size(); i++) {
        Button btn;
        btn.shape.setSize(sf::Vector2f(width, height));

        float posX = centerX - width / 2.f;
        float posY = startY + i * spacing;

        btn.shape.setPosition(posX, posY);
        btn.shape.setFillColor(sf::Color(100, 100, 100));
        btn.shape.setOutlineColor(sf::Color::White);
        btn.shape.setOutlineThickness(2);

        btn.text.setFont(font);
        btn.text.setString(labels[i]);
        btn.text.setCharacterSize(22);
        btn.text.setFillColor(sf::Color::White);

        // Centrage du texte
        sf::FloatRect textBounds = btn.text.getLocalBounds();
        btn.text.setOrigin(textBounds.left + textBounds.width / 2.f,
                           textBounds.top + textBounds.height / 2.f);
        btn.text.setPosition(centerX, posY + height / 2.f);

        // Actions des boutons
        if (labels[i] == "Reprendre") {
            btn.onClick = [this]() { state = GameState::PLAYING; };
        } else if (labels[i] == "Aller au menu") {
            btn.onClick = [this]() {
                resetGame();
                saveBestScore();
                state = GameState::MENU; 
            };
        } else if (labels[i] == "Quitter") {
            btn.onClick = [this]() { window.close(); };
        }

        pauseButtons.push_back(btn);
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
