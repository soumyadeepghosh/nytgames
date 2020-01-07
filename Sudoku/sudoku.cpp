#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include "sudoku.h"

#include <queue>
#include <unordered_map>

namespace Sudoku {

uint32_t SudokuTransaction::tx = 0;

bool SudokuSolver::processCell(char inputChar, unsigned row, unsigned col) {
    if (inputChar == '.') {
        sudokuState[getIndex(row, col)] = -1;
        return true;
    }
    SudokuValue num = inputChar - '0';
    if (num < 1 || num > GRID_SIZE) {
        std::cerr << " Incorrect input value " << inputChar << " found." <<
            " All sudoku cell values must be between 1 and " << GRID_SIZE <<
            std::endl;
        return false;
    }
    sudokuState[getIndex(row, col)] = num;
    return true;
}

void SudokuSolver::printSudokuState() const {
    rootTransaction->printSudokuState();
}

void SudokuTransaction::printSudokuState() const {
    for (unsigned i = 0; i < GRID_SIZE; i++) {
        std::stringstream ss;
        for (unsigned j = 0; j < GRID_SIZE; j++) {
            unsigned index = getIndex(i, j);
            if (sudokuState[index] == -1)
                ss << ". ";
            else
                ss << sudokuState[index] << " ";
        }
        std::cout << ss.str() << std::endl;
    }
}

bool SudokuSolver::processInputFile(const char* inputFileName) {
    static constexpr bool dbgFunction = false;
    std::ifstream inputFile(inputFileName);
    if (!inputFile.is_open()) {
        std::cerr << " Cannot find " << inputFileName << std::endl;
        return false;
    }
    unsigned currentRow = 0;
    unsigned currentCol = 0;
    char c;
    while (inputFile >> c) {
        if (currentRow > GRID_SIZE) {
            saneConfig = true;
            std::cerr << " Ignoring everything after the first " <<
                GRID_SIZE << " lines." << std::endl;
            return true;
        }
        if (!processCell(c, currentRow, currentCol)) {
            saneConfig = false;
            return false;
        }
        ++currentCol;
        if (currentCol >= GRID_SIZE) {
            currentRow++;
            currentCol = 0;
        }
    }
    saneConfig = true;
    return true;
}

SudokuSolver::SudokuSolver(const char* inputFileName) {
    bool processedInput = processInputFile(inputFileName);
    if (!processedInput) return;
    rootTransaction = std::make_unique<SudokuTransaction>(sudokuState);
    if (!rootTransaction->isValidTransaction()) {
        saneConfig = false;
        std::cerr << " Root configuration found invalid" << std::endl;
    }
}

bool SudokuSolver::solve() {
    if (rootTransaction->isSolved()) {
        commitSudokuState();
        return true;
    }
    saneConfig &= rootTransaction->solve();
    if (!rootTransaction->isValidTransaction()) {
        std::cerr << " No solution possible." << std::endl;
        return false;
    }
    if (!rootTransaction->isSolved()) {
        std::cerr << " No solution found." << std::endl;
        return false;
    }
    commitSudokuState();
    return true;
}

void SudokuTransaction::initState() {
    sudokuState.fill(0);
    rows.fill(0);
    cols.fill(0);
    squares.fill(0);
    allowedState.fill(0);
    valuePresentInRows.fill(0);
    valuePresentInCols.fill(0);
    valuePresentInSquares.fill(0);
    possibilities.fill(0);
}

SudokuTransaction::SudokuTransaction(
        const array<SudokuValue, GRID_SIZE * GRID_SIZE>& input) {
    solved = true;
    validTransaction = true;
    initState();
    // Local copy of the value for every transaction.
    sudokuState = input;
    for (unsigned i = 0; i < GRID_SIZE; i++) {
        for (unsigned j = 0; j < GRID_SIZE; j++) {
            auto index = getIndex(i, j);
            if (sudokuState[index] == -1) {
                solved = false;
                continue;
            }
            unsigned value = sudokuState[index];
            SudokuValue entry = getValue(value);

            // If the state of the sudoku is not solvable, then return
            // prematurely for this transaction.
            unsigned sqIndex = getSquareIndex(i, j);
            if (!isCandidatePossible(entry, i, j, sqIndex)) {
                validTransaction = false;
                return;
            }

            rows[i] |= entry;
            cols[j] |= entry;
            squares[sqIndex] |= entry;
            valuePresentInRows[getIndex(value, i)] = true;
            valuePresentInCols[getIndex(value, j)] = true;
            valuePresentInSquares[getIndex(value, sqIndex)] = true;
        }
    }
    if (solved) return;
    if (!processAllowed()) validTransaction = false;
    validTransaction &= updateSinglePossibilities();
}

SudokuTransaction::SudokuTransaction(const SudokuTransaction& parent,
        unsigned index,
        SudokuValue value) {
    unsigned row, col;
    reverseIndexLookup(index, &row, &col);
    copyState(parent);
    validTransaction = true;
    setCell(getIndex(row, col), value);
    if (solved || !validTransaction) return;
    // Update all single possibility states that emerged. This prevents
    // unnecessary forking off of transactions.
    validTransaction &= updateSinglePossibilities();
    if (solved || !validTransaction) return;
    validTransaction &= solve();
}

void SudokuTransaction::reverseIndexLookup(unsigned index,
                                           unsigned* row,
                                           unsigned* col) const {
    *row = index / GRID_SIZE;
    *col = index % GRID_SIZE;
}

void SudokuTransaction::setCell(unsigned index,
                                SudokuValue value) {
    // Update the entry at index = getIndex(row, col) to value.
    unsigned row, col;
    reverseIndexLookup(index, &row, &col);
    if (sudokuState.at(index) != -1) {
        std::stringstream ss;
        ss << "Trying to set value of cell (" << row << ", " << col <<
            ") already containing " << sudokuState.at(index) << " to " <<
            value << std::endl;
        throw std::runtime_error(ss.str());
    }
    SudokuValue entry = getValue(value);
    unsigned sqIndex = getSquareIndex(row, col);
    sudokuState[index] = value;
    rows[row] |= entry;
    cols[col] |= entry;
    squares[sqIndex] |= entry;
    valuePresentInRows[getIndex(value, row)] = true;
    valuePresentInCols[getIndex(value, col)] = true;
    valuePresentInSquares[getIndex(value, sqIndex)] = true;
    allowedState[index] = 0;
    possibilities[index] = 0;

    if (!processAllowed()) validTransaction = false;
}

void SudokuTransaction::copyState(const SudokuTransaction& parent) {
    sudokuState = parent.getSudokuState();
    rows = parent.getRows();
    cols = parent.getCols();
    squares = parent.getSquares();
    allowedState = parent.getAllowedState();
    possibilities = parent.getPossibilities();
    valuePresentInRows = parent.getValuePresentInRows();
    valuePresentInCols = parent.getValuePresentInCols();
    valuePresentInSquares = parent.getValuePresentInSquares();
    solved = parent.isSolved();
    validTransaction = parent.isValidTransaction();
}

bool SudokuTransaction::processAllowed() {
    bool foundPossibilities = false;
    for (unsigned i = 0; i < GRID_SIZE; i++) {
        for (unsigned j = 0; j < GRID_SIZE; j++) {
            unsigned index = getIndex(i, j);
            possibilities[index] = 0;
            allowedState[index] = 0;
            if (sudokuState[index] != -1) continue;
            unsigned sqIndex = getSquareIndex(i, j);
            for (unsigned k = 1; k <= GRID_SIZE; k++) {
                SudokuValue candidateValue = getValue(k);
                bool candidatePossible =
                    isCandidatePossible(candidateValue, i, j, sqIndex);
                if (!candidatePossible) continue;
                allowedState[index] |= candidateValue;
                possibilities[index]++;
                foundPossibilities = true;
            }
            if (possibilities[index] == 0) return false;
        }
    }
    if (!foundPossibilities) solved = true;
    return true;
}

SudokuValue SudokuTransaction::getValue(unsigned k) const {
    if (k < 1 || k > GRID_SIZE) {
        std::stringstream ss;
        ss << "Invalid value for a cell in the puzzle: " << k;
        throw std::runtime_error(ss.str());
    }
    return 1 << (k - 1);
}

unsigned SudokuTransaction::getSquareIndex(unsigned row,
                                           unsigned col) const {
    if (row < 0 || row > GRID_SIZE) {
        std::stringstream ss;
        ss << "Invalid row index in the puzzle: " << row;
        throw std::runtime_error(ss.str());
    }
    if (col < 0 || col > GRID_SIZE) {
        std::stringstream ss;
        ss << "Invalid column index in the puzzle: " << col;
        throw std::runtime_error(ss.str());
    }
    return (SQUARE_SIZE * (row / SQUARE_SIZE)) + (col / SQUARE_SIZE);
}

bool SudokuTransaction::isCandidatePossible(unsigned candidateValue,
        unsigned row,
        unsigned col,
        unsigned sqIndex) const {
    if (rows[row] != 0 && ((rows[row] & candidateValue) != 0)) return false;
    if (cols[col] != 0 && ((cols[col] & candidateValue) != 0)) return false;
    if (squares[sqIndex] != 0 && ((squares[sqIndex] & candidateValue) != 0))
        return false;
    return true;
}

unsigned SudokuTransaction::getNextCellToFill() const {
    unsigned rv = sudokuState.size();
    unsigned min_possibilities = GRID_SIZE + 1;
    for (unsigned i = 0; i < possibilities.size(); i++) {
        if (possibilities[i] == 0) continue;
        if (min_possibilities > possibilities[i]) {
            min_possibilities = possibilities[i];
            rv = i;
        }
    }
    return rv;
}

bool SudokuTransaction::solve() {
    // This is called after all the single possibility cells are filled out,
    // and there is at least one unfilled cell in the puzzle with multiple
    // possibilities.
    const auto nextCellToFill = getNextCellToFill();
    if (nextCellToFill == sudokuState.size()) return false;
    const auto values = getPossibilities(nextCellToFill);
    // If there are no possibilities, then clearly we cannot solve this
    // puzzle.
    if (values.size() == 0) return false;
    for (auto value : values) {
        std::unique_ptr<SudokuTransaction> nextStep =
            std::make_unique<SudokuTransaction>(*this, nextCellToFill,
                    value);
        if (!nextStep->isValidTransaction()) continue;
        if (!nextStep->isSolved()) continue;
        if (nextStep->isSolved()) {
            copyState(*nextStep);
            break;
        }
    }
    return true;
}

SudokuValue SudokuTransaction::getSinglePossibility(unsigned index) const {
    unsigned row, col;
    reverseIndexLookup(index, &row, &col);
    return getSinglePossibility(row, col);
}

SudokuValue SudokuTransaction::getSinglePossibility(unsigned row,
                                                    unsigned col) const {
    auto index = getIndex(row, col);
    if (possibilities[index] != 1) {
        std::stringstream ss;
        ss << "Multiple possibilities for cell (" << row << ", " << col <<
            ")";
        throw std::runtime_error(ss.str());
    }
    const auto possibilities = getPossibilities(row, col);
    if (possibilities.size() != 1) {
        std::stringstream ss;
        ss << "Discrepancy between possibilities and actual possible "
            "values for (" << row << ", " << col << ")";
        throw std::runtime_error(ss.str());
    }
    return possibilities.at(0);
}

const vector<SudokuValue>
SudokuTransaction::getPossibilities(unsigned index) const {
    unsigned row, col;
    reverseIndexLookup(index, &row, &col);
    return getPossibilities(row, col);
}

const vector<SudokuValue>
SudokuTransaction::getPossibilities(unsigned row, unsigned col) const {
    vector<SudokuValue> rv;
    auto index = getIndex(row, col);
    if (possibilities[index] == 0) return rv;
    for (unsigned i = 1; i <= GRID_SIZE; i++) {
        SudokuValue value = getValue(i);
        if (allowedState[index] && ((allowedState[index] & value) != 0)) {
            rv.push_back(i);
        }
    }
    return rv;
}

bool SudokuTransaction::updateSinglePossibilities() {
    queue<Step> singleNodes;
    std::unordered_map<unsigned, Step> nodesAddedToQueue;
    do {
        for (unsigned i = 0; i < possibilities.size(); i++) {
            if (possibilities.at(i) != 1) continue;
            Step toAdd(i, getSinglePossibility(i));
            if (nodesAddedToQueue.count(i)) continue;
            singleNodes.push(toAdd);
            nodesAddedToQueue.emplace(i, toAdd);
        }
        if (singleNodes.empty()) break;
        auto& nextStep = singleNodes.front();
        singleNodes.pop();
        nodesAddedToQueue.erase(nextStep.getIndex());
        setCell(nextStep.getIndex(), nextStep.getValue());
    } while (!singleNodes.empty());
    return true;
}

}
