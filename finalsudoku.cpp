#include <iostream>
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include <stdexcept>
#include <ctime>
#include <cstdlib>
#include <cstring>
#include <sqlite3.h>
#include <SFML/Audio.hpp>
using namespace std;

enum class Screen {
    Login,
    Dashboard,
    CreateAccount
};
//for games
class Game {
    const int CELL_SIZE = 60;
    const int GRID_SIZE = 9;

    vector<vector<vector<int>>> solutions;
    int grid[9][9] = {0};
    int puzzle[9][9] = {0};
    int userGrid[9][9] = {0};
    int solutionsFound = 0;

public:
    bool isValid(int g[9][9], int row, int col, int num) {
        for (int x = 0; x < 9; ++x)
            if (g[row][x] == num || g[x][col] == num)
                return false;

        int startRow = row - row % 3, startCol = col - col % 3;
        for (int i = 0; i < 3; ++i)
            for (int j = 0; j < 3; ++j)
                if (g[startRow + i][startCol + j] == num)
                    return false;

        return true;
    }

    bool solve(int g[9][9]) {
        for (int row = 0; row < 9; ++row) {
            for (int col = 0; col < 9; ++col) {
                if (g[row][col] == 0) {
                    for (int num = 1; num <= 9; ++num) {
                        if (isValid(g, row, col, num)) {
                            g[row][col] = num;
                            if (solve(g)) return true;// recursive call to solve the next cell
                            g[row][col] = 0;
                        }
                    }
                    return false;
                }
            }
        }
        return true;
    }

    void countSolutions(int g[9][9]) {
        for (int row = 0; row < 9; ++row) {
            for (int col = 0; col < 9; ++col) {
                if (g[row][col] == 0) {
                    for (int num = 1; num <= 9; ++num) {
                        if (isValid(g, row, col, num)) {
                            g[row][col] = num;
                            countSolutions(g);// recursive call to count solutions
                            g[row][col] = 0;
                        }
                    }
                    return;
                }
            }
        }
        vector<vector<int>> sol(9, vector<int>(9));
        for (int i = 0; i < 9; ++i)
            for (int j = 0; j < 9; ++j)
                sol[i][j] = g[i][j];
        solutions.push_back(sol);
        ++solutionsFound;
    }

    void drawGrid(sf::RenderWindow& window, sf::Font& font) {
        for (int i = 0; i <= GRID_SIZE; ++i) {
            sf::RectangleShape line(sf::Vector2f(GRID_SIZE * CELL_SIZE, i % 3 == 0 ? 3 : 1));
            line.setPosition(0, i * CELL_SIZE);
            line.setFillColor(sf::Color::White);
            window.draw(line);

            line.setSize(sf::Vector2f(i % 3 == 0 ? 3 : 1, GRID_SIZE * CELL_SIZE));
            line.setPosition(i * CELL_SIZE, 0);
            window.draw(line);
        }

        for (int row = 0; row < 9; ++row) {
            for (int col = 0; col < 9; ++col) {
                if (userGrid[row][col] != 0) {
                    sf::Text text;
                    text.setFont(font);
                    text.setString(to_string(userGrid[row][col]));
                    text.setCharacterSize(30);
                    text.setFillColor(sf::Color::Cyan);
                    text.setPosition(col * CELL_SIZE + 20, row * CELL_SIZE + 10);
                    window.draw(text);
                }
            }
        }
    }

