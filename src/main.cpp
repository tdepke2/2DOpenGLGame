/*
 *  CSCI 441 Computer Graphics, Fall 2017
 *
 *  Project: sq1
 *  File: main.cpp
 *
 *  Author: Thomas Depke - Fall 2017
 *
 *  Description:
 *		https://opengameart.org/content/animated-top-down-survivor-player
 *      https://opengameart.org/content/desert-tileset-1
 *      https://opengameart.org/content/animated-top-down-zombie
 */

#include <GLFW/glfw3.h>		// include GLFW framework header

#ifdef __APPLE__			// if compiling on Mac OS
	#include <OpenGL/gl.h>
#else						// else compiling on Linux OS
	#include <GL/gl.h>
#endif

// include GLM libraries and matrix functions
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <stdio.h>				// for printf functionality
#include <stdlib.h>				// for exit functionality

#include "Character.h"
#include "TextureRect.h"
#include "TileMap.h"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <list>
#include <stdexcept>

using namespace std;

//*************************************************************************************
//
// Global Parameters

// global variables to keep track of window width and height.
// set to initial values for convenience, but we need variables
// for later on in case the window gets resized.
int WINDOW_WIDTH = 512, WINDOW_HEIGHT = 512;

Character player;
TileMap levelMap;
glm::vec2 lastMousePosition(0.0f, 0.0f);

struct Bullet {
    float x, y, r;
    int life;
    
    Bullet() {}
    
    Bullet(float x, float y, float r, int life) {
        this->x = x;
        this->y = y;
        this->r = r;
        this->life = life;
    }
    
    void step() {
        x += cos(r) * 6.0f;
        y += sin(r) * 6.0f;
        --life;
    }
    
    void draw() const {
        glm::mat4 transMtx = glm::translate(glm::mat4(1.0f), glm::vec3(x, y, 0.0f));
        glMultMatrixf(&transMtx[0][0]); {
            glm::mat4 rotateMtx = glm::rotate(glm::mat4(1.0f), r, glm::vec3(0.0f, 0.0f, 1.0f));
            glMultMatrixf(&rotateMtx[0][0]); {
                glBegin(GL_TRIANGLE_STRIP); {
                    glColor3ub(150, 150, 150);
                    glVertex2f(-10.0f, -3.0f);
                    glVertex2f(10.0f, -3.0f);
                    glVertex2f(-10.0f, 3.0f);
                    glVertex2f(10.0f, 3.0f);
                }; glEnd();
            }; glMultMatrixf(&(glm::inverse(rotateMtx))[0][0]);
        }; glMultMatrixf(&(glm::inverse(transMtx))[0][0]);
    }
};
list<Bullet> bullets;

//*************************************************************************************
//
// Event Callbacks

static void error_callback(int error, const char* description) {
	fprintf(stderr, "[ERROR]: %s\n", description);
}

void keyboard_callback( GLFWwindow *win, int key, int scancode, int action, int mods ) {
    if (key == GLFW_KEY_1 && action == GLFW_PRESS) {
        
    }
}

void mouse_button_callback( GLFWwindow *window, int button, int action, int mods ) {
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        
    }
}

void cursor_callback( GLFWwindow *window, double x, double y ) {
    lastMousePosition.x = static_cast<float>(x);
    lastMousePosition.y = static_cast<float>(y);
}

//*************************************************************************************
//
// Setup Functions

//
//  void setupGLFW()
//
//      Used to setup everything GLFW related.  This includes the OpenGL context
//	and our window.
//
GLFWwindow* setupGLFW() {
	// set what function to use when registering errors
	// this is the ONLY GLFW function that can be called BEFORE GLFW is initialized
	// all other GLFW calls must be performed after GLFW has been initialized
	glfwSetErrorCallback( error_callback );

	// initialize GLFW
	if (!glfwInit()) {
		fprintf( stderr, "[ERROR]: Could not initialize GLFW\n" );
		exit(EXIT_FAILURE);
	} else {
		fprintf( stdout, "[INFO]: GLFW initialized\n" );
	}

	glfwWindowHint( GLFW_DOUBLEBUFFER, GLFW_TRUE  );	// use double buffering
	glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 2 );	// request OpenGL v2.X
	glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 1 );	// request OpenGL v2.1
	glfwWindowHint( GLFW_RESIZABLE, GLFW_FALSE );		// do not allow our window to be able to be resized

	// create a window for a given size, with a given title
	GLFWwindow *window = glfwCreateWindow( WINDOW_WIDTH, WINDOW_HEIGHT, "Top-down Zombies", NULL, NULL );
	if( !window ) {
		glfwTerminate();
		exit(EXIT_FAILURE);
	} else {
		fprintf( stdout, "[INFO]: GLFW Window created\n" );
	}
    
    glfwSetKeyCallback( window, keyboard_callback );    // Setup event callbacks.
    glfwSetMouseButtonCallback( window, mouse_button_callback );
    glfwSetCursorPosCallback( window, cursor_callback );

	glfwMakeContextCurrent(window);		// make the created window the current window
	glfwSwapInterval(1);				// update our window after at least 1 screen refresh

	return window;						// return the window that was created
}

//
//  void setupOpenGL()
//
//      Used to setup everything OpenGL related.  For now, the only setting
//	we need is what color to make the background of our window when we clear
//	the window.  In the future we will be adding many more settings to this
//	function.
//
void setupOpenGL() {
	glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );	// set the clear color to black
}

//*************************************************************************************
//
// Rendering / Drawing Functions - this is where the magic happens!



