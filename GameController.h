#ifndef PA2_GAMECONTROLLER_H
#define PA2_GAMECONTROLLER_H

#include "BlockFall.h"
#include "iostream"

using namespace std;

class GameController {
public:
    bool win = false;
    bool play(BlockFall &game, const string &commands_file); // Function that implements the gameplay

    void printGrid(BlockFall& game, ofstream& outfile);

    void rotateRight(BlockFall& game);

    void rotateLeft(BlockFall& game);

    void moveRight(BlockFall& game);

    void moveLeft(BlockFall& game);

    void drop(BlockFall& game, ofstream& outfile);

    void gravitySwitch(BlockFall& game);

    void printUnknownCommand(ofstream& outfile, string command);

};


#endif //PA2_GAMECONTROLLER_H