    void playSudokuWithDifficulty(int difficulty) {
        srand(time(0));

        int removedCells = (difficulty == 1) ? 30 : (difficulty == 2) ? 40 : 50;

        memset(grid, 0, sizeof(grid));
        solve(grid);
        memcpy(puzzle, grid, sizeof(grid));

        int removed = 0;
        while (removed < removedCells) {
            int r = rand() % 9, c = rand() % 9;
            if (puzzle[r][c] != 0) {
                int backup = puzzle[r][c];
                puzzle[r][c] = 0;
                int temp[9][9];
                memcpy(temp, puzzle, sizeof(puzzle));
                solutions.clear();
                solutionsFound = 0;
                countSolutions(temp);
                if (solutionsFound > 3)
                    puzzle[r][c] = backup;
                else
                    ++removed;
            }
        }
        memcpy(userGrid, puzzle, sizeof(puzzle));

        sf::RenderWindow window(sf::VideoMode(GRID_SIZE * CELL_SIZE, GRID_SIZE * CELL_SIZE + 60), "Sudoku Play");
        sf::Font font;
        if (!font.loadFromFile("arial.ttf")) {
            cerr << "Font load error\n";
            return;
        }

        int selectedRow = 0, selectedCol = 0;
        bool submitted = false;
        bool correctGrid[9][9] = {false};

        while (window.isOpen()) {
            sf::Event event;
            while (window.pollEvent(event)) {
                if (event.type == sf::Event::Closed)
                    window.close();

                if (event.type == sf::Event::KeyPressed && !submitted) {
                    if (event.key.code >= sf::Keyboard::Num1 && event.key.code <= sf::Keyboard::Num9) {
                        if (puzzle[selectedRow][selectedCol] == 0)
                            userGrid[selectedRow][selectedCol] = event.key.code - sf::Keyboard::Num0;
                    } else if (event.key.code == sf::Keyboard::BackSpace) {
                        if (puzzle[selectedRow][selectedCol] == 0)
                            userGrid[selectedRow][selectedCol] = 0;
                    } else if (event.key.code == sf::Keyboard::Enter) {
                        bool allFilled = true;
                        for (int i = 0; i < 9 && allFilled; ++i)
                            for (int j = 0; j < 9 && allFilled; ++j)
                                if (userGrid[i][j] == 0)
                                    allFilled = false;

                        if (!allFilled) {
                            cout << "Please fill all cells before submitting.\n";
                        } else {
                            int temp[9][9];
                            memcpy(temp, userGrid, sizeof(userGrid));
                            if (memcmp(temp, grid, sizeof(grid)) == 0) {
                                cout << "Correct solution!\n";
                                submitted = true;
                            } else {
                                cout << "Incorrect solution. Showing possible solutions:\n";
                                int tempGrid[9][9];
                                memcpy(tempGrid, puzzle, sizeof(puzzle));
                                solutions.clear();
                                solutionsFound = 0;
                                countSolutions(tempGrid);

                                for (int s = 0; s < solutions.size() && s < 3; ++s) {
                                    cout << "Solution " << s + 1 << ":\n";
                                    for (int i = 0; i < 9; ++i) {
                                        for (int j = 0; j < 9; ++j)
                                            cout << solutions[s][i][j] << " ";
                                        cout << "\n";
                                    }
                                }

                                for (int i = 0; i < 9; ++i)
                                    for (int j = 0; j < 9; ++j)
                                        if (puzzle[i][j] == 0)
                                            correctGrid[i][j] = (userGrid[i][j] == solutions[0][i][j]);
                                submitted = true;
                            }
                        }
                    }
                    if (event.key.code == sf::Keyboard::Right) selectedCol = (selectedCol + 1) % 9;
                    if (event.key.code == sf::Keyboard::Left) selectedCol = (selectedCol + 8) % 9;
                    if (event.key.code == sf::Keyboard::Down) selectedRow = (selectedRow + 1) % 9;
                    if (event.key.code == sf::Keyboard::Up) selectedRow = (selectedRow + 8) % 9;
                }
            }

            window.clear();
            drawGrid(window, font);

            if (submitted) {
                for (int row = 0; row < 9; ++row) {
                    for (int col = 0; col < 9; ++col) {
                        if (puzzle[row][col] == 0) {
                            sf::RectangleShape cell(sf::Vector2f(CELL_SIZE, CELL_SIZE));
                            cell.setPosition(col * CELL_SIZE, row * CELL_SIZE);
                            cell.setFillColor(correctGrid[row][col] ? sf::Color(0, 255, 0, 100) : sf::Color(255, 0, 0, 100));
                            window.draw(cell);
                        }
                    }
                }
            }

            sf::RectangleShape selector(sf::Vector2f(CELL_SIZE, CELL_SIZE));
            selector.setPosition(selectedCol * CELL_SIZE, selectedRow * CELL_SIZE);
            selector.setFillColor(sf::Color(255, 255, 255, 50));
            window.draw(selector);

            window.display();
        }
    }

