#ifndef _SCENE_H
#define _SCENE_H

#include<_common.h>
#include<_lighting.h>
#include<_model.h>
#include<_inputs.h>
#include<_texloader.h>
#include<_skybox.h>
#include<_camera.h>
#include <vector>
#include "_modelRat.h"
#include "_food.h"

class _Scene
{
    public:
        _Scene();         //constructor
        virtual ~_Scene();//destructor

        GLint initGL();    // Initializing my game objects and opengl
        void reSize(GLint,GLint); //window resizing
        void drawScene();  //rendering the scene
        void WndProc(
              HWND	hWnd,			// Handle For This Window
			  UINT	uMsg,			// Message For This Window
			  WPARAM wParam,		// Additional Message Information
			  LPARAM lParam);		// Additional Message Information
        void handleCollisions();

        void handlePlayerCollisions(); // melon-player collision
        void updatePlayer(float dt);
        void checkFoodInHole();

        _lighting* myLight;
        _model* myModel;
        _inputs* keyMS;
        _texLoader* myTex;
        _skyBox* sky;
        _camera* cam;

        _modelRat* playerModel; // new controllable sphere
        std::vector<_food*> foods;   // all falling foods

        glm::vec3 mouseHolePos;   // position of the mouse hole in world coordinates
        float mouseHoleRadius;    // radius for detecting food entering hole

    protected:
    private:
};

#endif // _SCENE_H
