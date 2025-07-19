#include "../includes/Board.hpp"
#include <algorithm>
#include <ranges>

/**
 * @brief Constructeur de la classe Board.
 * 
 * Initialise la grille de jeu avec la largeur et la hauteur spécifiées,
 * en remplissant toutes les cases avec la couleur noire (vide).
 * 
 * @param w Largeur de la grille (en nombre de blocs).
 * @param h Hauteur de la grille (en nombre de blocs).
 */
Board::Board(int w, int h)
    : width(w), height(h),
      grid(h, std::vector<sf::Color>(w, sf::Color::Black)) 
{}

/**
 * @brief Vérifie si un Tetromino entre en collision avec la grille ou les bords.
 * 
 * @param tetro Le Tetromino à vérifier.
 * @return true si une collision est détectée.
 * @return false sinon.
 */
bool Board::checkCollision(const Tetromino& tetro) const 
{
    for (auto& b : tetro.getBlocks()) 
    {
        // Collision avec les bords ou le bas de la grille
        if (b.x < 0 || b.x >= width || b.y >= height) 
            return true;

        // Collision avec des blocs déjà posés
        if (b.y >= 0 && grid[b.y][b.x] != sf::Color::Black) 
            return true;
    }
    return false;
}

/**
 * @brief Fusionne un Tetromino avec la grille.
 * 
 * Place la couleur du Tetromino dans la grille aux positions occupées par ses blocs.
 * 
 * @param tetro Le Tetromino à fusionner.
 */
void Board::mergeTetromino(const Tetromino& tetro) 
{
    for (auto& b : tetro.getBlocks()) 
    {
        if (b.y >= 0 && b.y < height) 
        {
            grid[b.y][b.x] = tetro.getColor();
        }
    }
}

/**
 * @brief Détecte les lignes complètes à effacer.
 * 
 * Parcourt la grille et ajoute les indices des lignes totalement remplies
 * à la liste `linesToClear`.
 */
void Board::detectLinesToClear() 
{
    linesToClear.clear();
    for (int i = 0; i < height; i++) 
    {   
        auto checkColor = [](const sf::Color& c) { return c != sf::Color::Black; };
        if (std::ranges::all_of(grid[i], checkColor)) 
        {
            linesToClear.push_back(i);
        }
    }
}

/**
 * @brief Efface les lignes complètes détectées et compresse la grille.
 * 
 * Les lignes complètes sont supprimées et remplacées par des lignes vides en haut de la grille.
 */
void Board::performClearLines() 
{
    // Trier les lignes par ordre décroissant pour éviter les problèmes d'indices
    std::sort(linesToClear.rbegin(), linesToClear.rend());

    int linesCleared = linesToClear.size();
    
    std::vector<std::vector<sf::Color>> newGrid;

    // Ajouter les lignes vides en haut
    for (int i = 0; i < linesCleared; i++) 
    {
        newGrid.push_back(std::vector<sf::Color>(width, sf::Color::Black));
    }
    
    // Copier les lignes restantes (non effacées)
    for (int i = 0; i < height; i++) 
    {
        bool shouldClear = false;
        for (const int& line : linesToClear) 
        {
            if (i == line) 
            {
                shouldClear = true;
                break;
            }
        }
        
        if (!shouldClear) 
        {
            newGrid.push_back(grid[i]);
        }
    }
    
    grid = std::move(newGrid);
    linesToClear.clear();
}

/**
 * @brief Dessine tous les blocs présents dans la grille.
 * 
 * @param window La fenêtre SFML où dessiner.
 * @param tileSize La taille (en pixels) de chaque bloc.
 */
void Board::draw(sf::RenderWindow& window, int tileSize) 
{
    sf::RectangleShape block(sf::Vector2f(tileSize - 1, tileSize - 1));

    for (int i = 0; i < height; i++) 
    {
        for (int j = 0; j < width; j++) 
        {
            if (grid[i][j] == sf::Color::Black) continue;
            block.setPosition(j * tileSize, i * tileSize);
            block.setFillColor(grid[i][j]);
            window.draw(block);
        }
    }
}

/**
 * @brief Dessine la grille en arrière-plan (lignes grises).
 * 
 * @param window La fenêtre SFML où dessiner.
 * @param tileSize La taille d’un bloc (en pixels).
 */
void Board::drawGrid(sf::RenderWindow& window, int tileSize) 
{
    sf::VertexArray lines(sf::Lines);
    sf::Color gridColor(50, 50, 50, 100);
    
    for (int x = 0; x <= width; x++) 
    {
        lines.append(sf::Vertex(sf::Vector2f(x * tileSize, 0), gridColor));
        lines.append(sf::Vertex(sf::Vector2f(x * tileSize, height * tileSize), gridColor));
    }

    for (int y = 0; y <= height; y++) 
    {
        lines.append(sf::Vertex(sf::Vector2f(0, y * tileSize), gridColor));
        lines.append(sf::Vertex(sf::Vector2f(width * tileSize, y * tileSize), gridColor));
    }
    
    window.draw(lines);
}

/**
 * @brief Dessine une animation d'explosion sur les lignes à effacer.
 * 
 * Les lignes clignotent alternativement en rouge et jaune avant leur suppression.
 * 
 * @param window La fenêtre SFML où dessiner.
 * @param tileSize La taille des blocs.
 * @param animTime Le temps écoulé (utilisé pour alterner les couleurs).
 */
void Board::drawExplosion(sf::RenderWindow& window, int tileSize, float animTime) 
{
    if (linesToClear.empty()) return;

    sf::Color flash = (static_cast<int>(animTime * 10) % 2 == 0)
        ? sf::Color::Red : sf::Color::Yellow;

    sf::RectangleShape block(sf::Vector2f(tileSize - 1, tileSize - 1));
    block.setFillColor(flash);

    for (int line : linesToClear) 
    {
        for (int j = 0; j < width; j++) 
        {
            block.setPosition(j * tileSize, line * tileSize);
            window.draw(block);
        }
    }
}