    int play() {
    sf::RenderWindow window(sf::VideoMode(500, 300), "Select Difficulty");
    window.setFramerateLimit(60);
    window.setActive();

    sf::Font font;
    if (!font.loadFromFile("arial.ttf")) {
        cerr << "Font load error\n";
        return 1;
    }

    // Load click sound
    sf::SoundBuffer buffer;
    if (!buffer.loadFromFile("click.wav")) {
        cerr << "Sound file not found\n";
    }
    sf::Sound clickSound;
    clickSound.setBuffer(buffer);

    vector<string> difficulties = {"Easy", "Medium", "Hard"};
    vector<sf::RectangleShape> buttons;
    vector<sf::Text> labels;

    const int buttonWidth = 200, buttonHeight = 50;
    const sf::Color normalColor(34, 139, 34);
    const sf::Color hoverColor(60, 179, 113);

    for (int i = 0; i < difficulties.size(); ++i) {
        sf::RectangleShape button(sf::Vector2f(buttonWidth, buttonHeight));
        button.setPosition(150, 60 + i * 70);
        button.setFillColor(normalColor);
        buttons.push_back(button);

        sf::Text label;
        label.setFont(font);
        label.setString(difficulties[i]);
        label.setCharacterSize(24);
        label.setFillColor(sf::Color::White);
        label.setPosition(150 + 40, 60 + i * 70 + 10);
        labels.push_back(label);
    }

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();

            if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
                sf::Vector2i mousePos = sf::Mouse::getPosition(window);
                sf::Vector2f mouse(mousePos.x, mousePos.y);
                for (int i = 0; i < buttons.size(); ++i) {
                    if (buttons[i].getGlobalBounds().contains(mouse)) {
                        clickSound.play();
                        window.close();
                        return i + 1;  // 1 = Easy, 2 = Medium, 3 = Hard
                    }
                }
            }
        }

        // Hover highlight
        sf::Vector2i mousePos = sf::Mouse::getPosition(window);
        sf::Vector2f mouse(mousePos.x, mousePos.y);
        for (int i = 0; i < buttons.size(); ++i)
            buttons[i].setFillColor(buttons[i].getGlobalBounds().contains(mouse) ? hoverColor : normalColor);

        window.clear(sf::Color(20, 20, 40));
        for (int i = 0; i < buttons.size(); ++i) {
            window.draw(buttons[i]);
            window.draw(labels[i]);
        }
        window.display();
    }

    return 1; // Default to Easy if something goes wrong
}

    int solver() {
        memset(userGrid, 0, sizeof(userGrid));
        sf::RenderWindow window(sf::VideoMode(GRID_SIZE * CELL_SIZE, GRID_SIZE * CELL_SIZE + 60), "Sudoku Solver");
        sf::Font font;
        if (!font.loadFromFile("arial.ttf")) {
            cerr << "Font load error\n";
            return 0;
        }

        int selectedRow = 0, selectedCol = 0;
        bool submitted = false;

        while (window.isOpen()) {
            sf::Event event;
            while (window.pollEvent(event)) {
                if (event.type == sf::Event::Closed)
                    window.close();

                if (event.type == sf::Event::KeyPressed && !submitted) {
                    if (event.key.code >= sf::Keyboard::Num1 && event.key.code <= sf::Keyboard::Num9) {
                        if (isValid(userGrid, selectedRow, selectedCol, event.key.code - sf::Keyboard::Num0))
                            userGrid[selectedRow][selectedCol] = event.key.code - sf::Keyboard::Num0;
                    } else if (event.key.code == sf::Keyboard::BackSpace) {
                        userGrid[selectedRow][selectedCol] = 0;
                    } else if (event.key.code == sf::Keyboard::Enter) {
                        int temp[9][9];
                        memcpy(temp, userGrid, sizeof(userGrid));
                        solutions.clear();
                        solutionsFound = 0;
                        countSolutions(temp);

                        if (solutionsFound == 0) {
                            cout << "Invalid or unsolvable board.\n";
                        } else {
                            cout << "Top " << min(3, solutionsFound) << " solutions:\n";
                            for (int s = 0; s < solutions.size() && s < 3; ++s) {
                                cout << "Solution " << s + 1 << ":\n";
                                for (int i = 0; i < 9; ++i) {
                                    for (int j = 0; j < 9; ++j)
                                        cout << solutions[s][i][j] << " ";
                                    cout << "\n";
                                }
                            }
                            submitted = true;
                        }
                    }
                    if (event.key.code == sf::Keyboard::Right) selectedCol = (selectedCol + 1) % 9;
                    if (event.key.code == sf::Keyboard::Left) selectedCol = (selectedCol + 8) % 9;
                    if (event.key.code == sf::Keyboard::Down) selectedRow = (selectedRow + 1) % 9;
                    if (event.key.code == sf::Keyboard::Up) selectedRow = (selectedRow + 8) % 9;
                }
            }

            window.clear();
            drawGrid(window, font);

            sf::RectangleShape selector(sf::Vector2f(CELL_SIZE, CELL_SIZE));
            selector.setPosition(selectedCol * CELL_SIZE, selectedRow * CELL_SIZE);
            selector.setFillColor(sf::Color(255, 255, 255, 50));
            window.draw(selector);

            window.display();
        }

        return 0;
    }
#include <SFML/Graphics.hpp>
#include <iostream>
#include <string>
#include <vector>

// Draw the Sudoku grid lines
void drawGrid(sf::RenderWindow& window) {
    const int gridSize = 450; // grid 450x450 px
    const int cellSize = gridSize / 9;
    const int offsetX = 75, offsetY = 20;

    sf::RectangleShape line;
    line.setFillColor(sf::Color::White);

    // Draw thin lines (cells)
    for (int i = 0; i <= 9; i++) {
        // vertical lines
        line.setSize(sf::Vector2f(1, gridSize));
        line.setPosition(offsetX + i * cellSize, offsetY);
        window.draw(line);

        // horizontal lines
        line.setSize(sf::Vector2f(gridSize, 1));
        line.setPosition(offsetX, offsetY + i * cellSize);
        window.draw(line);
    }

    // Draw thicker lines for 3x3 blocks
    line.setFillColor(sf::Color::White);
    for (int i = 0; i <= 9; i += 3) {
        // vertical thick lines
        line.setSize(sf::Vector2f(3, gridSize + 1));
        line.setPosition(offsetX + i * cellSize, offsetY - 1);
        window.draw(line);

        // horizontal thick lines
        line.setSize(sf::Vector2f(gridSize + 1, 3));
        line.setPosition(offsetX - 1, offsetY + i * cellSize);
        window.draw(line);
    }
}

