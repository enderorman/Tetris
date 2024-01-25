#ifndef BLOCKFALL_H
#define BLOCKFALL_H

#define occupiedCellChar "██"
#define unoccupiedCellChar "▒▒"

#include <vector>
#include <string>
#include <map>

#include "Block.h"
#include "LeaderboardEntry.h"
#include "Leaderboard.h"

using namespace std;

class BlockFall {
public:

    BlockFall(string grid_file_name, string blocks_file_name, bool gravity_mode_on, const string &leaderboard_file_name,
              const string &player_name);
    virtual ~BlockFall();
    int rows;  // Number of rows in the grid
    int cols;  // Number of columns in the grid
    int currentBlockStartRow = 0, currentBlockStartCol = 0;
    vector<vector<int>> grid;  // 2D game grid
    vector<vector<bool>> power_up; // 2D matrix of the power-up shape
    Block * initial_block = nullptr; // Head of the list of game blocks. Must be filled up and initialized after a call to read_blocks()
    Block * active_rotation = nullptr; // Currently active rotation of the active block. Must start with the initial_block
    bool gravity_mode_on = false; // Gravity mode of the game
    bool dropIsActive = false;
    unsigned long current_score = 0; // Current score of the game
    string leaderboard_file_name; // Leaderboard file name, taken from the command-line argument 5 in main
    string player_name; // Player name, taken from the command-line argument 6 in main
    Leaderboard leaderboard;

    void initialize_grid(const string & input_file); // Initializes the grid using the command-line argument 1 in main
    void read_blocks(const string & input_file); // Reads the input file and calls the read_block() function for each block;
    void connectRotationBlocks(Block* block, Block* rightRotationBlock, Block* leftRotationBlock, Block* halfRotationBlock);
    vector<vector<bool>> getRightRotatedMatrix(vector<vector<bool>>& matrix);
    vector<vector<bool>> getLeftRotatedMatrix(vector<vector<bool>>& matrix);
    vector<vector<bool>> getHalfRotatedMatrix(vector<vector<bool>>& matrix);
    bool checkBlockPositionIsOkay(vector<vector<bool>>& currentBlockMatrix, int startRow, int startCol);
    bool checkIfBlockIsOutOfBonds(int i, int j);
    bool checkCollisionEnteringGrid();
    bool checkIfThereIsPowerUp();
    bool checkPowerUpForGivenRange(int startRow, int startCol);
    bool checkIfThereAreFullRows();
    bool checkIfRowIsFull(int rowIndex);
    pair<int, int> getNoGravityDropPosition();
    void printCollisionGameOver(ofstream& outfile);
    void updateGameStatementIfThereIsPowerUp();
    void updateGameStatementIfThereAreFullRows();
    void dropBlockOnGrid(ofstream& outfile);
    void printFinishGameNoMoreCommands(ofstream& outfile);
    void activateGravitySwitch(ofstream& outfile);
    void locateBlock(vector<vector<int>>& matrix);
    void clearGrid();
    void shiftGrid(int rowIndex);
    void printMatrix(vector<vector<int>>& matrix, ofstream& outfile);
    void printGameGrid(ofstream& outfile);
    void printFinalGrid(ofstream& outfile);
    void printGridBeforeClearing(ofstream& outfile);
    void printLeaderBoard(ofstream& outfile);
    void printScores(ofstream& outfile);
    void printGameOverBlock(ofstream &outfile);
    void activateNextBlock(ofstream& outfile);
    void printFinishGameNoMoreBlocksLeft(ofstream& outfile);
    void endGame(ofstream& outfile);
    int calculateDropBonus();
    int calculatePowerUpBonus();
    int getNumberOfOccupiedCellsByBlock();
};



#endif // BLOCKFALL_H
