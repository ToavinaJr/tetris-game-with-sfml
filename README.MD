Sure, here's a `README.md` for your project, designed to be clear, engaging, and informative.

-----

# Tetris Game

This is a classic Tetris game developed in C++ using the **SFML** multimedia library for graphics and user interaction. The project is built with **CMake** for cross-platform compatibility and documented using **Doxygen** for clear code understanding.

-----

## Features

  * Classic Tetris gameplay mechanics.
  * Smooth graphics and animations powered by SFML.
  * Intuitive controls.
  * Modular and well-structured C++ codebase.
  * Comprehensive Doxygen documentation for easy code navigation.

-----

## Getting Started

Follow these instructions to get a copy of the project up and running on your local machine.

### Prerequisites

Before you begin, ensure you have the following installed:

  * **C++ Compiler**: A C++ compatible compiler (e.g., GCC, Clang, MSVC).
  * **CMake**: Version 3.10 or higher.
      * [Download CMake](https://cmake.org/download/)
  * **SFML**: The Simple and Fast Multimedia Library.
      * [Download SFML](https://www.google.com/search?q=https://www.org/download/sfml/) (Choose the appropriate version for your compiler and OS).
      * Make sure SFML is correctly installed and its libraries are discoverable by CMake. On Linux, you might install it via your package manager (e.g., `sudo apt install libsfml-dev`).

### Building the Project

1. **Clone the repository:**

    ```bash
    git clone https://github.com/ToavinaJr/tetris-game-with-sfml
    cd tetris-game-with-sfml
    ```

2. **Create a build directory and navigate into it:**

    ```bash
    mkdir build
    cd build
    ```

3. **Configure the project with CMake:**

    ```bash
    cmake ..
    ```

    If CMake struggles to find SFML, you might need to provide a hint, for example:

    ```bash
    cmake -DCMAKE_PREFIX_PATH="/path/to/your/SFML" ..
    ```

4. **Build the project:**

    ```bash
    cmake --build .
    ```

    This command will compile the source code and create the executable.

### Running the Game

After a successful build, you'll find the executable in your `build` directory (or a subdirectory like `build/Debug` or `build/Release` depending on your OS and build type).

From the `build` directory:

```bash
# On Linux/macOS
./tetris # Or whatever your executable is named by CMake

# On Windows
.\tetris.exe
```

-----

## Project Structure

```bash
.
├── CMakeLists.txt          # CMake build configuration
├── docs                    # Generated Doxygen documentation (HTML, LaTeX)
│   ├── html                # Web-based documentation (open index.html)
│   └── latex               # LaTeX source for PDF documentation
├── Doxyfile                # Doxygen configuration file
├── includes                # Header files (.hpp) for class declarations
│   ├── Board.hpp
│   ├── Game.hpp
│   └── Tetromino.hpp
├── README.MD               # This documentation file
└── sources                 # Source files (.cpp) for class implementations
    ├── Board.cpp
    ├── Game.cpp
    ├── main.cpp
    └── Tetromino.cpp
```

-----

## Documentation

The project's code is thoroughly documented using **Doxygen**.

To generate or view the documentation:

1. Ensure you have Doxygen installed:
      * [Download Doxygen](https://www.doxygen.nl/download.html)
2. From the project root directory (where `Doxyfile` is located), run:

    ```bash
    doxygen Doxyfile
    ```

    This will regenerate the `docs/` directory.
3. Open `docs/html/index.html` in your web browser to explore the HTML documentation.

-----

## Contributing

Feel free to fork the repository, open issues, or submit pull requests. All contributions are welcome\!

-----