// Highlight cells based on indexes with color
void highlightCells(sf::RenderWindow& window, const std::vector<sf::Vector2i>& cells, sf::Color color) {
    const int gridSize = 450;
    const int cellSize = gridSize / 9;
    const int offsetX = 75, offsetY = 20;

    sf::RectangleShape highlight(sf::Vector2f(cellSize, cellSize));
    highlight.setFillColor(color);

    for (const auto& c : cells) {
        highlight.setPosition(offsetX + c.x * cellSize + 1, offsetY + c.y * cellSize + 1);
        window.draw(highlight);
    }
}

// Draw Sudoku numbers inside the grid
void drawNumbers(sf::RenderWindow& window, const int board[9][9], const sf::Font& font) {
    const int gridSize = 450;
    const int cellSize = gridSize / 9;
    const int offsetX = 75, offsetY = 20;

    sf::Text numText;
    numText.setFont(font);
    numText.setCharacterSize(cellSize - 8);
    numText.setFillColor(sf::Color::White);

    for (int row = 0; row < 9; row++) {
        for (int col = 0; col < 9; col++) {
            if (board[row][col] != 0) {
                numText.setString(std::to_string(board[row][col]));
                // Center text in cell
                sf::FloatRect bounds = numText.getLocalBounds();
                float xPos = offsetX + col * cellSize + (cellSize - bounds.width) / 2 - bounds.left;
                float yPos = offsetY + row * cellSize + (cellSize - bounds.height) / 2 - bounds.top - 5;
                numText.setPosition(xPos, yPos);
                window.draw(numText);
            }
        }
    }
}

