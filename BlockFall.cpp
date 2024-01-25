#include "BlockFall.h"
#include <fstream>
#include <iostream>
#include <algorithm>


BlockFall::BlockFall(string grid_file_name, string blocks_file_name, bool gravity_mode_on, const string &leaderboard_file_name, const string &player_name) : gravity_mode_on(
        gravity_mode_on), leaderboard_file_name(leaderboard_file_name), player_name(player_name) {
    initialize_grid(grid_file_name);
    read_blocks(blocks_file_name);
    leaderboard.read_from_file(leaderboard_file_name);
}

void BlockFall::read_blocks(const string &input_file) {
    // TODO: Read the blocks from the input file and initialize "initial_block" and "active_rotation" member variables
    // TODO: For every block, generate its rotations and properly implement the multilevel linked list structure
    //       that represents the game blocks, as explained in the PA instructions.
    // TODO: Initialize the "power_up" member variable as the last block from the input file (do not add it to the linked list!)
    Block* prev = nullptr;
    Block* dummy = new Block;
    Block* currentBlock = dummy;
    ifstream file(input_file);
    vector<vector<bool>> matrix;
    if (file.is_open()){
        string line;
        vector<bool> row;
        while (getline(file, line)) {
            bool endBracket = false;
            if (line.empty() || line == "\n" || line == "\t"){
                continue;
            }
            for (char& c : line) {
                if (c == '['){
                    row.clear();
                }
                else if (c == ']'){
                    endBracket = true;
                }
                else if (isdigit(c)){
                    row.push_back(c - '0');
                }
            }
            if (!row.empty()){
                matrix.push_back(row);
            }
            row.clear();
            if (endBracket){
                Block* newBlock = new Block();
                newBlock->shape = matrix;
                Block* temp = newBlock;
                vector<vector<bool>> currentRotationMatrix = matrix;
                for (int i = 0; i < 3; i++) {
                    Block *nextRotationBlock = new Block();
                    vector<vector<bool>> nextRotationMatrix = getRightRotatedMatrix(currentRotationMatrix);
                    nextRotationBlock->shape = nextRotationMatrix;
                    currentRotationMatrix = nextRotationMatrix;
                    temp->right_rotation = nextRotationBlock;
                    nextRotationBlock->left_rotation = temp;
                    temp = nextRotationBlock;
                }

                temp->right_rotation = newBlock;
                newBlock->left_rotation = temp;

                temp = currentBlock;
                if (temp->right_rotation != nullptr){
                    for (int i = 0; i < 4; i++){
                        temp->next_block = newBlock;
                        temp = temp->right_rotation;
                    }
                }
                else{
                    temp->next_block = newBlock;
                }
                matrix.clear();
                prev = currentBlock;
                currentBlock = currentBlock->next_block;
            }
        }
        power_up = currentBlock->shape;

    }
    file.close();
    for(int i=0; i<4; i++){
        Block* del=currentBlock;
        currentBlock=currentBlock->right_rotation;
        delete del;
    }
    currentBlock= nullptr;
    for(int i=0; i<4; i++){
        prev->next_block= nullptr;
        prev=prev->right_rotation;
    }
    initial_block = dummy->next_block;
    active_rotation = initial_block;
    delete dummy;
}

void BlockFall::initialize_grid(const string &input_file) {
    // TODO: Initialize "rows" and "cols" member variables
    // TODO: Initialize "grid" member variable using the command-line argument 1 in main
    ifstream file(input_file);
    if (file.is_open()){
        string line;
        vector<int> row;
        while (getline(file, line)){
            for (char& c: line){
                if (isdigit(c)){
                    row.push_back(c - '0');
                }
            }
            grid.push_back(row);
            row.clear();
        }
    }
    file.close();
    rows = grid.size();
    cols = grid.at(0).size();
}



