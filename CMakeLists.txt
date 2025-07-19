cmake_minimum_required(VERSION 3.16)

#  Nom du projet et langage
project(TetrisSFML LANGUAGES CXX)

#  Version du standard C++ (C++23 ici)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

#  Ajouter les répertoires d'includes
include_directories(${CMAKE_SOURCE_DIR}/includes)

# Rechercher SFML (nécessite SFML installé)
find_package(SFML 2.5 COMPONENTS graphics window system REQUIRED)

#  Ajouter l'exécutable
add_executable(tetris
    sources/main.cpp
    sources/Game.cpp
    sources/Board.cpp
    sources/Tetromino.cpp
)

#  Lier SFML à l'exécutable
target_link_libraries(tetris sfml-graphics sfml-window sfml-system)

#  Optionnel : Activer plus d’avertissements en mode debug
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_options(tetris PRIVATE -Wall -Wextra -pedantic)
endif()