void tutorial() {
    sf::RenderWindow window(sf::VideoMode(600, 600), "Sudoku Tutorial");
    window.setFramerateLimit(60);

    sf::Font font;
    if (!font.loadFromFile("arial.ttf")) {
        std::cerr << "Failed to load font\n";
        return;
    }

    // Sample Sudoku puzzle (0 = empty)
    int board[9][9] = {
        {5, 3, 0, 0, 7, 0, 0, 0, 0},
        {6, 0, 0, 1, 9, 5, 0, 0, 0},
        {0, 9, 8, 0, 0, 0, 0, 6, 0},
        {8, 0, 0, 0, 6, 0, 0, 0, 3},
        {4, 0, 0, 8, 0, 3, 0, 0, 1},
        {7, 0, 0, 0, 2, 0, 0, 0, 6},
        {0, 6, 0, 0, 0, 0, 2, 8, 0},
        {0, 0, 0, 4, 1, 9, 0, 0, 5},
        {0, 0, 0, 0, 8, 0, 0, 7, 9}
    };

    // Sound effect setup (optional)
    sf::SoundBuffer buffer;
    if (!buffer.loadFromFile("click.wav")) {
        std::cerr << "Sound file not found\n";
    }
    sf::Sound clickSound;
    clickSound.setBuffer(buffer);

    // Buttons
    sf::RectangleShape backButton(sf::Vector2f(100, 40));
    backButton.setPosition(75, 550);
    backButton.setFillColor(sf::Color(34, 139, 34));

    sf::Text backLabel("Back", font, 24);
    backLabel.setFillColor(sf::Color::White);
    backLabel.setPosition(115, 555);

    sf::RectangleShape nextButton(sf::Vector2f(100, 40));
    nextButton.setPosition(425, 550);
    nextButton.setFillColor(sf::Color(34, 139, 34));

    sf::Text nextLabel("Next", font, 24);
    nextLabel.setFillColor(sf::Color::White);
    nextLabel.setPosition(470, 555);

    int step = 0; // 0 = row, 1 = column, 2 = block

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) window.close();

            if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
                sf::Vector2i mousePos = sf::Mouse::getPosition(window);
                sf::Vector2f mouseF(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y));

                if (backButton.getGlobalBounds().contains(mouseF)) {
                    clickSound.play();
                    window.close();
                } else if (nextButton.getGlobalBounds().contains(mouseF)) {
                    clickSound.play();
                    step = (step + 1) % 3;
                }
            }
        }

        // Hover effect for buttons
        sf::Vector2i mousePos = sf::Mouse::getPosition(window);
        sf::Vector2f mouseF(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y));

        backButton.setFillColor(backButton.getGlobalBounds().contains(mouseF) ? sf::Color(60, 179, 113) : sf::Color(34, 139, 34));
        nextButton.setFillColor(nextButton.getGlobalBounds().contains(mouseF) ? sf::Color(60, 179, 113) : sf::Color(34, 139, 34));

        // Background and grid
        window.clear(sf::Color(20, 20, 40));
        drawGrid(window);

        // Highlight depending on step
        std::string explanation;
        switch (step) {
            case 0: {
                // Highlight row 4 (index 3)
                std::vector<sf::Vector2i> rowCells;
                for (int i = 0; i < 9; ++i) rowCells.push_back({i, 3});
                highlightCells(window, rowCells, sf::Color(255, 165, 0, 120)); // Orange translucent
                explanation = "Row Explanation:\nEach number 1-9 must appear only once in each row.";
                break;
            }
            case 1: {
                // Highlight column 5 (index 4)
                std::vector<sf::Vector2i> colCells;
                for (int i = 0; i < 9; ++i) colCells.push_back({4, i});
                highlightCells(window, colCells, sf::Color(0, 128, 255, 120)); // Blue translucent
                explanation = "Column Explanation:\nEach number 1-9 must appear only once in each column.";
                break;
            }
            case 2: {
                // Highlight block 3x3 starting at (6,6) bottom-right block
                std::vector<sf::Vector2i> blockCells;
                for (int i = 6; i < 9; ++i) {
                    for (int j = 6; j < 9; ++j) {
                        blockCells.push_back({i, j});
                    }
                }
                highlightCells(window, blockCells, sf::Color(34, 139, 34, 120)); // Green translucent
                explanation = "Block Explanation:\nEach 3x3 block must contain unique numbers 1-9.";
                break;
            }
        }

        drawNumbers(window, board, font);

        // Draw explanation text in a box below grid
        sf::RectangleShape textBox(sf::Vector2f(550, 90));
        textBox.setPosition(25, 460);
        textBox.setFillColor(sf::Color(40, 40, 70, 200));
        textBox.setOutlineColor(sf::Color::White);
        textBox.setOutlineThickness(1);
        window.draw(textBox);

        sf::Text explanationText;
        explanationText.setFont(font);
        explanationText.setString(explanation);
        explanationText.setCharacterSize(20);
        explanationText.setFillColor(sf::Color::White);
        explanationText.setPosition(35, 470);

        window.draw(explanationText);
        window.draw(backButton);
        window.draw(backLabel);
        window.draw(nextButton);
        window.draw(nextLabel);

        window.display();
    }
}

    bool mainmenu() {
    sf::RenderWindow window(sf::VideoMode(600, 400), "Sudoku Main Menu");
    window.setFramerateLimit(60);
    window.setActive();

    sf::Font font;
    if (!font.loadFromFile("arial.ttf")) {
        cerr << "Font load error\n";
        return true;
    }

    // Load click sound
    sf::SoundBuffer buffer;
    if (!buffer.loadFromFile("click.wav")) {
        cerr << "Sound file not found\n";
    }
    sf::Sound clickSound;
    clickSound.setBuffer(buffer);

    // Button setup
    const int buttonWidth = 300, buttonHeight = 50;
    const sf::Color normalColor(70, 130, 180);
    const sf::Color hoverColor(100, 149, 237);

    vector<string> options = {"Play Sudoku", "Tutorial", "Sudoku Solver", "Exit"};
    vector<sf::RectangleShape> buttons;
    vector<sf::Text> labels;

    for (size_t i = 0; i < options.size(); ++i) {
        sf::RectangleShape button(sf::Vector2f(buttonWidth, buttonHeight));
        button.setPosition(150, 80 + i * 70);
        button.setFillColor(normalColor);
        buttons.push_back(button);

        sf::Text label;
        label.setFont(font);
        label.setString(options[i]);
        label.setCharacterSize(24);
        label.setFillColor(sf::Color::White);
        label.setPosition(150 + 20, 80 + i * 70 + 10);
        labels.push_back(label);
    }

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();

            if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
                sf::Vector2i mousePos = sf::Mouse::getPosition(window);
                sf::Vector2f mouse(mousePos.x, mousePos.y);  // Correct conversion
                for (int i = 0; i < buttons.size(); ++i) {
                    if (buttons[i].getGlobalBounds().contains(mouse)) {
                        clickSound.play();
                        window.close();

                        switch (i) {
                            case 0: {
                                int diff = play();  // Show difficulty selection
                                playSudokuWithDifficulty(diff);
                                break;
                            }
                            case 1:
                                tutorial();
                                break;
                            case 2:
                                solver();
                                break;
                            case 3:
                                return false;
                        }
                    }
                }
            }
        }

        // Hover color update
        sf::Vector2i mousePos = sf::Mouse::getPosition(window);
        sf::Vector2f mouse(mousePos.x, mousePos.y);
        for (int i = 0; i < buttons.size(); ++i)
            buttons[i].setFillColor(buttons[i].getGlobalBounds().contains(mouse) ? hoverColor : normalColor);

        window.clear(sf::Color(30, 30, 60));
        for (int i = 0; i < buttons.size(); ++i) {
            window.draw(buttons[i]);
            window.draw(labels[i]);
        }
        window.display();
    }
    return true;
}

};

