Sudoku Game (C++ & SFML)

This project is a fully interactive Sudoku game built in C++ using SFML for graphics, audio, and user interaction.
It includes a graphical grid, smooth UI buttons, sound effects, and a tutorial system.

Note:
Although SQLite files (sqlite3.c, sqlite3.h, mydatabase.db) are included in the repository, the current version of the game does not use a database. All gameplay runs entirely in memory.

 Features

* 9×9 Sudoku gameplay

* Mouse-based input

* Click sound effects

* SFML-based GUI

* Automatic number validation

* In-built tutorial screen

* Works on Windows, Linux, and macOS

📂 Project Structure
finalsudoku.cpp       → Main game source code
click.wav             → Sound effect for button clicks
arial.ttf             → Font used for SFML text
sqlite3.c/h           → SQLite library (not used in current version)
mydatabase.db         → Database file (not used)


You may safely remove:
extra unused font files

 How to Run the Game
🪟 Windows
1. Install Requirements

SFML 2.5+

C++ compiler (MinGW-w64 or Visual Studio)

2. Make Sure These Files Are Together
finalsudoku.cpp
sqlite3.c
sqlite3.h
click.wav
arial.ttf   (or whichever font you use)


(The DB files are optional because they are unused.)

3. Compile (MinGW Example)
g++ finalsudoku.cpp sqlite3.c -I C:\SFML\include -L C:\SFML\lib \
-lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio -o sudoku.exe


Copy the required SFML DLLs from C:\SFML\bin into your folder.

4. Run
sudoku.exe

🐧= Linux
Install SFML
sudo apt update
sudo apt install libsfml-dev

Compile
g++ finalsudoku.cpp sqlite3.c -lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio -o sudoku

Run
./sudoku

🍏 macOS (Intel & M1/M2)
Install SFML (via Homebrew)
brew install sfml

Compile
g++ finalsudoku.cpp sqlite3.c \
-I /opt/homebrew/include \
-L /opt/homebrew/lib \
-lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio \
-o sudoku

Run
./sudoku


If macOS blocks it:

sudo xattr -rd com.apple.quarantine ./sudoku

Gameplay Overview

The game loads a 9×9 Sudoku grid.

Click any cell to select it.

Use number keys 1–9 to fill values.

The program validates your move:

no row conflicts

no column conflicts

no 3×3 grid conflicts

A clear button resets the current selection.

A tutorial screen explains how to play.

 Notes

SQLite is not used by the current code.

You may delete unused database and extra font files.

The game is fully self-contained and runs without external dependencies except SFML.


Built for academic & project work using C++ and SFML.
