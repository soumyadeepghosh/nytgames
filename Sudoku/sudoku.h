#ifndef SUDOKU_H_
#define SUDOKU_H_

#include <memory>
#include <array>
#include <vector>

using namespace std;

namespace Sudoku {

static constexpr unsigned GRID_SIZE = 9;
static constexpr unsigned SQUARE_SIZE = 3;
using SudokuValue = int;

template<typename T>
using OneDGrid = array<T, GRID_SIZE>;
template<typename T>
using TwoDGrid = array<T, GRID_SIZE * GRID_SIZE>;

inline unsigned getIndex(unsigned row, unsigned col) {
    return (row * GRID_SIZE + col);
}

class Step {
    unsigned index;
    SudokuValue value;

 public:
    explicit Step(unsigned i, SudokuValue v) : index(i), value(v) { }
    unsigned getIndex() const { return index; }
    unsigned getValue() const { return value; }
};

class CellState {
    unsigned index_i;
    SudokuValue state_i;
    SudokuValue allowed_i;
    SudokuValue possibilities_i;

 public:
    explicit CellState(unsigned i, SudokuValue s, SudokuValue a, SudokuValue p)
        : index_i(i), state_i(s), allowed_i(a), possibilities_i(p) { }
    explicit CellState(const CellState& c)
        : index_i(c.index()), state_i(c.state()), allowed_i(c.allowed()),
          possibilities_i(c.possibilities()) { }

    unsigned index() const { return index_i; }
    unsigned state() const { return state_i; }
    unsigned allowed() const { return allowed_i; }
    unsigned possibilities() const { return possibilities_i; }
    bool isFilled() const { return possibilities_i == 0; }

    void setIndex(unsigned i) { index_i = i; }
    void setState(SudokuValue s) { state_i = s; }
    void setAllowed(SudokuValue a) { allowed_i = a; }
    void setPossibilities(SudokuValue p) { possibilities_i = p; }
};

class SudokuTransaction {
    bool solved;
    bool validTransaction;
    // Entire state of the sudoku puzzle.
    TwoDGrid<SudokuValue> sudokuState;
    // rows[i] = All the values already present in the ith row.
    OneDGrid<SudokuValue> rows;
    // cols[j] = All the values already present in the jth column.
    OneDGrid<SudokuValue> cols;
    // squares[k] = All the values already present in the kth square.
    // k = 0, for the SQUARE_SIZE * SQUARE_SIZE grid starting at (0, 0)
    // k = 1, for the SQUARE_SIZE * SQUARE_SIZE grid starting at (0,
    // SQUARE_SIZE)
    OneDGrid<SudokuValue> squares;
    // allowedState[i][j] = All the numbers allowed for the cell at (i, j)
    TwoDGrid<SudokuValue> allowedState;
    // possibilities[i][j] = Values possible for the cell at (i, j)
    TwoDGrid<SudokuValue> possibilities;
    // valuePresentIn*[i][j] = true, if the sudoku cell value i is present in
    // the particular row, column, or square, i.
    TwoDGrid<bool> valuePresentInRows;
    TwoDGrid<bool> valuePresentInCols;
    TwoDGrid<bool> valuePresentInSquares;

    void copyState(const SudokuTransaction& parent);
    bool processAllowed(bool dbgMethod = false);

    SudokuValue getValue(unsigned k) const;
    unsigned getSquareIndex(unsigned row, unsigned col) const;
    bool isCandidatePossible(unsigned candidateValue, unsigned row,
                             unsigned col, unsigned sqIndex,
                             bool dbgMethod=false) const;

    const vector<SudokuValue>
        getPossibilities(unsigned row, unsigned col) const;
    const vector<SudokuValue> getPossibilities(unsigned index) const;
    SudokuValue getSinglePossibility(unsigned index) const;
    SudokuValue getSinglePossibility(unsigned row, unsigned col) const;
    bool updateSinglePossibilities(bool dbgMethod = false);
    void setCell(unsigned index, SudokuValue value, bool dbgMethod = false);
    void initState();
    void reverseIndexLookup(unsigned index, unsigned* row, unsigned* col) const;
    unsigned getNextCellToFill(bool dbgMethod) const;

 public:
    explicit SudokuTransaction(
            const array<SudokuValue, GRID_SIZE * GRID_SIZE>& input);
    explicit SudokuTransaction(const SudokuTransaction& parent,
                               unsigned index,
                               SudokuValue value);

    bool isValidTransaction() const { return validTransaction; }
    bool isSolved() const { return solved; }
    bool solve(bool dbgMethod = false);
    void printSudokuState() const;

    SudokuValue getSudokuState(unsigned row, unsigned col) const {
        return sudokuState.at(getIndex(row, col));
    }
    SudokuValue getRow(unsigned row) const { return rows.at(row); }
    SudokuValue getCol(unsigned col) const { return cols.at(col); }
    SudokuValue getSquare(unsigned sqIndex) const {
        return squares.at(sqIndex);
    }
    SudokuValue getAllowedState(unsigned row, unsigned col) const {
        return allowedState.at(getIndex(row, col));
    }
    SudokuValue getPossibleValues(unsigned row, unsigned col) const {
        return possibilities.at(getIndex(row, col));
    }
    bool rowPresent(unsigned row, unsigned col) const {
        return valuePresentInRows.at(getIndex(row, col));
    }
    bool colPresent(unsigned row, unsigned col) const {
        return valuePresentInCols.at(getIndex(row, col));
    }
    bool squarePresent(unsigned row, unsigned col) const {
        return valuePresentInSquares.at(getIndex(row, col));
    }

    const array<SudokuValue, GRID_SIZE * GRID_SIZE>& getSudokuState() const {
        return sudokuState;
    }
    const array<SudokuValue, GRID_SIZE>& getRows() const { return rows; }
    const array<SudokuValue, GRID_SIZE>& getCols() const { return cols; }
    const array<SudokuValue, GRID_SIZE>& getSquares() const { return squares; }
    const array<SudokuValue, GRID_SIZE * GRID_SIZE>& getAllowedState() const {
        return allowedState;
    }
    const array<SudokuValue, GRID_SIZE * GRID_SIZE>& getPossibilities() const {
        return possibilities;
    }
    const array<bool, GRID_SIZE * GRID_SIZE>& getValuePresentInRows() const {
        return valuePresentInRows;
    }
    const array<bool, GRID_SIZE * GRID_SIZE>& getValuePresentInCols() const {
        return valuePresentInCols;
    }
    const array<bool, GRID_SIZE * GRID_SIZE>& getValuePresentInSquares() const {
        return valuePresentInSquares;
    }
};

class SudokuSolver {
    bool saneConfig = false;
    TwoDGrid<SudokuValue> sudokuState;
    std::unique_ptr<SudokuTransaction> rootTransaction;

    bool processInputFile(const char* inputFileName);
    bool processCell(char inputChar, unsigned row, unsigned col);

    void commitSudokuState() { sudokuState = rootTransaction->getSudokuState(); }

 public:
    void printSudokuState() const;
    explicit SudokuSolver(const char* inputFileName);
    bool isSanePuzzle() const { return saneConfig; }
    bool solve();
};

}

#endif  /* SUDOKU_H_ */
