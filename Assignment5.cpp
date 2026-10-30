/*
 * Program Name : EECS 348 Assignment 4 - Recursive Object-Oriented Sudoku Solver
 *
 * Description  : Solves Sudoku puzzles using recursive backtracking inside a
 *                SudokuSolver class. It finds every solution for a puzzle, and
 *                prints "No solution found" if there are none.
 *
 * Inputs       : puzzle1.txt through puzzle5.txt in the current folder. Each file
 *                has 9 rows of 9 space-separated symbols: a digit 1-9 for a given
 *                clue, or an underscore (_) for a blank cell.
 *
 * Output       : For each file: the file name, the puzzle as read, each solution
 *                found (or "No solution found"), and the number of solutions.
 *
 * Collaborators: None.
 *
 * Other sources: - ChatGPT (chatgpt.com): first draft. I used its class layout,
 *                  its input checking, and its solution counter.
 *                - Gemini (gemini.google.com): second draft. I used its
 *                  cell-by-cell recursion and its single-loop block check.
 *                Code marked [MIX] combines ideas from both drafts. I reviewed
 *                and tested all of it.
 *
 * Author       : Charvi Reddy Konudula
 * Creation date: October 15, 2026
 *
 * Labels in the comments: [CHATGPT] = idea taken from ChatGPT's draft,
 * [GEMINI] = idea taken from Gemini's draft, [MIX] = combined by me from both.
 */

#include <iostream>   // cout and cerr for printing
#include <fstream>    // ifstream for reading the puzzle files
#include <string>     // string for file names and for reading symbols

// Constants so the board size is written in one place instead of using 9 and 3
// everywhere. ChatGPT's draft hard-coded these numbers, so this part is [MIX].
const int SIZE = 9;          // the board is 9 x 9
const int BOX = 3;           // each block is 3 x 3
const char EMPTY = '_';      // symbol for a blank cell
const int NUM_PUZZLES = 5;   // puzzle1.txt through puzzle5.txt

// One SudokuSolver object holds one puzzle and solves it.
class SudokuSolver {
private:
    char board[SIZE][SIZE];      // working grid that the search fills in and erases
    char original[SIZE][SIZE];   // copy of the puzzle as read, used for printing it
    int solutionCount;           // number of full solutions found so far

    // [GEMINI] Checks whether 'digit' can go at (row, col). One loop looks at
    // the row, the column, and the 3x3 block at the same time. boxTop and
    // boxLeft are the top-left corner of the block that holds this cell. For
    // each i from 0 to 8, board[row][i] is the row cell and board[i][col] is
    // the column cell. i / BOX and i % BOX turn i into an offset inside the
    // block, so the third check covers all 9 block cells. If the digit is
    // already in any of them, it returns false. Otherwise it returns true.
    bool isSafe(int row, int col, char digit) const {
        int boxTop = BOX * (row / BOX);
        int boxLeft = BOX * (col / BOX);
        for (int i = 0; i < SIZE; ++i) {
            if (board[row][i] == digit) {
                return false;
            }
            if (board[i][col] == digit) {
                return false;
            }
            if (board[boxTop + i / BOX][boxLeft + i % BOX] == digit) {
                return false;
            }
        }
        return true;
    }

    // Prints a 9x9 grid, one row per line with spaces between the symbols.
    // It takes the grid as a parameter so it can print either the working
    // board (for solutions) or the saved original (for the puzzle as read).
    void printBoard(const char grid[SIZE][SIZE]) const {
        for (int r = 0; r < SIZE; ++r) {
            for (int c = 0; c < SIZE; ++c) {
                std::cout << grid[r][c];
                if (c < SIZE - 1) {
                    std::cout << ' ';
                }
            }
            std::cout << '\n';
        }
    }

    // [GEMINI] [MIX] The recursive search. 'cell' goes from 0 to 80 and
    // stands for the cell we are working on (row = cell / 9, col = cell % 9).
    // Gemini's draft moved through the cells in order instead of rescanning
    // the grid for an empty cell each time; I turned its row/col pair into a
    // single index.
    //
    // Base case: when cell reaches 81, every cell is filled, so this is a
    // complete solution. It is counted and printed, and then the function
    // returns so the search can keep looking for other solutions.
    //
    // If the cell already has a digit (a given clue), skip to the next cell.
    //
    // Otherwise try the digits 1 to 9. For each digit that isSafe() allows,
    // place it, call search() on the next cell, and then erase it again.
    // The erase is the backtracking step: it lets the loop try the next
    // digit, and it makes sure the search finds every solution, not just
    // the first one. If no digit fits, the loop ends and the function
    // returns to the previous cell, which then tries its next digit.
    void search(int cell) {
        if (cell == SIZE * SIZE) {
            ++solutionCount;
            std::cout << "\nSolution " << solutionCount << ":\n";
            printBoard(board);
            return;
        }
        int row = cell / SIZE;
        int col = cell % SIZE;
        if (board[row][col] != EMPTY) {
            search(cell + 1);
            return;
        }
        for (char digit = '1'; digit <= '9'; ++digit) {
            if (isSafe(row, col, digit)) {
                board[row][col] = digit;
                search(cell + 1);
                board[row][col] = EMPTY;
            }
        }
    }

public:
    SudokuSolver() : solutionCount(0) {}

    // [CHATGPT] Reads a puzzle file into both grids. I kept ChatGPT's input
    // checking because Gemini's draft had none. It returns false with an
    // error message if the file will not open, if there are fewer than 81
    // symbols, or if a symbol is not "_" or a single digit 1-9. Each symbol
    // is read as a string token (so values like "10" are caught) and its
    // first character is stored in board and in original.
    bool loadPuzzle(const std::string& filename) {
        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Error: cannot open " << filename << '\n';
            return false;
        }
        std::string token;
        for (int r = 0; r < SIZE; ++r) {
            for (int c = 0; c < SIZE; ++c) {
                if (!(file >> token)) {
                    std::cerr << "Error: " << filename << " has fewer than 81 cells\n";
                    return false;
                }
                bool isBlank = (token == "_");
                bool isDigit = (token.length() == 1 && token[0] >= '1' && token[0] <= '9');
                if (!isBlank && !isDigit) {
                    std::cerr << "Error: bad symbol '" << token << "' in " << filename << '\n';
                    return false;
                }
                board[r][c] = token[0];
                original[r][c] = token[0];
            }
        }
        return true;
    }

    // Prints the puzzle exactly as it was read from the file.
    void printOriginal() const {
        printBoard(original);
    }

    // Resets the counter and starts the search at cell 0. When the search is
    // done, it prints the total number of solutions, or "No solution found"
    // (the wording the assignment asks for) if the count is still 0.
    void solve() {
        solutionCount = 0;
        search(0);
        if (solutionCount == 0) {
            std::cout << "No solution found\n";
        } else {
            std::cout << "\nTotal solutions found: " << solutionCount << '\n';
        }
    }
};

// main goes through the five puzzle files. For each one it builds the file
// name, prints a header with the name, creates a new SudokuSolver, loads the
// puzzle, and only if the load worked prints the puzzle and solves it.
// A blank line is printed between puzzles.
int main() {
    for (int i = 1; i <= NUM_PUZZLES; ++i) {
        std::string filename = "puzzle" + std::to_string(i) + ".txt";
        std::cout << "========================================\n";
        std::cout << "File: " << filename << '\n';
        std::cout << "========================================\n";
        SudokuSolver solver;
        if (solver.loadPuzzle(filename)) {
            std::cout << "\nPuzzle:\n";
            solver.printOriginal();
            solver.solve();
        }
        std::cout << '\n';
    }
    return 0;
}
