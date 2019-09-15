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
 *      https://prayingmantis.itch.io/
 *      https://opengameart.org/content/ascii-bitmap-font-oldschool
 *      https://opengameart.org/content/boxy-bold-font-split
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
#include "Enemy.h"
#include "Projectile.h"
#include "TextureRect.h"
#include "TileMap.h"
#include <algorithm>
#include <chrono>
#include <cmath>
#include <ctime>
#include <iostream>
#include <list>
#include <stdexcept>
#include <thread>
#include <utility>

#ifndef PI
#define PI acos(-1.0f)
#endif

using namespace std;

//*************************************************************************************
//
// Global Parameters

// global variables to keep track of window width and height.
// set to initial values for convenience, but we need variables
// for later on in case the window gets resized.
int WINDOW_WIDTH = 768, WINDOW_HEIGHT = 768;

Character player;
list<Enemy> enemies;
vector<glm::vec2> spawners;
list<Projectile> projectiles;
TileMap levelMap, textBottomMap;
vector<vector<GLint>> playerBodyAnimations, playerFeetAnimations, enemyAnimations;
GLint spriteTextures, fontTexture;
vector<pair<int, int>> weaponAmmo, maximumAmmo;
vector<int> weaponCooldown;
int weaponCooldownCounter;
glm::vec2 lastMousePosition(0.0f, 0.0f);

//*************************************************************************************
//
// Event Callbacks

static void error_callback(int error, const char* description) {
	fprintf(stderr, "[ERROR]: %s\n", description);
}