int main() {
sf::RenderWindow window(sf::VideoMode(600, 500), "SudoMaster Login");
    // Load font
    sf::Font font;
    if (!font.loadFromFile("arial.ttf")) {
        std::cerr << "Failed to load font.\n";
        return -1;
    }
    Screen currentScreen = Screen::Login;
    // --- Login UI ---

    sf::Text title("SudoMaster", font, 36);
    title.setFillColor(sf::Color(75, 0, 130)); // Dark purple
    title.setPosition(190, 40);

    sf::Text subtitle("Welcome Back!", font, 26);
    subtitle.setFillColor(sf::Color(75, 0, 130)); // Dark purple
    subtitle.setPosition(180, 90);

    sf::RectangleShape usernameBox(sf::Vector2f(350, 50));
    usernameBox.setFillColor(sf::Color(255, 248, 235));
    usernameBox.setPosition(125, 150);

    sf::Texture usernameIcon;
    sf::Sprite usernameIconimage;
    // Load the texture (check if successful)
if (!usernameIcon.loadFromFile("username.jpeg")) {
    cerr << "Failed to load username icon image\n";
    // handle error
}
usernameIconimage.setTexture(usernameIcon);
float iconWidth = 30.f;
float iconHeight = 30.f;

// Get the original size of the texture
sf::Vector2u texSize = usernameIcon.getSize();

// Calculate scale factors
float scaleX = iconWidth / texSize.x;
float scaleY = iconHeight / texSize.y;

// Apply scaling (keep aspect ratio if you want)
// Here scaling proportionally to iconHeight:
float scale = min(scaleX, scaleY);
usernameIconimage.setScale(scale, scale);

// Position icon inside the username box with some padding
float paddingLeft = 10.f;
float boxPosX = usernameBox.getPosition().x;
float boxPosY = usernameBox.getPosition().y;

// Vertically center icon inside box:
float iconPosX = boxPosX + paddingLeft;
float iconPosY = boxPosY + (usernameBox.getSize().y - iconHeight) / 2.f;

usernameIconimage.setPosition(iconPosX, iconPosY);


    // We'll use inputUsernameText for actual text shown
    sf::Text inputUsername("", font, 20);
    inputUsername.setFillColor(sf::Color::Black);
    inputUsername.setPosition(170, 165);

    sf::RectangleShape passwordBox(sf::Vector2f(350, 50));
    passwordBox.setFillColor(sf::Color(255, 248, 235));
    passwordBox.setPosition(125, 220);

    sf::Texture passwordIcon;
    sf::Sprite passwordIconimage;
    if (!passwordIcon.loadFromFile("password.png")) {
    cerr << "Failed to load username icon image\n";
    // handle error
}

passwordIconimage.setTexture(passwordIcon);
float piconWidth = 30.f;
float piconHeight = 30.f;
// Get the original size of the texture
sf::Vector2u ptexSize = passwordIcon.getSize();

// Calculate scale factors
float pscaleX = piconWidth / ptexSize.x;
float pscaleY = piconHeight / ptexSize.y;

// Apply scaling (keep aspect ratio if you want)
// Here scaling proportionally to iconHeight:
float pscale = min(pscaleX, pscaleY);
passwordIconimage.setScale(pscale, pscale);

// Position icon inside the username box with some padding
float ppaddingLeft = 10.f;
float pboxPosX = passwordBox.getPosition().x;
float pboxPosY = passwordBox.getPosition().y;

// Vertically center icon inside box:
float piconPosX = pboxPosX + ppaddingLeft;
float piconPosY = pboxPosY + (passwordBox.getSize().y - piconHeight) / 2.f;

passwordIconimage.setPosition(piconPosX, piconPosY);


    sf::Text inputPassword("", font, 20);
    inputPassword.setFillColor(sf::Color::Black);
    inputPassword.setPosition(170, 235);

    sf::RectangleShape loginButton(sf::Vector2f(350, 50));
    loginButton.setFillColor(sf::Color(255, 140, 0)); // Orange
    loginButton.setPosition(125, 290);

    sf::Text loginText("Log In", font, 22);
    loginText.setFillColor(sf::Color::White);
    loginText.setPosition(260, 300);

    sf::Text CreateText("Create an account", font, 18);
   CreateText.setFillColor(sf::Color::Blue);
CreateText.setStyle(sf::Text::Underlined);
    CreateText.setPosition(220, 355);

    sf::Text loginErrorMessage("", font, 18);
    loginErrorMessage.setFillColor(sf::Color::Red);
    loginErrorMessage.setPosition(200, 400);
    // Input strings
    std::string username = "";
    std::string password = "";
    int typingField=0;

     // ---------- CREATE ACCOUNT ELEMENTS ----------
    sf::RectangleShape caUsernameBox(sf::Vector2f(350, 50));
    caUsernameBox.setFillColor(sf::Color(255, 248, 235));
    caUsernameBox.setPosition(125, 100);

    sf::RectangleShape caPasswordBox(sf::Vector2f(350, 50));
    caPasswordBox.setFillColor(sf::Color(255, 248, 235));
    caPasswordBox.setPosition(125, 170);

    sf::RectangleShape caConfirmPasswordBox(sf::Vector2f(350, 50));
    caConfirmPasswordBox.setFillColor(sf::Color(255, 248, 235));
    caConfirmPasswordBox.setPosition(125, 240);

    sf::Text caUsernameText("New username", font, 20);
    caUsernameText.setFillColor(sf::Color::Black);
    caUsernameText.setPosition(140, 115);

    sf::Text caPasswordText("Password", font, 20);
    caPasswordText.setFillColor(sf::Color::Black);
    caPasswordText.setPosition(140, 185);

    sf::Text caConfirmPasswordText("Confirm password", font, 20);
    caConfirmPasswordText.setFillColor(sf::Color::Black);
    caConfirmPasswordText.setPosition(140, 255);

    sf::RectangleShape caCreateButton(sf::Vector2f(350, 50));
    caCreateButton.setFillColor(sf::Color(100, 149, 237));
    caCreateButton.setPosition(125, 310);

    sf::Text caCreateText("Create Account", font, 22);
    caCreateText.setFillColor(sf::Color::White);
    caCreateText.setPosition(200, 320);

    string caUsername = "", caPassword = "", caConfirmPassword = "";
    int caTypingField = 0;
    bool typingUsername=true;

     while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }

            if (event.type == sf::Event::TextEntered && event.text.unicode < 128) {
                char c = static_cast<char>(event.text.unicode);
                if (c == 8) {
                    if (currentScreen == Screen::Login) {
                        if (typingField == 0 && !username.empty()) username.pop_back();
                        else if (typingField == 1 && !password.empty()) password.pop_back();
                    } else if (currentScreen == Screen::CreateAccount) {
                        if (caTypingField == 0 && !caUsername.empty()) caUsername.pop_back();
                        else if (caTypingField == 1 && !caPassword.empty()) caPassword.pop_back();
                        else if (caTypingField == 2 && !caConfirmPassword.empty()) caConfirmPassword.pop_back();
                    }
                } else if (std::isprint(c)) {
                    if (currentScreen == Screen::Login) {
                        if (typingField == 0 && username.size() < 30) username += c;
                        else if (typingField == 1 && password.size() < 30) password += c;
                    } else if (currentScreen == Screen::CreateAccount) {
                        if (caTypingField == 0 && caUsername.size() < 30) caUsername += c;
                        else if (caTypingField == 1 && caPassword.size() < 30) caPassword += c;
                        else if (caTypingField == 2 && caConfirmPassword.size() < 30) caConfirmPassword += c;
                    }
                }
            }

            if (event.type == sf::Event::MouseButtonPressed) {
                auto mouse = sf::Mouse::getPosition(window);

                if (currentScreen == Screen::Login) {
                    if (usernameBox.getGlobalBounds().contains(mouse.x, mouse.y)) typingField = 0;
                    else if (passwordBox.getGlobalBounds().contains(mouse.x, mouse.y)) typingField = 1;

                    if (loginButton.getGlobalBounds().contains(mouse.x, mouse.y)) {
                        sqlite3* db;
                        sqlite3_stmt* stmt;
                        int rc = sqlite3_open("mydatabase.db", &db);
                        if (rc != SQLITE_OK) {
                            cerr << "DB Error: " << sqlite3_errmsg(db) << endl;
                        } else {
                            string sql = "SELECT * FROM login_users WHERE username = ? AND password = ?";
                            rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
                            if (rc == SQLITE_OK) {
                                sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);
                                sqlite3_bind_text(stmt, 2, password.c_str(), -1, SQLITE_STATIC);
                                if (sqlite3_step(stmt) == SQLITE_ROW) {
                                    std::cout << "Login successful!\n";
                                    currentScreen = Screen::Dashboard;
                                    loginErrorMessage.setString("");
                                } else {
                                    loginErrorMessage.setString("Account doesn't exist");
                                }
                                sqlite3_finalize(stmt);
                            }
                            sqlite3_close(db);
                        }
                    }

                    if (CreateText.getGlobalBounds().contains(mouse.x, mouse.y)) {
                        currentScreen = Screen::CreateAccount;
                        caUsername = "";
                        caPassword = "";
                        caConfirmPassword = "";
                        caTypingField = 0;
                    }
                }

                else if (currentScreen == Screen::CreateAccount) {
                    if (caUsernameBox.getGlobalBounds().contains(mouse.x, mouse.y)) caTypingField = 0;
                    else if (caPasswordBox.getGlobalBounds().contains(mouse.x, mouse.y)) caTypingField = 1;
                    else if (caConfirmPasswordBox.getGlobalBounds().contains(mouse.x, mouse.y)) caTypingField = 2;

                    if (caCreateButton.getGlobalBounds().contains(mouse.x, mouse.y)) {
                        if (caUsername.empty() || caPassword.empty() || caConfirmPassword.empty()) {
                            cout << "Fields cannot be empty.\n";
                        } else if (caPassword != caConfirmPassword) {
                            cout << "Passwords do not match.\n";
                        } else {
                            sqlite3* db;
                            sqlite3_stmt* stmt;
                            int rc = sqlite3_open("mydatabase.db", &db);
                            if (rc != SQLITE_OK) {
                                cerr << "DB error: " << sqlite3_errmsg(db) << std::endl;
                            } else {
                                string sql = "INSERT INTO login_users (username, password) VALUES (?, ?)";
                                rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
                                if (rc == SQLITE_OK) {
                                    sqlite3_bind_text(stmt, 1, caUsername.c_str(), -1, SQLITE_STATIC);
                                    sqlite3_bind_text(stmt, 2, caPassword.c_str(), -1, SQLITE_STATIC);
                                    if (sqlite3_step(stmt) == SQLITE_DONE) {
                                        cout << "Account created!\n";
                                        currentScreen = Screen::Login;
                                    } else {
                                        cerr << "Failed to insert user.\n";
                                    }
                                    sqlite3_finalize(stmt);
                                }
                                sqlite3_close(db);
                            }
                        }
                    }
                }
            }
        }

        window.clear(sf::Color(255, 248, 235));

        if (currentScreen == Screen::Login) {
            window.draw(title);
            window.draw(subtitle);
            if (typingUsername) {
                usernameBox.setOutlineThickness(2);
                usernameBox.setOutlineColor(sf::Color::Blue);
                passwordBox.setOutlineThickness(0);
            }
            else {
                passwordBox.setOutlineThickness(2);
                passwordBox.setOutlineColor(sf::Color::Blue);
                usernameBox.setOutlineThickness(0);
            }

            inputUsername.setString(username.empty() ? "Username" : username);
            inputUsername.setFillColor(username.empty() ? sf::Color(105, 105, 105) : sf::Color::Black);
            window.draw(usernameBox);
            window.draw(usernameIconimage);
            window.draw(inputUsername);

            inputPassword.setString(password.empty() ? "Password" : string(password.length(), '*'));
            inputPassword.setFillColor(password.empty() ? sf::Color(105, 105, 105) : sf::Color::Black);
            window.draw(passwordBox);
            window.draw(passwordIconimage);
            window.draw(inputPassword);

            window.draw(loginButton);
            window.draw(loginText);
            window.draw(CreateText);
            window.draw(loginErrorMessage);
        }

        else if (currentScreen == Screen::CreateAccount) {
            caUsernameText.setString(caUsername.empty() ? "Username" : caUsername);
            caUsernameText.setFillColor(caUsername.empty() ? sf::Color(105, 105, 105) : sf::Color::Black);
            window.draw(caUsernameBox);
            window.draw(caUsernameText);

            caPasswordText.setString(caPassword.empty() ? "Password" : string(caPassword.length(), '*'));
            caPasswordText.setFillColor(caPassword.empty() ? sf::Color(105, 105, 105) : sf::Color::Black);
            window.draw(caPasswordBox);
            window.draw(caPasswordText);

            caConfirmPasswordText.setString(caConfirmPassword.empty() ? "Confirm Password" : string(caConfirmPassword.length(), '*'));
            caConfirmPasswordText.setFillColor(caConfirmPassword.empty() ? sf::Color(105, 105, 105) : sf::Color::Black);
            window.draw(caConfirmPasswordBox);
            window.draw(caConfirmPasswordText);

            window.draw(caCreateButton);
            window.draw(caCreateText);
        }

        else if (currentScreen == Screen::Dashboard) {
            sf::Text dashboardText("Welcome to Dashboard!", font, 30);
            dashboardText.setFillColor(sf::Color(75,0,130));
            dashboardText.setPosition(120, 200);
            window.draw(dashboardText);
        }

        window.display();
    }

    //game part
    try {
        Game g;
        while (g.mainmenu());
        cout << "Thank you for playing!\n";
    } catch (const exception& e) {
        cerr << "Fatal error: " << e.what() << endl;
        return 1;
    }
    return 0;
}