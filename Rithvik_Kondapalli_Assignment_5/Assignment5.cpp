
/*
 * Program Name : EECS 348 Assignment 5 - Improved Sudoku Solver
 *
 * Description  : Improves the recursive object-oriented Sudoku solver from
 *                Assignment 4 by adding correctness, time, space, and
 *                maintainability improvements. It uses recursive backtracking
 *                to find every solution to a Sudoku puzzle.
 *
 * Inputs       : puzzle1.txt through puzzle5.txt in the current folder. Each file
 *                has 9 rows of 9 space-separated symbols: a digit 1-9 for a given
 *                clue, or an underscore (_) for a blank cell.
 *
 * Output       : solution1.txt through solution5.txt. Each file contains every
 *                solution found and the total number of solutions, or
 *                "No solution found." if the puzzle has no solutions.
 *                The console also prints the number of solutions for each file.
 *
 * Collaborators: None.
 *
 * Other sources: - ChatGPT (chatgpt.com): assisted with the original class
 *                  layout, input checking, solution counter, and improvements.
 *                - Gemini (gemini.google.com): assisted with the original
 *                  cell-by-cell recursion and single-loop block checking.
 *                - Claude (claude.ai): assisted with Assignment 5 improvements
 *                  involving correctness, execution time, space complexity,
 *                  and maintainability.
 *
 * Author       : Charvi Reddy Konudula
 * Creation date: October 15, 2026
 * Revised by   : Rithvik Kondapalli
 * Revision date: October 29, 2026
 *
 * Labels in the comments: [CHATGPT] = idea taken from ChatGPT's draft,
 * [GEMINI] = idea taken from Gemini's draft, [MIX] = combined ideas
 * or modifications based on multiple sources.
 */

#include <iostream>   // cout and cerr for printing
#include <fstream>    // ifstream for reading and ofstream for writing files
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
    int solutionCount;           // number of full solutions found so far

    // [GEMINI] Checks whether 'digit' can go at (row, col). One loop looks at
    // the row, the column, and the 3x3 block at the same time. boxTop and
    // boxLeft are the top-left corner of the block that holds this cell. For
    // each i from 0 to 8, board[row][i] is the row cell and board[i][col] is
    // the column cell. i / BOX and i % BOX turn i into an offset inside the
    // block, so the third check covers all 9 block cells. If the digit is
    // already in any of them, it returns false. Otherwise it returns true.
    // Check whether a candidate conflicts with an existing value.
    bool isSafe(int row, int col, char digit) const {
        int boxTop = BOX * (row / BOX);
        int boxLeft = BOX * (col / BOX);
        for (int i = 0; i < SIZE; ++i) {
            if (board[row][i] == digit) return false;
            if (board[i][col] == digit) return false;
            if (board[boxTop + i / BOX][boxLeft + i % BOX] == digit) return false;
        }
        return true;
    }

    // [MIX] Goes through the original puzzle and checks that the given
    // digits do not already break Sudoku rules. For each filled cell, it
    // checks the rest of the row, column, and 3x3 block for duplicates.
    // The current cell is excluded from the comparisons so that a digit
    // does not conflict with itself. If a duplicate is found, it returns
    // false. Otherwise it returns true.
    // Correctness improvement: ensure given clues do not already conflict.
    bool validInitialPuzzle() const {
        for (int r = 0; r < SIZE; ++r) {
            for (int c = 0; c < SIZE; ++c) {
                if (board[r][c] == EMPTY) continue;
                char digit = board[r][c];
                for (int k = 0; k < SIZE; ++k) {
                    if (k != c && board[r][k] == digit) return false;
                    if (k != r && board[k][c] == digit) return false;
                }
                int boxTop = BOX * (r / BOX);
                int boxLeft = BOX * (c / BOX);
                for (int dr = 0; dr < BOX; ++dr) {
                    for (int dc = 0; dc < BOX; ++dc) {
                        int rr = boxTop + dr;
                        int cc = boxLeft + dc;
                        if ((rr != r || cc != c) && board[rr][cc] == digit) {
                            return false;
                        }
                    }
                }
            }
        }
        return true;
    }

    // [MIX] Prints a 9x9 grid, one row per line with spaces between the
    // symbols. Instead of always using cout, it takes an output stream
    // as a parameter so the same function can print to the console or
    // write to a file. The board is not changed while printing.
    // Maintainability improvement: this works for files or standard output.
    void writeBoard(std::ostream& out) const {
        for (int r = 0; r < SIZE; ++r) {
            for (int c = 0; c < SIZE; ++c) {
                out << board[r][c];
                if (c < SIZE - 1) out << ' ';
            }
            out << '\n';
        }
    }

    // [GEMINI] [MIX] The recursive search. 'cell' goes from 0 to 80 and
    // stands for the cell we are working on (row = cell / 9, col = cell % 9).
    // Gemini's draft moved through the cells in order instead of rescanning
    // the grid for an empty cell each time; I turned its row/col pair into a
    // single index.
    //
    // Base case: when cell reaches 81, every cell is filled, so this is a
    // complete solution. It is counted and written to the output stream,
    // and then the function returns so the search can keep looking for
    // other solutions.
    //
    // If the cell already has a digit (a given clue), skip to the next cell.
    // The while loop skips consecutive filled cells without needing a new
    // recursive call for each one.
    //
    // Otherwise try the digits 1 to 9. For each digit that isSafe() allows,
    // place it, call search() on the next cell, and then erase it again.
    // The erase is the backtracking step: it lets the loop try the next
    // digit, and it makes sure the search finds every solution, not just
    // the first one. If no digit fits, the loop ends and the function
    // returns to the previous cell, which then tries its next digit.
    // Space improvement: pass the stream by reference instead of storing it.
    void search(int cell, std::ostream& out) {
        // Time improvement: skip all consecutive pre-filled cells in one call.
        while (cell < SIZE * SIZE &&
               board[cell / SIZE][cell % SIZE] != EMPTY) {
            ++cell;
        }

        if (cell == SIZE * SIZE) {
            if (solutionCount > 0) out << '\n';
            ++solutionCount;
            writeBoard(out);
            return;
        }

        int row = cell / SIZE;
        int col = cell % SIZE;
        for (char digit = '1'; digit <= '9'; ++digit) {
            if (isSafe(row, col, digit)) {
                board[row][col] = digit;
                search(cell + 1, out);
                board[row][col] = EMPTY;
            }
        }
    }