void keyboard_callback( GLFWwindow *win, int key, int scancode, int action, int mods ) {
    if (key == GLFW_KEY_R && action == GLFW_PRESS) {
        if (weaponCooldownCounter == 0 && weaponAmmo[player.getBody() / 4].first < maximumAmmo[player.getBody() / 4].first && weaponAmmo[player.getBody() / 4].second > 0) {
            player.setBody((player.getBody() / 4) * 4 + 2);
        }
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

void scroll_callback( GLFWwindow* window, double xoffset, double yoffset) {
    player.setBody(((player.getBody() / 4 + static_cast<int>(-yoffset)) % 4) * 4);
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
    glfwSetScrollCallback( window, scroll_callback );

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
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void setupGame() {
    srand(static_cast<unsigned int>(time(NULL)));    // Still using rand() in 2019? Thats a big no no, oh well.
    spriteTextures = loadTexture("spritesheet.png");
    fontTexture = loadTexture("oldschool_font.png");
    
    playerBodyAnimations.push_back(loadAnimation("survivor/knife/idle/survivor-idle_knife_0.png", 0, 0));//19));
    playerBodyAnimations.push_back(loadAnimation("survivor/knife/move/survivor-move_knife_0.png", 0, 0));//19));
    playerBodyAnimations.push_back(vector<GLint>(0));    // You can't reload your knife! (but it would be cool)
    playerBodyAnimations.push_back(loadAnimation("survivor/knife/meleeattack/survivor-meleeattack_knife_0.png", 0, 0));//14));
    
    playerBodyAnimations.push_back(loadAnimation("survivor/handgun/idle/survivor-idle_handgun_0.png", 0, 0));//19));
    playerBodyAnimations.push_back(loadAnimation("survivor/handgun/move/survivor-move_handgun_0.png", 0, 0));//19));
    playerBodyAnimations.push_back(loadAnimation("survivor/handgun/reload/survivor-reload_handgun_0.png", 0, 14));
    playerBodyAnimations.push_back(loadAnimation("survivor/handgun/shoot/survivor-shoot_handgun_0.png", 0, 2));
    
    playerBodyAnimations.push_back(loadAnimation("survivor/rifle/idle/survivor-idle_rifle_0.png", 0, 0));//19));
    playerBodyAnimations.push_back(loadAnimation("survivor/rifle/move/survivor-move_rifle_0.png", 0, 0));//19));
    playerBodyAnimations.push_back(loadAnimation("survivor/rifle/reload/survivor-reload_rifle_0.png", 0, 19));
    playerBodyAnimations.push_back(loadAnimation("survivor/rifle/shoot/survivor-shoot_rifle_0.png", 0, 2));
    
    playerBodyAnimations.push_back(loadAnimation("survivor/shotgun/idle/survivor-idle_shotgun_0.png", 0, 0));//19));
    playerBodyAnimations.push_back(loadAnimation("survivor/shotgun/move/survivor-move_shotgun_0.png", 0, 0));//19));
    playerBodyAnimations.push_back(loadAnimation("survivor/shotgun/reload/survivor-reload_shotgun_0.png", 0, 0));//19));
    playerBodyAnimations.push_back(loadAnimation("survivor/shotgun/shoot/survivor-shoot_shotgun_0.png", 0, 2));
    
    playerFeetAnimations.push_back(loadAnimation("survivor/feet/idle/survivor-idle_0.png", 0, 0));
    playerFeetAnimations.push_back(loadAnimation("survivor/feet/run/survivor-run_0.png", 0, 0));//19));
    playerFeetAnimations.push_back(loadAnimation("survivor/feet/strafe_left/survivor-strafe_left_0.png", 0, 0));//19));
    playerFeetAnimations.push_back(loadAnimation("survivor/feet/strafe_right/survivor-strafe_right_0.png", 0, 0));//19));
    
    enemyAnimations.push_back(loadAnimation("zombie/skeleton-move_0.png", 0, 0));//16));
    enemyAnimations.push_back(loadAnimation("zombie/skeleton-attack_0.png", 0, 0));//8));
    enemyAnimations.push_back(vector<GLint>(1, loadTexture("zombie/splatter.png")));
    
    player.bodyAnimationsPtr = &playerBodyAnimations;
    player.feetAnimationsPtr = &playerFeetAnimations;
    player.setBody(4);
    player.setFeet(0);
    player.position = glm::vec2(0.0f, 0.0f);
    player.setSize(glm::vec2(110.0f, 100.0f));
    
    weaponAmmo.push_back(pair<int, int>(0, 0));
    weaponAmmo.push_back(pair<int, int>(10, 30));
    weaponAmmo.push_back(pair<int, int>(1000000, 100));
    weaponAmmo.push_back(pair<int, int>(5, 60));
    maximumAmmo.push_back(pair<int, int>(0, 0));
    maximumAmmo.push_back(pair<int, int>(10, 30));
    maximumAmmo.push_back(pair<int, int>(20, 100));
    maximumAmmo.push_back(pair<int, int>(5, 60));
    weaponCooldown.push_back(20);
    weaponCooldown.push_back(15);
    weaponCooldown.push_back(10);
    weaponCooldown.push_back(25);
    weaponCooldownCounter = 0;
    
    enemies.push_back(Enemy());
    enemies.back().bodyAnimationsPtr = &enemyAnimations;
    enemies.back().setBody(0);
    enemies.back().position = glm::vec2(100.0f, 0.0f);
    enemies.back().setSize(glm::vec2(130.0f, 130.0f));
    enemies.back().health = 100;
    enemies.back().targetPtr = &player;
    enemies.back().speed = 3.0f;
    
    levelMap.loadMap("levels/level0.csv", loadTexture("desert_tileset.png"), glm::uvec2(448, 128), glm::uvec2(32, 32));
    textBottomMap.loadFont(fontTexture, glm::uvec2(504, 216), glm::uvec2(28, 36));
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
    glm::mat4 transMtx = glm::translate(glm::mat4(1.0f), glm::vec3(-player.position.x + WINDOW_WIDTH / 2.0f, -player.position.y + WINDOW_HEIGHT / 2.0f, 0.0f));
    glMultMatrixf(&transMtx[0][0]); {
        glm::mat4 scaleMtx = glm::scale(glm::mat4(1.0f), glm::vec3(3.0f, 3.0f, 1.0f));
        glMultMatrixf(&scaleMtx[0][0]); {
            levelMap.draw();
        }; glMultMatrixf(&(glm::inverse(scaleMtx))[0][0]);
        
        for (const Enemy& enemy : enemies) {
            enemy.draw();
        }
        
        for (const Projectile& projectile : projectiles) {
            projectile.draw();
        }
        
        player.draw();
    }; glMultMatrixf(&(glm::inverse(transMtx))[0][0]);
    
    glm::mat4 scaleMtx2 = glm::scale(glm::mat4(1.0f), glm::vec3(0.7f, 0.7f, 1.0f));
    glMultMatrixf(&scaleMtx2[0][0]); {
        textBottomMap.draw();
    }; glMultMatrixf(&(glm::inverse(scaleMtx2))[0][0]);
}

string intToString(int x, unsigned int width) {
    string s = to_string(x);
    if (width > s.length()) {
        return string(width - s.length(), ' ') + s;
    } else {
        return s;
    }
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
    
    if (fabs(player.velocity.x) > 1.0f || fabs(player.velocity.y) > 1.0f) {
        if (player.getBody() % 4 == 0) {
            player.setBody(player.getBody() + 1);
        }
        float velocityAngle = 0.0f;
        if (player.velocity.x != 0.0f) {
            velocityAngle = atan(player.velocity.y / player.velocity.x) + (player.velocity.x > 0.0f ? 0.0f : PI);
        }
        float angleBetween = player.rotation - velocityAngle;    // Angle between player velocity and rotation, positive if velocity is to the right of look angle, negative otherwise.
        if (angleBetween > PI) {
            angleBetween = -2.0f * PI + angleBetween;
        } else if (angleBetween < -PI) {
            angleBetween = 2.0f * PI + angleBetween;
        }
        if (fabs(angleBetween) < PI * 0.25f || fabs(angleBetween) > PI * 0.75f) {
            player.setFeet(1);
        } else if (angleBetween > 0.0f) {
            player.setFeet(3);
        } else {
            player.setFeet(2);
        }
    } else {
        if (player.getBody() % 4 == 1) {
            player.setBody(player.getBody() - 1);
        }
        player.setFeet(0);
    }
    player.update();
    
    if (lastMousePosition.x - WINDOW_WIDTH / 2.0f != 0.0f) {
        float lookAngle = atan(((WINDOW_HEIGHT - lastMousePosition.y) - WINDOW_HEIGHT / 2.0f) / (lastMousePosition.x - WINDOW_WIDTH / 2.0f)) + (lastMousePosition.x - WINDOW_WIDTH / 2.0f > 0.0f ? 0.0f : PI);
        player.rotation = lookAngle;
    }
    
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS && weaponCooldownCounter == 0 && weaponAmmo[player.getBody() / 4].first > 0) {    // Check for weapon fire.
        Projectile bullet;
        bullet.texture = spriteTextures;
        bullet.texCoordBottomLeft = glm::vec2(0.0f, 0.0f);
        bullet.texCoordTopRight = glm::vec2(1.0f / 4.0f, 1.0f / 3.0f);
        bullet.position = glm::vec2(player.position.x + 50.0f * cos(player.rotation) - 26.0f * cos(player.rotation + PI / 2.0f), player.position.y + 50.0f * sin(player.rotation) - 26.0f * sin(player.rotation + PI / 2.0f));
        bullet.size = glm::vec2(100.0f, 5.0f);
        bullet.centerOrigin();
        bullet.rotation = player.rotation + 0.1f * static_cast<float>(rand()) / RAND_MAX;
        bullet.velocity = 50.0f;
        bullet.lifespan = 40;
        if (player.getBody() / 4 == 1) {
            bullet.health = 5;
            bullet.damage = 20;
            projectiles.push_back(bullet);
        } else if (player.getBody() / 4 == 2) {
            bullet.health = 30;
            bullet.damage = 25;
            projectiles.push_back(bullet);
        } else {
            bullet.health = 10;
            bullet.damage = 80;
            projectiles.push_back(bullet);
            bullet.rotation += 0.2f;
            projectiles.push_back(bullet);
            bullet.rotation -= 0.4f;
            projectiles.push_back(bullet);
        }
        
        weaponCooldownCounter = weaponCooldown[player.getBody() / 4];
        --weaponAmmo[player.getBody() / 4].first;
    } else if (weaponCooldownCounter > 0) {
        --weaponCooldownCounter;
    }
    
    if (player.getBody() % 4 == 2 && player.getBodyNumber() == 0) {    // Check if reload finished.
        int numBulletsReloaded = min(maximumAmmo[player.getBody() / 4].first - weaponAmmo[player.getBody() / 4].first, weaponAmmo[player.getBody() / 4].second);
        weaponAmmo[player.getBody() / 4].first += numBulletsReloaded;
        weaponAmmo[player.getBody() / 4].second -= numBulletsReloaded;
        player.setBody(player.getBody() - 2);
    }
    
    textBottomMap.loadText("Health:" + intToString(player.health, 4) + "      Ammo:" + intToString(weaponAmmo[player.getBody() / 4].first, 4) + "/" + intToString(weaponAmmo[player.getBody() / 4].second, 4));
    
    for (auto projectileIter = projectiles.begin(); projectileIter != projectiles.end();) {
        if (projectileIter->lifespan <= 0) {
            projectileIter = projectiles.erase(projectileIter);
        } else {
            projectileIter->update(enemies);
            ++projectileIter;
        }
    }
    for (auto enemyIter = enemies.begin(); enemyIter != enemies.end();) {
        if (enemyIter->lifespan <= 0) {
            enemyIter = enemies.erase(enemyIter);
        } else {
            enemyIter->update();
            ++enemyIter;
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
    
    //auto lastTime = chrono::high_resolution_clock::now();    // I put this here with an FPS limiter below to set a frame limit but it seems like OpenGL is already using vsync! nice
    try {
        setupGame();

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
            
            /*auto currentTime = chrono::high_resolution_clock::now();
            long long sleepTime = static_cast<long long>(1.0 / 60.0 * 1.0e6) - chrono::duration_cast<chrono::microseconds>(currentTime - lastTime).count();    // Pause a little bit to limit FPS and TPS to 60.
            if (sleepTime > 0) {
                this_thread::sleep_for(chrono::microseconds(sleepTime));
            }
            lastTime = currentTime;*/
            
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