vector<vector<bool>> BlockFall::getRightRotatedMatrix(vector<vector<bool>>& matrix){
    int height = matrix.size(), width = matrix[0].size();
    vector<vector<bool>> rightRotatedMatrix(width, vector<bool>(height));
    for (int j = 0; j < width; j++){
        for (int  i = height - 1; i >= 0; i--){
            rightRotatedMatrix[j][height - i - 1] = matrix[i][j];
        }
    }
    return rightRotatedMatrix;
}


bool BlockFall::checkBlockPositionIsOkay(vector<vector<bool>>& currentBlockMatrix, int startRow, int startCol){
    int blockHeight = currentBlockMatrix.size(), blockWidth = currentBlockMatrix.at(0).size();
    for (int i = startRow; i < startRow + blockHeight; i++){
        for (int j = startCol; j < startCol + blockWidth; j++){
            if ((checkIfBlockIsOutOfBonds(i, j) || grid[i][j] && currentBlockMatrix[i - startRow][j - startCol])){
                return false;
            }
        }
    }
    return true;
}

bool BlockFall::checkIfBlockIsOutOfBonds(int i, int j){
    if (i < 0 || i >= rows || j < 0 || j >= cols){
        return true;
    }
    return false;
}

pair<int, int> BlockFall::getNoGravityDropPosition(){
    vector<vector<bool>> currentMatrix = active_rotation->shape;
    int blockHeight = currentMatrix.size(), blockWidth = currentMatrix.at(0).size();
    int dropStartRow = 0;
    for (int i = 0; i + blockHeight <= rows; i++){
        if (checkBlockPositionIsOkay(currentMatrix, i, currentBlockStartCol)){
            dropStartRow = max(dropStartRow, i);
        }
        else{
            break;
        }
    }
    return make_pair(dropStartRow, currentBlockStartCol);
}


void BlockFall::dropBlockOnGrid(ofstream& outfile){
    dropIsActive = true;
    pair<int, int> dropPosition;
    current_score += calculateDropBonus();
    dropPosition = getNoGravityDropPosition();
    int dropStartRow = dropPosition.first, dropStartCol = dropPosition.second;
    currentBlockStartRow = dropStartRow, currentBlockStartCol = dropStartCol;
    locateBlock(grid);
    if (gravity_mode_on){
        activateGravitySwitch(outfile);
        activateNextBlock(outfile);
        dropIsActive = false;
        return;
    }
    if (checkIfThereIsPowerUp()){
        printGridBeforeClearing(outfile);
    }
    updateGameStatementIfThereIsPowerUp();
    if (checkIfThereAreFullRows()){
        printGridBeforeClearing(outfile);
    }
    updateGameStatementIfThereAreFullRows();
    activateNextBlock(outfile);
    dropIsActive = false;
}

void BlockFall::activateGravitySwitch(ofstream& outfile){
    for (int i = rows - 1 ; i >= 0; i--){
        for (int j = 0; j < cols; j++){
            if (!grid[i][j]){
                continue;
            }
            grid[i][j] = 0;
            for (int k = rows - 1; k >= 0; k--){
                if (!grid[k][j]){
                    grid[k][j] = 1;
                    break;
                }
            }
        }
    }
    if (checkIfThereIsPowerUp() && dropIsActive){
        printGridBeforeClearing(outfile);
    }
    updateGameStatementIfThereIsPowerUp();
    if (checkIfThereAreFullRows() && dropIsActive){
        printGridBeforeClearing(outfile);
    }
    updateGameStatementIfThereAreFullRows();
}
void BlockFall::locateBlock(vector<vector<int>>& matrix){
    vector<vector<bool>> currentBlockMatrix = active_rotation->shape;
    int blockHeight = currentBlockMatrix.size(), blockWidth = currentBlockMatrix.at(0).size();
    for (int i = currentBlockStartRow; i < currentBlockStartRow + blockHeight; i++){
        for (int j = currentBlockStartCol; j < currentBlockStartCol + blockWidth; j++){
            if (currentBlockMatrix[i - currentBlockStartRow][j - currentBlockStartCol]){
                matrix[i][j] = 1;
            }
        }
    }
}


