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

#include "Animation.h"
#include "Character.h"
#include "Enemy.h"
#include "Item.h"
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
const int ENEMY_CAP = 500;    // Max number of enemies on screen at a time.

Character player;    // Ok ok so this is way too many globals, next time I will use a Game class to manage this.
list<Enemy> enemies;
list<Projectile> projectiles;
TileMap levelMap, textBottomMap, textCenterMap, textTopMap;
vector<vector<glm::vec2>> levelPositionsData;
Item pistolItem, rifleItem, shotgunItem;
list<Item*> itemDrops;
vector<Animation> playerBodyAnimations, playerFeetAnimations, enemyAnimations;
pair<GLint, glm::uvec2> spriteTextures, fontTexture;
vector<pair<int, int>> weaponAmmo, maximumAmmo;
vector<int> weaponCooldown;
int weaponCooldownCounter, playerCash, playerScore, redFlashAlpha;
int roundNumber, roundCooldownCounter, enemiesRemaining, startOfRoundCooldown;
bool gameOver;
glm::vec2 lastMousePosition(0.0f, 0.0f);

void setupGame();

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
    } else if (key == GLFW_KEY_E && action == GLFW_PRESS) {
        if (sqrt(pow(player.position.x - pistolItem.position.x, 2.0f) + pow(player.position.y - pistolItem.position.y, 2.0f)) < 100.0f) {
            if (playerCash >= 10 && weaponAmmo[1].first < maximumAmmo[1].first || weaponAmmo[1].second < maximumAmmo[1].second) {
                playerCash -= 10;
                weaponAmmo[1].first = maximumAmmo[1].first;
                weaponAmmo[1].second = maximumAmmo[1].second;
                player.setBody(4);
            }
        } else if (sqrt(pow(player.position.x - rifleItem.position.x, 2.0f) + pow(player.position.y - rifleItem.position.y, 2.0f)) < 100.0f) {
            if (playerCash >= 90 && weaponAmmo[2].first < maximumAmmo[2].first || weaponAmmo[2].second < maximumAmmo[2].second) {
                playerCash -= 90;
                weaponAmmo[2].first = maximumAmmo[2].first;
                weaponAmmo[2].second = maximumAmmo[2].second;
                player.setBody(8);
            }
        } else if (sqrt(pow(player.position.x - shotgunItem.position.x, 2.0f) + pow(player.position.y - shotgunItem.position.y, 2.0f)) < 100.0f) {
            if (playerCash >= 80 && weaponAmmo[3].first < maximumAmmo[3].first || weaponAmmo[3].second < maximumAmmo[3].second) {
                playerCash -= 80;
                weaponAmmo[3].first = maximumAmmo[3].first;
                weaponAmmo[3].second = maximumAmmo[3].second;
                player.setBody(12);
            }
        }
    } else if (key == GLFW_KEY_ENTER && action == GLFW_PRESS) {
        if (gameOver) {
            setupGame();
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
    int currentWeapon = (player.getBody() / 4 + (yoffset < 0.0 ? 1 : -1) + 4) % 4;
    while (currentWeapon != 0 && weaponAmmo[currentWeapon].first == 0 && weaponAmmo[currentWeapon].second == 0) {
        currentWeapon = (currentWeapon + (yoffset < 0.0 ? 1 : -1)) % 4;
    }
    player.setBody(currentWeapon * 4);
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

void loadResources() {    // Load file resources for textures/fonts/etc.
    cout << "Controls: Use WASD to move, mouse to aim, left click to attack, R to reload, and E to interact/purchase item." << endl << endl;
    cout << "Please wait, loading resources.";
    srand(static_cast<unsigned int>(time(NULL)));    // Still using rand() in 2019? Thats a big no no, oh well.
    spriteTextures = loadTexture("spritesheet.png");
    fontTexture = loadTexture("oldschool_font.png");
    
    cout << "\n    Player animations...";
    playerBodyAnimations.emplace_back("survivor/knife/idle/survivor-idle_knife_0.png", 0, 0, 1);//19));
    playerBodyAnimations.emplace_back("survivor/knife/move/survivor-move_knife_0.png", 0, 0, 1);//19));
    playerBodyAnimations.emplace_back(vector<GLint>(0), glm::vec2(0.0f, 0.0f), 1);    // You can't reload your knife! (but it would be cool)
    playerBodyAnimations.emplace_back("survivor/knife/meleeattack/survivor-meleeattack_knife_0.png", 0, 14, 1);
    
    cout << ".";
    playerBodyAnimations.emplace_back("survivor/handgun/idle/survivor-idle_handgun_0.png", 0, 19, 1);
    playerBodyAnimations.emplace_back("survivor/handgun/move/survivor-move_handgun_0.png", 0, 19, 1);
    playerBodyAnimations.emplace_back("survivor/handgun/reload/survivor-reload_handgun_0.png", 0, 14, 3);
    playerBodyAnimations.emplace_back("survivor/handgun/shoot/survivor-shoot_handgun_0.png", 0, 2, 1);
    
    cout << ".";
    playerBodyAnimations.emplace_back("survivor/rifle/idle/survivor-idle_rifle_0.png", 0, 0, 1);//19));
    playerBodyAnimations.emplace_back("survivor/rifle/move/survivor-move_rifle_0.png", 0, 0, 1);//19));
    playerBodyAnimations.emplace_back("survivor/rifle/reload/survivor-reload_rifle_0.png", 0, 19, 4);
    playerBodyAnimations.emplace_back("survivor/rifle/shoot/survivor-shoot_rifle_0.png", 0, 2, 1);
    
    cout << ".";
    playerBodyAnimations.emplace_back("survivor/shotgun/idle/survivor-idle_shotgun_0.png", 0, 0, 1);//19));
    playerBodyAnimations.emplace_back("survivor/shotgun/move/survivor-move_shotgun_0.png", 0, 0, 1);//19));
    playerBodyAnimations.emplace_back("survivor/shotgun/reload/survivor-reload_shotgun_0.png", 0, 19, 5);
    playerBodyAnimations.emplace_back("survivor/shotgun/shoot/survivor-shoot_shotgun_0.png", 0, 2, 1);
    
    cout << ".";
    playerFeetAnimations.emplace_back("survivor/feet/idle/survivor-idle_0.png", 0, 0, 0);
    playerFeetAnimations.emplace_back("survivor/feet/run/survivor-run_0.png", 0, 19, 0);
    playerFeetAnimations.emplace_back("survivor/feet/strafe_left/survivor-strafe_left_0.png", 0, 19, 0);
    playerFeetAnimations.emplace_back("survivor/feet/strafe_right/survivor-strafe_right_0.png", 0, 19, 0);
    
    cout << "\n    Enemy animations...";
    enemyAnimations.emplace_back("zombie/skeleton-move_0.png", 0, 16, 1);
    enemyAnimations.emplace_back("zombie/skeleton-attack_0.png", 0, 8, 5);
    pair<GLint, glm::uvec2> splatterTexture = loadTexture("zombie/splatter.png");
    enemyAnimations.emplace_back(vector<GLint>(1, splatterTexture.first), splatterTexture.second, 1);
    
    player.bodyAnimationsPtr = &playerBodyAnimations;
    player.feetAnimationsPtr = &playerFeetAnimations;
    
    cout << "\n    Building level...";
    levelPositionsData = levelMap.loadMap("levels/level0.csv", loadTexture("desert_tileset.png"), glm::uvec2(32, 32));
    
    pistolItem = Item(spriteTextures, levelPositionsData[2][0], glm::vec2(160.0f, 80.0f), Item::PISTOL, 0);
    pistolItem.labelTextMap.loadFont(fontTexture, glm::uvec2(28, 36));
    pistolItem.labelTextMap.loadText("Pistol: $10");
    pistolItem.labelTextMap.color = glm::uvec4(127, 127, 127, 255);
    pistolItem.drawLabel = true;
    
    rifleItem = Item(spriteTextures, levelPositionsData[2][1], glm::vec2(160.0f, 80.0f), Item::RIFLE, 0);
    rifleItem.labelTextMap.loadFont(fontTexture, glm::uvec2(28, 36));
    rifleItem.labelTextMap.loadText("Rifle: $90");
    rifleItem.labelTextMap.color = glm::uvec4(127, 127, 127, 255);
    rifleItem.drawLabel = true;
    
    shotgunItem = Item(spriteTextures, levelPositionsData[2][2], glm::vec2(160.0f, 80.0f), Item::SHOTGUN, 0);
    shotgunItem.labelTextMap.loadFont(fontTexture, glm::uvec2(28, 36));
    shotgunItem.labelTextMap.loadText("Shotgun: $80");
    shotgunItem.labelTextMap.color = glm::uvec4(127, 127, 127, 255);
    shotgunItem.drawLabel = true;
    
    textBottomMap.loadFont(fontTexture, glm::uvec2(28, 36));
    textCenterMap.loadFont(fontTexture, glm::uvec2(28, 36));
    textTopMap.loadFont(fontTexture, glm::uvec2(28, 36));
    
    cout << "\nDone." << endl << endl;
}

void setupGame() {
    player.position = levelPositionsData[0][0];
    player.setBody(4);
    player.setFeet(0);
    player.setScale(0.5f);
    player.health = 100;
    player.iFrames = 0;
    player.iFramesMax = 20;
    
    enemies.clear();
    projectiles.clear();
    for (Item* itemPtr : itemDrops) {
        delete itemPtr;
    }
    itemDrops.clear();
    
    maximumAmmo.clear();
    maximumAmmo.push_back(pair<int, int>(0, 0));
    maximumAmmo.push_back(pair<int, int>(10, 30));
    maximumAmmo.push_back(pair<int, int>(20, 100));
    maximumAmmo.push_back(pair<int, int>(5, 60));
    weaponAmmo.clear();
    weaponAmmo.push_back(pair<int, int>(0, 0));
    weaponAmmo.push_back(maximumAmmo[1]);
    weaponAmmo.push_back(pair<int, int>(0, 0));
    weaponAmmo.push_back(pair<int, int>(0, 0));
    weaponCooldown.push_back(45);
    weaponCooldown.push_back(20);
    weaponCooldown.push_back(15);
    weaponCooldown.push_back(35);
    weaponCooldownCounter = 0;
    
    textBottomMap.clearText();
    textCenterMap.clearText();
    textCenterMap.color = glm::uvec4(255, 0, 0, 255);
    textTopMap.clearText();
    textTopMap.color = glm::uvec4(0, 0, 255, 255);
    
    playerCash = 9999;
    playerScore = 0;
    redFlashAlpha = 0;
    roundNumber = 0;
    roundCooldownCounter = 0;
    enemiesRemaining = 0;
    startOfRoundCooldown = 150;
    gameOver = false;
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
        
        pistolItem.draw();
        rifleItem.draw();
        shotgunItem.draw();
        
        for (const Item* itemPtr : itemDrops) {
            itemPtr->draw();
        }
        
        for (const Projectile& projectile : projectiles) {
            projectile.draw();
        }
        
        player.draw();
    }; glMultMatrixf(&(glm::inverse(transMtx))[0][0]);
    
    glBegin(GL_TRIANGLE_STRIP); {    // Red flash when player takes damage.
        glColor4ub(199, 0, 0, redFlashAlpha);
        glVertex2f(0.0f, static_cast<float>(WINDOW_HEIGHT));
        glVertex2f(0.0f, 0.0f);
        glVertex2f(static_cast<float>(WINDOW_WIDTH), static_cast<float>(WINDOW_HEIGHT));
        glVertex2f(static_cast<float>(WINDOW_WIDTH), 0.0f);
    }; glEnd();
    
    glBegin(GL_TRIANGLE_STRIP); {    // Draw HUD.
        glColor3ub(0, 0, 0);
        glVertex2f(0.0f, 25.0f);
        glVertex2f(0.0f, 0.0f);
        glVertex2f(static_cast<float>(WINDOW_WIDTH), 25.0f);
        glVertex2f(static_cast<float>(WINDOW_WIDTH), 0.0f);
    }; glEnd();
    
    glm::mat4 transMtx2 = glm::translate(glm::mat4(1.0f), glm::vec3(2.0f, 2.0f, 0.0f));
    glMultMatrixf(&transMtx2[0][0]); {
        glm::mat4 scaleMtx2 = glm::scale(glm::mat4(1.0f), glm::vec3(0.6f, 0.6f, 1.0f));
        glMultMatrixf(&scaleMtx2[0][0]); {
            textBottomMap.draw();
        }; glMultMatrixf(&(glm::inverse(scaleMtx2))[0][0]);
    }; glMultMatrixf(&(glm::inverse(transMtx2))[0][0]);
    
    glm::mat4 transMtx3 = glm::translate(glm::mat4(1.0f), glm::vec3(280.0f, 450.0f, 0.0f));
    glMultMatrixf(&transMtx3[0][0]); {
        textCenterMap.draw();
    }; glMultMatrixf(&(glm::inverse(transMtx3))[0][0]);
    
    glm::mat4 transMtx4 = glm::translate(glm::mat4(1.0f), glm::vec3(2.0f, 745.0f, 0.0f));
    glMultMatrixf(&transMtx4[0][0]); {
        glm::mat4 scaleMtx3 = glm::scale(glm::mat4(1.0f), glm::vec3(0.6f, 0.6f, 1.0f));
        glMultMatrixf(&scaleMtx3[0][0]); {
            textTopMap.draw();
        }; glMultMatrixf(&(glm::inverse(scaleMtx3))[0][0]);
    }; glMultMatrixf(&(glm::inverse(transMtx4))[0][0]);
}

string intToString(int x, unsigned int width) {
    string s = to_string(x);
    if (width > s.length()) {
        return string(width - s.length(), ' ') + s;
    } else {
        return s;
    }
}

void nextRound() {
    ++roundNumber;
    enemiesRemaining = roundNumber * 5 + 10;
    startOfRoundCooldown = 200;
    textCenterMap.loadText("Round " + to_string(roundNumber) + "      ");
    if (roundNumber != 1) {
        playerScore += roundNumber * 10;
    }
}

void nextTick(GLFWwindow* window) {    // Update simulation objects.
    if (gameOver) {
        return;
    }
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
    
    float maxVelocity = (player.getBody() % 4 == 2 ? 2.0f : 3.0f);    // Move player.
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
    if (player.position.x < 910.0f) {
        player.position.x = 910.0f;
    } else if (player.position.x > 3700.0f) {
        player.position.x = 3700.0f;
    }
    player.position.y += player.velocity.y;
    if (player.position.y < 910.0f) {
        player.position.y = 910.0f;
    } else if (player.position.y > 3700.0f) {
        player.position.y = 3700.0f;
    }
    
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
    
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS && weaponCooldownCounter == 0 && (weaponAmmo[player.getBody() / 4].first > 0 || player.getBody() / 4 == 0)) {    // Check for weapon fire.
        if (player.getBody() / 4 == 0) {    // Attack with knife.
            for (Enemy& enemy : enemies) {
                if (enemy.health > 0 && enemy.iFrames <= 0 && checkCollisionAABBDistance(player.position, player.getHitbox(), enemy.position, enemy.getHitbox()) < 80.0f) {
                    int numPoints = enemy.applyDamage(25);
                    playerCash += numPoints;
                    playerScore += numPoints * 3;
                    break;
                }
            }
        } else {    // Attack with gun.
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
            if (player.getBody() / 4 == 1) {    // Pistol.
                bullet.health = 5;
                bullet.damage = 20;
                projectiles.push_back(bullet);
            } else if (player.getBody() / 4 == 2) {    // Rifle.
                bullet.health = 30;
                bullet.damage = 25;
                projectiles.push_back(bullet);
            } else {    // Shotgun.
                bullet.health = 10;
                bullet.damage = 40;
                projectiles.push_back(bullet);
                bullet.rotation += 0.2f;
                projectiles.push_back(bullet);
                bullet.rotation -= 0.4f;
                projectiles.push_back(bullet);
            }
            --weaponAmmo[player.getBody() / 4].first;
        }
        
        weaponCooldownCounter = weaponCooldown[player.getBody() / 4];
        player.setBody((player.getBody() / 4) * 4 + 3);
    } else if (weaponCooldownCounter > 0) {
        --weaponCooldownCounter;
        if (player.getBody() % 4 == 3 && player.getBodyFrameNumber() == 0) {
            player.setBody(player.getBody() - 3);
        }
    }
    
    if (player.getBody() % 4 == 2 && player.getBodyFrameNumber() == 0) {    // Check if reload finished.
        int numBulletsReloaded = min(maximumAmmo[player.getBody() / 4].first - weaponAmmo[player.getBody() / 4].first, weaponAmmo[player.getBody() / 4].second);
        weaponAmmo[player.getBody() / 4].first += numBulletsReloaded;
        weaponAmmo[player.getBody() / 4].second -= numBulletsReloaded;
        player.setBody(player.getBody() - 2);
    }
    
    for (auto projectileIter = projectiles.begin(); projectileIter != projectiles.end();) {
        if (projectileIter->lifespan <= 0) {
            projectileIter = projectiles.erase(projectileIter);
        } else {
            int numPoints = projectileIter->update(enemies);
            playerCash += numPoints;
            playerScore += numPoints * 3;
            ++projectileIter;
        }
    }
    
    bool enemiesAlive = false;
    for (auto enemyIter = enemies.begin(); enemyIter != enemies.end();) {
        if (enemyIter->lifespan <= 0) {
            enemyIter = enemies.erase(enemyIter);
        } else {
            if (enemyIter->update() > 0) {
                redFlashAlpha = 127;
            }
            if (enemyIter->health > 0) {
                enemiesAlive = true;
            } else if (enemyIter->itemDropPtr != nullptr) {
                itemDrops.push_back(enemyIter->itemDropPtr);
                itemDrops.back()->position = enemyIter->position;
                enemyIter->itemDropPtr = nullptr;
            }
            ++enemyIter;
        }
    }
    
    for (auto itemPtrIter = itemDrops.begin(); itemPtrIter != itemDrops.end();) {
        if ((*itemPtrIter)->lifespan <= 0) {
            delete (*itemPtrIter);
            itemPtrIter = itemDrops.erase(itemPtrIter);
        } else if (checkCollisionAABBDistance((*itemPtrIter)->position, (*itemPtrIter)->size, player.position, player.getHitbox()) < 50.0f) {
            if ((*itemPtrIter)->type == Item::HEALTH && player.health < 100) {
                player.health = min(player.health + 50, 100);
                delete (*itemPtrIter);
                itemPtrIter = itemDrops.erase(itemPtrIter);
            } else if ((*itemPtrIter)->type == Item::AMMO && (weaponAmmo[player.getBody() / 4].first < maximumAmmo[player.getBody() / 4].first || weaponAmmo[player.getBody() / 4].second < maximumAmmo[player.getBody() / 4].second)) {
                weaponAmmo[player.getBody() / 4].first = maximumAmmo[player.getBody() / 4].first;
                weaponAmmo[player.getBody() / 4].second = maximumAmmo[player.getBody() / 4].second;
                delete (*itemPtrIter);
                itemPtrIter = itemDrops.erase(itemPtrIter);
            } else {
                ++itemPtrIter;
            }
        } else {
            (*itemPtrIter)->update();
            ++itemPtrIter;
        }
    }
    
    textBottomMap.loadText("Health: " + intToString(player.health, 3) + "     Cash: $" + intToString(playerCash, 5) + "     Ammo: " + intToString(weaponAmmo[player.getBody() / 4].first, 2) + "/" + intToString(weaponAmmo[player.getBody() / 4].second, 3) + "    ");
    textTopMap.loadText("Score: " + to_string(playerScore) + "        ");
    
    if (redFlashAlpha > 0) {
        redFlashAlpha = max(redFlashAlpha - 10, 0);
    }
    if (player.health <= 0) {    // Check game over (should occur after text updates for score/health).
        gameOver = true;
        textCenterMap.clearText();
        textCenterMap.color = glm::uvec4(0, 0, 0, 255);
        textCenterMap.loadText("Game Over\n(press enter)");
        return;
    }
    
    if (enemiesRemaining <= 0 && !enemiesAlive) {    // Check for round updates.
        if (startOfRoundCooldown <= 0) {
            nextRound();
        } else {
            --startOfRoundCooldown;
        }
    } else if (startOfRoundCooldown == 1) {
        textCenterMap.clearText();
        startOfRoundCooldown = 0;
    } else if (startOfRoundCooldown > 0) {
        --startOfRoundCooldown;
    } else {
        if (roundCooldownCounter <= 0 && enemiesRemaining > 0 && enemies.size() < ENEMY_CAP) {
            enemies.push_back(Enemy());
            enemies.back().bodyAnimationsPtr = &enemyAnimations;
            enemies.back().setBody(0);
            enemies.back().position = levelPositionsData[1][rand() % levelPositionsData[1].size()];
            enemies.back().setScale(0.5f);
            enemies.back().iFramesMax = 5;
            enemies.back().targetPtr = &player;
            
            enemies.back().health = 20 + static_cast<int>(2.0f * roundNumber);
            enemies.back().speed = 2.0f + 2.0f * static_cast<float>(rand()) / RAND_MAX + 0.2f * sqrt(static_cast<float>(roundNumber));
            enemies.back().damage = 20 + static_cast<int>(2.0f * roundNumber);
            if (static_cast<float>(rand()) / RAND_MAX < (roundNumber % 10 == 0 ? 0.5f : 0.02f)) {    // Rare zombie varient.
                enemies.back().color = glm::uvec4(98, 142, 209, 255);
                enemies.back().setScale(0.7f);
                enemies.back().health *= 4;
                enemies.back().speed += 1.0f;
            }
            if (static_cast<float>(rand()) / RAND_MAX < 0.2f / sqrt(static_cast<float>(roundNumber))) {    // Item drops.
                enemies.back().itemDropPtr = new Item(spriteTextures, glm::vec2(0.0f, 0.0f), glm::vec2(50.0f, 50.0f), Item::HEALTH, 600);
            } else if (static_cast<float>(rand()) / RAND_MAX < 0.05f / sqrt(static_cast<float>(roundNumber))) {
                enemies.back().itemDropPtr = new Item(spriteTextures, glm::vec2(0.0f, 0.0f), glm::vec2(50.0f, 50.0f), Item::AMMO, 600);
            }
            --enemiesRemaining;
            roundCooldownCounter = static_cast<int>((10.0f + 5.0f * static_cast<float>(rand()) / RAND_MAX) / (0.1f * roundNumber));
        }
        --roundCooldownCounter;
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
        loadResources();
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