public:
    SudokuSolver() : solutionCount(0) {}

    // [CHATGPT] Reads a puzzle file into the board. I kept ChatGPT's input
    // checking because Gemini's draft had none. It returns false with an
    // error message if the file will not open, if there are fewer than 81
    // symbols, or if a symbol is not "_" or a single digit 1-9. Each symbol
    // is read as a string token (so values like "10" are caught) and its
    // first character is stored in board.
    //
    // [MIX] Additional input checking rejects files containing more than
    // 81 cells and puzzles with conflicting given clues. The puzzle is only
    // accepted if all these checks pass.
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
                    std::cerr << "Error: " << filename
                              << " has fewer than 81 cells\n";
                    return false;
                }
                bool isBlank = (token == "_");
                bool isDigit = (token.length() == 1 &&
                                token[0] >= '1' && token[0] <= '9');
                if (!isBlank && !isDigit) {
                    std::cerr << "Error: bad symbol '" << token
                              << "' in " << filename << '\n';
                    return false;
                }
                board[r][c] = token[0];
            }
        }
        if (file >> token) {
            std::cerr << "Error: " << filename << " has more than 81 cells\n";
            return false;
        }
        if (!validInitialPuzzle()) {
            std::cerr << "Error: conflicting clues in " << filename << '\n';
            return false;
        }
        return true;
    }

    // [MIX] Opens the solution file, resets the solution counter, and
    // starts the recursive search at cell 0. Each complete solution is
    // written directly to the output file instead of being stored in
    // memory. After searching, it writes the number of solutions found
    // or "No solution found." if the count is still zero.
    //
    // If the output file cannot be opened, an error message is printed
    // and -1 is returned. Otherwise the number of solutions is returned.
    int solve(const std::string& solutionFile) {
        std::ofstream out(solutionFile);
        if (!out.is_open()) {
            std::cerr << "Error: cannot write " << solutionFile << '\n';
            return -1;
        }

        solutionCount = 0;
        search(0, out);
        if (solutionCount == 0) {
            out << "No solution found.\n";
        } else {
            out << "\nSolutions found: " << solutionCount << '\n';
        }
        return solutionCount;
    }
};

// main goes through the five puzzle files. For each one it builds the input
// and output file names, creates a new SudokuSolver, and loads the puzzle.
// If loading succeeds, it solves the puzzle and writes every solution to
// the corresponding output file. The console prints the number of solutions
// and the output file name. If a file fails to load or write, the program
// prints an error message and continues to the next puzzle.
int main() {
    for (int i = 1; i <= NUM_PUZZLES; ++i) {
        std::string puzzleFile = "puzzle" + std::to_string(i) + ".txt";
        std::string solutionFile = "solution" + std::to_string(i) + ".txt";
        SudokuSolver solver;
        if (solver.loadPuzzle(puzzleFile)) {
            int count = solver.solve(solutionFile);
            if (count >= 0) {
                std::cout << puzzleFile << ": " << count
                          << " solution(s) written to " << solutionFile << '\n';
            }
        }
    }
    return 0;
}
