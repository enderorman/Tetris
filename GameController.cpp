#include "GameController.h"
#include <fstream>
bool GameController::play(BlockFall& game, const string& commands_file){
    ofstream outfile("compare.out");
    // TODO: Implement the gameplay here while reading the commands from the input file given as the 3rd command-line
    //       argument. The return value represents if the gameplay was successful or not: false if game over,
    //       true otherwise.

    ifstream file(commands_file);
    if (file.is_open()){
        string line;
        while (getline(file, line)){
            string command = line;
            outfile << command << endl;
            if (game.initial_block == nullptr){
                game.printFinishGameNoMoreBlocksLeft(outfile);
                return true;
            }
            else if (game.checkCollisionEnteringGrid()){
                game.printCollisionGameOver(outfile);
                return false;
            }
            else if (command == "PRINT_GRID"){
                printGrid(game, outfile);
            }
            else if (command == "ROTATE_RIGHT"){
                rotateRight(game);
            }
            else if (command == "ROTATE_LEFT"){
                rotateLeft(game);
            }
            else if (command == "MOVE_RIGHT"){
                moveRight(game);
            }
            else if (command == "MOVE_LEFT"){
                moveLeft(game);
            }
            else if (command == "DROP"){
                drop(game, outfile);
            }
            else if (command == "GRAVITY_SWITCH"){
                game.gravity_mode_on = !game.gravity_mode_on;
                if (game.gravity_mode_on){
                    game.activateGravitySwitch(outfile);
                }
            }
            else{
                printUnknownCommand(outfile, command);
            }
        }
        game.printFinishGameNoMoreCommands(outfile);
    }
    outfile.close();
    return true;
}

void GameController::printGrid(BlockFall &game, ofstream& outfile) {
    game.printGameGrid(outfile);
}

void GameController::rotateRight(BlockFall& game){
    Block* rightRotationBlock = game.active_rotation->right_rotation;
    vector<vector<bool>> rightRotationMatrix = rightRotationBlock->shape;
    if (game.checkBlockPositionIsOkay(rightRotationMatrix, game.currentBlockStartRow, game.currentBlockStartCol)){
        game.active_rotation= rightRotationBlock;
    }
}


void GameController::rotateLeft(BlockFall& game){
    Block* leftRotationBlock = game.active_rotation->left_rotation;
    vector<vector<bool>> leftRotationMatrix = leftRotationBlock->shape;
    if (game.checkBlockPositionIsOkay(leftRotationMatrix, game.currentBlockStartRow, game.currentBlockStartCol)){
        game.active_rotation = leftRotationBlock;
    }
}

void GameController::moveRight(BlockFall& game){
    Block* currentBlock = game.active_rotation;
    vector<vector<bool>> currentBlockMatrix = currentBlock->shape;
    if (game.checkBlockPositionIsOkay(currentBlockMatrix, game.currentBlockStartRow, game.currentBlockStartCol + 1)){
        game.currentBlockStartCol++;
    }
}

void GameController::moveLeft(BlockFall& game){
    Block* currentBlock = game.active_rotation;
    vector<vector<bool>> currentBlockMatrix = currentBlock->shape;
    if (game.checkBlockPositionIsOkay(currentBlockMatrix, game.currentBlockStartRow, game.currentBlockStartCol - 1)){
        game.currentBlockStartCol--;
    }
}

void GameController::drop(BlockFall& game, ofstream& outfile){
    game.dropBlockOnGrid(outfile);
}

void GameController::printUnknownCommand(ofstream &outfile, string command) {
    cout << "Unknown command: " << command << "\n";
    outfile << "Unknown command: " << command << "\n";
}







