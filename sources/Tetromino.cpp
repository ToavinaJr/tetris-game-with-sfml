#include "../includes/Tetromino.hpp"
#include <SFML/Graphics.hpp>

namespace {
    // SHAPES: I, O, T, S, Z, J, L
    constexpr std::array<std::array<int,4>,7> SHAPES {
        {
            {1,3,5,7},   // I
            {2,3,4,5},   // O
            {3,5,4,6},   // T
            {3,4,5,6},   // S
            {2,4,5,7},   // Z
            {2,3,5,7},   // J
            {3,5,4,7}    // L
        }
    };
    // COLORS: Cyan, Yellow, Magenta, Green, Red, Blue, Orange pour les 7 types de Tetrominos
    std::array<sf::Color,7> COLORS {{
        sf::Color::Cyan, sf::Color::Yellow, sf::Color::Magenta,
        sf::Color::Green, sf::Color::Red, sf::Color::Blue, sf::Color(255,165,0)
    }};
}

// Constructeur de Tetromino
Tetromino::Tetromino(TetrominoType t, int startX) : type(t) {
    // on caste le type en entier pour accéder aux formes et couleurs
    int n = static_cast<int>(t);
    color = COLORS[n];
    
    // Remplir les blocs avec les coordonnées de la forme correspondante
    for (int i=0; i<4; i++) {
        blocks[i].x = SHAPES[n][i] % 2 + startX;
        blocks[i].y = SHAPES[n][i] / 2;
    }
}

// Déplacement du Tetromino
void Tetromino::move(int dx, int dy) {
    for (auto& b : blocks) {
        b.x += dx;
        b.y += dy;
    }
}

void Tetromino::rotate() {
    // Le Tetromino O ne peut pas être tourné, on le laisse tel quel
    if (type == TetrominoType::O) return;

    // Rotation autour du deuxième bloc (le pivot)
    auto pivot = blocks[1];

    // Appliquer la rotation 90 degrés dans le sens horaire
    for (auto& b : blocks) {
        int x = b.y - pivot.y;
        int y = b.x - pivot.x;

        b.x = pivot.x - x;
        b.y = pivot.y + y;
    }
}

// Dessiner le Tetromino
void Tetromino::draw(sf::RenderWindow& window, int tileSize) {
    sf::RectangleShape block(sf::Vector2f(tileSize-1, tileSize-1));
    block.setFillColor(color);

    for (auto& b : blocks) {
        block.setPosition(b.x * tileSize, b.y * tileSize);
        window.draw(block);
    }
}