//
//	void renderScene()
//
//		This method will contain all of the objects to be drawn.
//
void renderScene() {
    glm::mat4 transMtx = glm::translate(glm::mat4(1.0f), glm::vec3(-player.position.x, -player.position.y, 0.0f));
    glMultMatrixf(&transMtx[0][0]); {
        levelMap.draw();
    }; glMultMatrixf(&(glm::inverse(transMtx))[0][0]);
    
    for (const Bullet& b : bullets) {
        b.draw();
    }
    
    player.draw();
}

void nextTick(GLFWwindow* window) {    // Update simulation objects.
    glm::vec2 accel(0.0f, 0.0f);    // Check player movement.
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
        accel.y += 0.2f;
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
        accel.x -= 0.2f;
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
        accel.y -= 0.2f;
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
        accel.x += 0.2f;
    }
    
    float maxVelocity = 3.0f;    // Move player.
    if (accel.x != 0.0f) {
        if (accel.x < 0.0f) {
            player.velocity.x = max(player.velocity.x + accel.x, -maxVelocity);
        } else {
            player.velocity.x = min(player.velocity.x + accel.x, maxVelocity);
        }
    } else {
        player.velocity.x *= 0.8f;
    }
    if (accel.y != 0.0f) {
        if (accel.y < 0.0f) {
            player.velocity.y = max(player.velocity.y + accel.y, -maxVelocity);
        } else {
            player.velocity.y = min(player.velocity.y + accel.y, maxVelocity);
        }
    } else {
        player.velocity.y *= 0.8f;
    }
    player.position.x += player.velocity.x;
    player.position.y += player.velocity.y;
    
    if (WINDOW_WIDTH / 2.0f - lastMousePosition.x != 0.0f) {
        float lookAngle = atan((WINDOW_HEIGHT / 2.0f - (WINDOW_HEIGHT - lastMousePosition.y)) / (WINDOW_WIDTH / 2.0f - lastMousePosition.x)) + (WINDOW_WIDTH / 2.0f - lastMousePosition.x > 0.0f ? acos(-1.0f) : 0.0f);
        player.direction = lookAngle;
        player.body.rotation = lookAngle;
    }
    
    for (auto bulletIter = bullets.begin(); bulletIter != bullets.end();) {
        if (bulletIter->life <= 0) {
            bulletIter = bullets.erase(bulletIter);
        } else {
            bulletIter->step();
            ++bulletIter;
        }
    }
}

//*************************************************************************************
//
// Our main function

//
//	int main( int argc, char *argv[] )
//
//		Really you should know what this is by now.  We will make use of the parameters later
//
int main( int argc, char* argv[] ) {
	GLFWwindow *window = setupGLFW();	// initialize all of the GLFW specific information related to OpenGL and our window
										// GLFW sets up our OpenGL context so must be done first
	setupOpenGL();						// initialize all of the OpenGL specific information
    
    try {
        player.body.texture = loadTexture("map15.png");
        player.body.position = glm::vec2(WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT / 2.0f);
        player.body.size = glm::vec2(100.0f, 100.0f);
        player.body.centerOrigin();
        
        levelMap.loadMap("levels/level0.csv", loadTexture("tileset.png"), glm::uvec2(256, 256), glm::uvec2(32, 32));

        //  This is our draw loop - all rendering is done here.  We use a loop to keep the window open
        //	until the user decides to close the window and quit the program.  Without a loop, the
        //	window will display once and then the program exits.
        while( !glfwWindowShouldClose(window) ) {
            glDrawBuffer( GL_BACK );		// ensure we are drawing to the back buffer
            glClear( GL_COLOR_BUFFER_BIT );	// clear the current color contents in the buffer

            // update the projection matrix based on the window size
            // the GL_PROJECTION matrix governs properties of the view coordinates;
            // i.e. what gets seen - use an Orthographic projection that ranges
            // from [0, windowWidth] in X and [0, windowHeight] in Y. (0,0) is the lower left.
            glm::mat4 projMtx = glm::ortho( 0.0f, (GLfloat)WINDOW_WIDTH, 0.0f, (GLfloat)WINDOW_HEIGHT );
            glMatrixMode( GL_PROJECTION );	// change to the Projection matrix
            glLoadIdentity();				// set the matrix to be the identity
            glMultMatrixf( &projMtx[0][0] );// load our orthographic projection matrix into OpenGL's projection matrix state

            // Get the size of our framebuffer.  Ideally this should be the same dimensions as our window, but
            // when using a Retina display the actual window can be larger than the requested window.  Therefore
            // query what the actual size of the window we are rendering to is.
            GLint framebufferWidth, framebufferHeight;
            glfwGetFramebufferSize( window, &framebufferWidth, &framebufferHeight );

            // update the viewport - tell OpenGL we want to render to the whole window
            glViewport( 0, 0, framebufferWidth, framebufferHeight );

            glMatrixMode( GL_MODELVIEW );	// make the ModelView matrix current to be modified by any transformations
            glLoadIdentity();				// set the matrix to be the identity

            renderScene();					// draw everything to the window

            glfwSwapBuffers(window);		// flush the OpenGL commands and make sure they get rendered!
            glfwPollEvents();				// check for any events and signal to redraw screen
            
            nextTick(window);
        }
    } catch (exception& ex) {
        cout << "\n****************************************************" << endl;
        cout << "* A fatal error has occurred, terminating program. *" << endl;
        cout << "****************************************************" << endl;
        cout << "Error: " << ex.what() << endl;
        cout << "(Press enter)" << endl;
        cin.get();
        return -1;
    }
    
	return 0;
}
