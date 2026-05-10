// ================================================================
//  _food.cpp
//
//  PURPOSE:
//  Represents a single food object in the game world.
//
//  Food objects are:
//
//      - Collectible items for the player
//      - Physics-enabled world objects
//      - Billboard-rendered sprites/textures
//
//  Each food item stores:
//
//      - Its texture/image
//      - Display size
//      - Collision radius
//      - Physics state
//      - Food type identifier
//
//  HOW IT WORKS:
//
//      Constructor
//          Loads texture and initializes food properties
//
//      update()
//          Updates the food's physics position each frame
//
//      Destructor
//          Cleans up dynamically allocated texture memory
//
//  OTHER SYSTEM CONNECTIONS:
//
//      _foodsystem.cpp
//          Creates/manages collections of food objects
//
//      _scene.cpp
//          Updates and renders food through renderer systems
//
//      _physicsobject
//          Handles movement and floor collision physics
//
//      _renderer
//          Draws food billboards using texture + size data
//
// ================================================================

#include "_food.h"



// ================================================================
//  Constructor
//
//  PARAMETERS:
//
//      texPath
//          Path to the food texture image
//
//      dispSize
//          Visual render size of the food
//
//      type
//          Integer identifying food type
//
//          Example:
//              0 = banana
//              1 = cheese
//              etc.
//
//  PURPOSE:
//
//      Initializes a new food item with:
//
//          - texture
//          - size
//          - collision radius
//          - food type
//
// ================================================================
_food::_food(const std::string& texPath,
             float dispSize,
             int type)
{

    // ============================================================
    // Store food type identifier
    //
    // Used by gameplay systems to determine:
    //      - what food this is
    //      - what texture/type spawned
    // ============================================================
    foodType = type;



    // ============================================================
    // Store display size
    //
    // This controls:
    //      - rendered billboard size
    //      - overall visual scale
    // ============================================================
    displaySize = dispSize;



    // ============================================================
    // Collision radius
    //
    // Used for collision detection.
    //
    // Multiplied by 0.42f so collision volume better matches
    // the visible sprite instead of using the full render size.
    // ============================================================
    collisionRadius = dispSize * 0.42f;



    // ============================================================
    // Store texture path
    //
    // Useful for:
    //      - save/load systems
    //      - reloading textures later
    // ============================================================
    this->texturePath = texPath;



    // ============================================================
    // Create texture loader
    //
    // Texture is dynamically allocated because each food object
    // owns its own texture resource pointer.
    // ============================================================
    texture = new _texLoader();



    // ============================================================
    // Load the food image into OpenGL
    //
    // Example textures:
    //      banana.png
    //      cheese.png
    //      donut.png
    // ============================================================
    texture->loadTexture(texPath.c_str());
}



// ================================================================
//  Destructor
//
//  PURPOSE:
//
//      Frees dynamically allocated texture memory.
//
//  IMPORTANT:
//
//      Because texture was created using "new",
//      it MUST be deleted to avoid memory leaks.
// ================================================================
_food::~_food()
{

    delete texture;
}



// ================================================================
//  update()
//
//  PARAMETERS:
//
//      dt
//          Delta time (time since previous frame)
//
//      floorY
//          Y-position of the floor
//
//  PURPOSE:
//
//      Updates food physics every frame.
//
//  HOW IT WORKS:
//
//      physics.updatePhysics():
//
//          - updates velocity
//          - applies movement
//          - handles gravity/floor collision
//
//  WHY floorY + collisionRadius?
//
//      The food should rest ON TOP of the floor,
//      not with its center intersecting the floor.
//
//      So we offset upward by the collision radius.
// ================================================================
void _food::update(float dt, float floorY)
{

    // ============================================================
    // Update physics simulation
    //
    // Keeps food positioned properly on the floor while
    // still allowing movement/velocity interactions.
    // ============================================================
    physics.updatePhysics(

        dt,

        // Raise resting height so object sits on top
        // of the floor instead of clipping through it.
        floorY + collisionRadius
    );
}
