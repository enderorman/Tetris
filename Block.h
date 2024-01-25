#ifndef PA2_BLOCK_H
#define PA2_BLOCK_H

#include <vector>

using namespace std;

class Block {
public:

    vector<vector<bool>> shape; // Two-dimensional vector corresponding to the block's shape
    Block * right_rotation = nullptr; // Pointer to the block's clockwise neighbor block (its right rotation)
    Block * left_rotation = nullptr; // Pointer to the block's counter-clockwise neighbor block (its left rotation)
    Block * next_block = nullptr; // Pointer to the next block to appear in the game
    bool operator==(const Block& other) const {
        // TODO: Overload the == operator to compare two blocks based on their shapes
        int height = shape.size(), width = shape.at(0).size();
        vector<vector<bool>> otherShape = other.shape;
        for (int i = 0; i < height; i++){
            for (int j = 0; j < width; j++){
                if (otherShape[i][j] != shape[i][j]){
                    return false;
                }
            }
        }
        return true;
    }

    bool operator!=(const Block& other) const {
        // TODO: Overload the != operator to compare two blocks based on their shapes
        return !(*this == other);
    }
};


#endif //PA2_BLOCK_H
