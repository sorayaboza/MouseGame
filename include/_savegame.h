#ifndef _SAVEGAME_H
#define _SAVEGAME_H

#include <string>
#include <vector>
#include <glm/vec3.hpp>

// ================================================================
//  _savegame  –  3-slot save/load system
//
//  Each slot is a binary file: save_slot1.dat / save_slot2.dat /
//  save_slot3.dat in the working directory.
//
//  File layout (all little-endian, packed):
//    int32  magic            ('MHSV' = 0x5648534D in little-endian)
//    int32  version          (currently 1)
//    int32  level            (1..3)
//    int32  score
//    float  player_x, player_y, player_z
//    int32  num_foods
//    foreach food:
//        int32  food_type (0..4)
//        float  display_size
//        float  pos_x, pos_y, pos_z
//        float  vel_x, vel_y, vel_z
//    int32  num_dogs
//    float  cat_x, cat_y, cat_z, cat_facing
//    foreach dog:
//        float  pos_x, pos_y, pos_z, facing
// ================================================================

struct SavedFood {
    int       foodType;     // 0=banana,1=cheese,2=donut,3=milk,4=watermelon
    float     displaySize;
    glm::vec3 pos;
    glm::vec3 velocity;
    std::string texturePath;
};

struct SavedEnemy {
    glm::vec3 pos;
    float     facingAngle;  // radians
};

struct SaveData {
    int                       level;
    int                       score;
    glm::vec3                 playerPos;
    std::vector<SavedFood>    foods;
    SavedEnemy                cat;
    std::vector<SavedEnemy>   dogs;
};

// Slot metadata (used by save-menu UI to show what's in each slot)
struct SlotInfo {
    bool exists;
    int  level;
    int  score;
};

class _savegame {
public:
    static const int NUM_SLOTS = 3;

    // Returns the save file path for a slot (1-based: 1, 2, or 3).
    static std::string slotPath(int slot);

    // Save current state to slot.  Returns true on success.
    static bool save(int slot, const SaveData& data);

    // Load slot into out.  Returns true on success.
    static bool load(int slot, SaveData& out);

    // Quick existence + summary check (used by menu UI).
    static SlotInfo peek(int slot);

    // Delete a save slot (returns true if file existed and was removed).
    static bool erase(int slot);
};

#endif