void BlockFall::activateNextBlock(ofstream& outfile){
    Block* nextBlock = initial_block->next_block;
    Block* temp = initial_block;
    for (int i = 0; i < 4; i++){
        Block* nextRotation = temp->right_rotation;
        delete temp;
        temp = nextRotation;
    }
    initial_block = nextBlock;
    active_rotation = initial_block;
    currentBlockStartRow = 0, currentBlockStartCol = 0;
}

bool BlockFall::checkCollisionEnteringGrid(){
    vector<vector<bool>> currentBlockMatrix = active_rotation->shape;
    return !checkBlockPositionIsOkay(currentBlockMatrix, currentBlockStartRow, currentBlockStartCol);
}


void BlockFall::updateGameStatementIfThereIsPowerUp(){
    if (checkIfThereIsPowerUp()){
        current_score += calculatePowerUpBonus();
        clearGrid();
    }
}

void BlockFall::updateGameStatementIfThereAreFullRows(){
    for (int rowIndex = 0; rowIndex < rows; rowIndex++){
        if (checkIfRowIsFull(rowIndex)){
            current_score += cols;
            shiftGrid(rowIndex);
        }
    }
}

bool BlockFall::checkIfThereAreFullRows() {
    for (int rowIndex = 0; rowIndex < rows; rowIndex++) {
        if (checkIfRowIsFull(rowIndex)) {
            return true;
        }
    }
    return false;
}

bool BlockFall::checkIfRowIsFull(int rowIndex){
    for (int j = 0; j < cols; j++){
        if (!grid[rowIndex][j]){
            return false;
        }
    }
    return true;
}

bool BlockFall::checkIfThereIsPowerUp(){
    int powerUpHeight = power_up.size(), powerUpWidth = power_up.at(0).size();
    for (int i = 0; i + powerUpHeight <= rows; i++){
        for (int j = 0; j + powerUpWidth <= cols; j++){
            if (checkPowerUpForGivenRange(i, j)){
                return true;
            }
        }
    }
    return false;
};

bool BlockFall::checkPowerUpForGivenRange(int startRow, int startCol){
    int powerUpHeight = power_up.size(), powerUpWidth = power_up.at(0).size();
    for (int i = startRow; i < startRow + powerUpHeight; i++){
        for (int j = startCol; j < startCol + powerUpWidth; j++){
            if (power_up[i - startRow][j - startCol] != grid[i][j]){
                return false;
            }
        }
    }
    return true;
}

int BlockFall::calculateDropBonus(){
    pair<int, int> dropPosition = getNoGravityDropPosition();
    int dist = (dropPosition.first - currentBlockStartRow);
    return dist * getNumberOfOccupiedCellsByBlock();
}

int BlockFall::calculatePowerUpBonus(){
    int powerUpBonus = 1000;
    for (int i = 0; i < rows; i++){
        for (int j = 0; j < cols; j++){
            powerUpBonus += grid[i][j];
        }
    }
    return powerUpBonus;
}

int BlockFall::getNumberOfOccupiedCellsByBlock(){
    int blockCount = 0;
    vector<vector<bool>> currentBlockShape = active_rotation->shape;
    int blockHeight = currentBlockShape.size(), blockWidth = currentBlockShape.at(0).size();
    for (int i = 0; i < blockHeight; i++){
        for (int j = 0; j < blockWidth; j++){
            blockCount += currentBlockShape[i][j];
        }
    }
    return blockCount;
}


void BlockFall::clearGrid(){
    for (int i = 0; i < rows; i++){
        for (int j = 0; j < cols; j++){
            grid[i][j] = 0;
        }
    }
}

void BlockFall::shiftGrid(int rowIndex){
    for (int i = rowIndex; i > 0; i--){
        for (int j = 0; j < cols; j++){
            grid[i][j] = grid[i - 1][j];
        }
    }
    std::fill(grid[0].begin(), grid[0].end(), 0);
}



