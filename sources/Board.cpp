#include "../includes/Board.hpp"

#include <algorithm>
#include <ranges>

Board::Board(int w, int h)
    : width(w), height(h),
      grid(h, std::vector<sf::Color>(w, sf::Color::Black)) {}

bool Board::checkCollision(const Tetromino& tetro) const {
    for (auto& b : tetro.getBlocks()) {
        if (b.x < 0 || b.x >= width || b.y >= height) return true;
        if (b.y >= 0 && grid[b.y][b.x] != sf::Color::Black) return true;
    }
    return false;
}

void Board::mergeTetromino(const Tetromino& tetro) {
    for (auto& b : tetro.getBlocks()) {
        if (b.y >= 0 && b.y < height) {
            grid[b.y][b.x] = tetro.getColor(); // Utiliser la vraie couleur du tétromino
        }
    }
}

void Board::detectLinesToClear() {
    linesToClear.clear();
    for (int i = 0; i < height; i++) {
        if (std::ranges::all_of(grid[i], [](auto& c){ return c != sf::Color::Black; })) {
            linesToClear.push_back(i);
        }
    }
}

void Board::performClearLines() {
    // Trier les lignes par ordre décroissant pour éviter les problèmes d'indices
    std::sort(linesToClear.rbegin(), linesToClear.rend());

    // Compter le nombre de lignes à effacer
    int linesCleared = linesToClear.size();
    
    // Créer une nouvelle grille
    std::vector<std::vector<sf::Color>> newGrid;
    
    // Ajouter les lignes vides en haut
    for (int i = 0; i < linesCleared; i++) {
        newGrid.push_back(std::vector<sf::Color>(width, sf::Color::Black));
    }
    
    // Copier les lignes qui ne sont pas effacées
    for (int i = 0; i < height; i++) {
        bool shouldClear = false;
        for (int line : linesToClear) {
            if (i == line) {
                shouldClear = true;
                break;
            }
        }
        if (!shouldClear) {
            newGrid.push_back(grid[i]);
        }
    }
    
    // Remplacer l'ancienne grille
    grid = std::move(newGrid);
    linesToClear.clear();
}


void Board::draw(sf::RenderWindow& window, int tileSize) {
    sf::RectangleShape block(sf::Vector2f(tileSize - 1, tileSize - 1));
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            if (grid[i][j] == sf::Color::Black) continue;
            block.setPosition(j * tileSize, i * tileSize);
            block.setFillColor(grid[i][j]);
            window.draw(block);
        }
    }
}

void Board::drawGrid(sf::RenderWindow& window, int tileSize) {
    sf::VertexArray lines(sf::Lines);
    sf::Color gridColor(50, 50, 50, 100);
    for (int x = 0; x <= width; x++) {
        lines.append(sf::Vertex(sf::Vector2f(x * tileSize, 0), gridColor));
        lines.append(sf::Vertex(sf::Vector2f(x * tileSize, height * tileSize), gridColor));
    }
    for (int y = 0; y <= height; y++) {
        lines.append(sf::Vertex(sf::Vector2f(0, y * tileSize), gridColor));
        lines.append(sf::Vertex(sf::Vector2f(width * tileSize, y * tileSize), gridColor));
    }
    window.draw(lines);
}

void Board::drawExplosion(sf::RenderWindow& window, int tileSize, float animTime) {
    if (linesToClear.empty()) return;

    sf::Color flash = (static_cast<int>(animTime * 10) % 2 == 0)
        ? sf::Color::Red : sf::Color::Yellow;

    sf::RectangleShape block(sf::Vector2f(tileSize - 1, tileSize - 1));
    block.setFillColor(flash);

    for (int line : linesToClear) {
        for (int j = 0; j < width; j++) {
            block.setPosition(j * tileSize, line * tileSize);
            window.draw(block);
        }
    }
}