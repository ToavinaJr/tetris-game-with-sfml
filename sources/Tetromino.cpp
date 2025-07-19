#include "../includes/Tetromino.hpp"
#include <SFML/Graphics.hpp>

namespace {
    /**
     * @brief Liste des formes des Tetrominos (I, O, T, S, Z, J, L).
     * 
     * Chaque forme est représentée par 4 entiers (positions sur une grille 2x4).
     * Exemple : `{1,3,5,7}` représente les coordonnées d'un Tetromino en partant du coin supérieur gauche.
     */
    constexpr std::array<std::array<int,4>,7> SHAPES {
        {
            {1,3,5,7},   ///< Forme I
            {2,3,4,5},   ///< Forme O
            {3,5,4,6},   ///< Forme T
            {3,4,5,6},   ///< Forme S
            {2,4,5,7},   ///< Forme Z
            {2,3,5,7},   ///< Forme J
            {3,5,4,7}    ///< Forme L
        }
    };

    /**
     * @brief Couleurs des 7 Tetrominos.
     * 
     * Ordre : Cyan, Jaune, Magenta, Vert, Rouge, Bleu, Orange.
     */
    std::array<sf::Color,7> COLORS {{
        sf::Color::Cyan, sf::Color::Yellow, sf::Color::Magenta,
        sf::Color::Green, sf::Color::Red, sf::Color::Blue, sf::Color(255,165,0)
    }};
}

/**
 * @brief Constructeur du Tetromino.
 * 
 * Initialise la forme et la couleur en fonction du type passé en paramètre.
 * La position de départ est centrée sur `startX`.
 * 
 * @param t Le type du Tetromino (I, O, T, S, Z, J, L).
 * @param startX La position de départ sur l'axe X (en blocs).
 */
Tetromino::Tetromino(TetrominoType t, int startX) : type(t) {
    int n = static_cast<int>(t);
    color = COLORS[n];
    
    // Remplir les coordonnées des 4 blocs du Tetromino
    for (int i=0; i<4; i++) {
        blocks[i].x = SHAPES[n][i] % 2 + startX;
        blocks[i].y = SHAPES[n][i] / 2;
    }
}

/**
 * @brief Déplace le Tetromino.
 * 
 * @param dx Déplacement sur l'axe X (en blocs).
 * @param dy Déplacement sur l'axe Y (en blocs).
 */
void Tetromino::move(int dx, int dy) {
    for (auto& b : blocks) {
        b.x += dx;
        b.y += dy;
    }
}

/**
 * @brief Fait pivoter le Tetromino de 90° dans le sens horaire.
 * 
 * Le pivot est le deuxième bloc du Tetromino.
 * Le Tetromino de type O ne tourne pas.
 */
void Tetromino::rotate() {
    if (type == TetrominoType::O) return; // Le carré ne tourne pas

    auto pivot = blocks[1];

    for (auto& b : blocks) {
        int x = b.y - pivot.y;
        int y = b.x - pivot.x;

        b.x = pivot.x - x;
        b.y = pivot.y + y;
    }
}

/**
 * @brief Dessine le Tetromino à l'écran.
 * 
 * @param window La fenêtre SFML dans laquelle dessiner.
 * @param tileSize Taille d'un bloc en pixels.
 */
void Tetromino::draw(sf::RenderWindow& window, int tileSize) {
    sf::RectangleShape block(sf::Vector2f(tileSize-1, tileSize-1));
    block.setFillColor(color);

    for (auto& b : blocks) {
        block.setPosition(b.x * tileSize, b.y * tileSize);
        window.draw(block);
    }
}
