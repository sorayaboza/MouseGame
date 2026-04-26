#ifndef _SCENE_H
#define _SCENE_H

#include <vector>
#include<_common.h>
#include<_foodsystem.h>
#include<_lighting.h>
#include<_inputs.h>
#include<_texloader.h>
#include<_skybox.h>
#include<_camera.h>
#include<_food.h>
#include<_player.h>
#include<_ui.h>
#include<_abilities.h>
#include<_renderer.h>

class _Scene
{
    public:
        _Scene();         //constructor
        virtual ~_Scene();//destructor

        GLint initGL();    // Initializing my game objects and opengl
        void reSize(GLint,GLint); //window resizing
        void drawScene();  //rendering the scene
        void updateScene(float dt);
        void WndProc(
              HWND	hWnd,			// Handle For This Window
			  UINT	uMsg,			// Message For This Window
			  WPARAM wParam,		// Additional Message Information
			  LPARAM lParam);		// Additional Message Information
        void handleCollisions();

        void handlePlayerCollisions(); // melon-player collision
        void updatePlayer(float dt);
        void checkFoodInHole(float dt);

        _lighting* myLight;
        _inputs* keyMS;
        _texLoader* myTex;
        _skyBox* sky;
        _camera* cam;
        _player* player;
        _food* heldFood = nullptr;
        _foodsystem* foodSystem;
        _ui* ui;
        _abilities* abilities;
        _renderer* renderer;

        glm::vec3 mouseHolePos;   // position of the mouse hole in world coordinates
        float mouseHoleRadius;    // radius for detecting food entering hole

    protected:
    private:
};

#endif // _SCENE_H