void BlockFall::printGameGrid(ofstream& outfile){
    vector<vector<int>> gridCopy = grid;
    locateBlock(gridCopy);
    printScores(outfile);
    printMatrix(gridCopy, outfile);
}



void BlockFall::printGridBeforeClearing(ofstream& outfile){
    outfile << "Before clearing:" << endl;
    cout << "Before clearing:" << endl;
    printMatrix(grid, outfile);
}

void BlockFall::printGameOverBlock(ofstream &outfile){
    vector<vector<bool>> lastBlockShape = active_rotation->shape;
    int lastBlockHeight = lastBlockShape.size() , lastBlockWidth = lastBlockShape.at(0).size();
    for (int i = 0; i < lastBlockHeight; i++){
        for (int j = 0; j < lastBlockWidth; j++){
            if (lastBlockShape[i][j]){
                outfile << "⬜";
                cout << occupiedCellChar;
            }
            else{
                outfile << "⬛";
                cout << unoccupiedCellChar;
            }
        }
        cout << "\n";
        outfile << "\n";
    }
    cout << "\n\n";
    outfile << "\n\n";
}

void BlockFall::printFinalGrid(ofstream &outfile) {
    cout << "Final grid and score:" << "\n\n";
    outfile << "Final grid and score:" << "\n\n";
    printScores(outfile);
    printMatrix(grid, outfile);
}

void BlockFall::printScores(ofstream& outfile){
    outfile << "Score: " << current_score << endl;
    outfile << "High Score: " << leaderboard.getHighScore() << endl;
    cout << "Score: " << current_score << endl;
    cout << "High Score: " << endl;
}

void BlockFall::printMatrix(vector<vector<int>>& matrix, ofstream& outfile){
    int height = matrix.size(), width = matrix.at(0).size();
    for (int i = 0; i < height; i++){
        for (int j = 0; j < width; j++){
            if (matrix[i][j]){
                outfile << "⬜";
                cout << occupiedCellChar;
            }
            else{
                outfile << "⬛";
                cout << unoccupiedCellChar;
            }
        }
        outfile << endl;
        cout << endl;
    }
    outfile << "\n\n";
    cout << "\n\n";
}

void BlockFall::printLeaderBoard(ofstream& outfile){
    leaderboard.print_leaderboard();
}

void BlockFall::printCollisionGameOver(ofstream& outfile){
    cout << "GAME OVER!" << endl;
    cout << "Next block that couldn't fit:" << endl;
    outfile << "GAME OVER!" << endl;
    outfile << "Next block that couldn't fit:" << endl;
    printGameOverBlock(outfile);
    endGame(outfile);
}

void BlockFall::printFinishGameNoMoreCommands(ofstream& outfile){
    cout << "GAME FINISHED!" << "\n";
    outfile << "GAME FINISHED!" << "\n";
    cout << "No more commands." << "\n";
    outfile << "No more commands." << "\n";
    endGame(outfile);
}

void BlockFall::printFinishGameNoMoreBlocksLeft(ofstream& outfile){
    cout << "YOU WIN!" << "\n";
    outfile << "YOU WIN!" << "\n";
    cout << "No more blocks." << "\n";
    outfile << "No more blocks." << "\n";
    endGame(outfile);
}

void BlockFall::endGame(ofstream& outfile){
    printFinalGrid(outfile);
    printLeaderBoard(outfile);
}

BlockFall::~BlockFall() {
    // TODO: Free dynamically allocated memory used for storing game blocks
    if (initial_block == nullptr){
        return;
    }
    Block* current_block = initial_block;
    Block* tmp;
    while (current_block != nullptr) {
        Block* next_block = current_block->next_block;
        tmp = current_block;
        for (int i = 0; i < 4; ++i) {
            Block* next_rotation = tmp->right_rotation;
            delete tmp;
            tmp = next_rotation;
        }
        //
        current_block = next_block;
    }
}

























