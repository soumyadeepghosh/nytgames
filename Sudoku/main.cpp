#include <iostream>
#include "sudoku.h"
int main(int argc, char **argv) {
    if (argc < 2) {
        std::cerr << " No input file provided" << std::endl;
        return 1;
    }
    if (argc > 2) {
        std::cerr << " Too many arguments provided" << std::endl;
        return 1;
    }
    Sudoku::SudokuSolver solver(argv[1]);
    if (!solver.isSanePuzzle()) {
        std::cerr << " Input puzzle failed sanity checks" << std::endl;
        return 1;
    }
    solver.solve();
    solver.printSudokuState();

    return 0;
}
