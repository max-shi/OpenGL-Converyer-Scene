//  ========================================================================
//  Max Shi (msh254), code for assignment 1
//  ========================================================================

#include <cmath>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <GL/freeglut.h>
#include "loadTGA.h"
using namespace std;

//-- Globals ---------------------------------------------------------------
// Texture for floor
GLuint floorTex;

// Conveyor belt globals
float beltOffset = 0.0f;     // Current position along the belt
float beltSpeed  = 0.1f;      // Speed of belt movement (units per update)
float beltLength = 30.0f;     // Total length of the conveyor belt
float beltWidth  = 5.0f;

// Camera globals for smooth first-person movement
float camPosX = 0.0f, camPosY = 4.0f, camPosZ = 10.0f;
float camYaw   = 0.0f;        // Yaw in degrees; 0 means looking along -Z
float camPitch = 0.0f;        // Pitch in degrees; 0 means horizontal view
const float moveSpeed = 0.2f; // Movement speed per update
const float rotSpeed  = 1.6f;  // Rotation speed (degrees per update)

// Key state arrays for smooth continuous movement
bool keyStates[256] = { false };
bool specialKeyStates[256] = { false };


//------------------- Update Scene (Belt & Camera) -------------------------
void updateScene(int value) {
    // Update conveyor belt offset
    beltOffset += beltSpeed;
    if (beltOffset > beltLength)
        beltOffset = fmod(beltOffset, beltLength); // loop back

    // Update camera movement based on currently pressed keys
    float radYaw = camYaw * M_PI / 180.0f;

    if (keyStates['w'] || keyStates['W']) {
       camPosX += moveSpeed * sin(radYaw);
       camPosZ += -moveSpeed * cos(radYaw);
    }
    if (keyStates['s'] || keyStates['S']) {
       camPosX -= moveSpeed * sin(radYaw);
       camPosZ -= -moveSpeed * cos(radYaw);
    }
    if (keyStates['a'] || keyStates['A']) {
       // Strafe left (perpendicular to the look direction)
       camPosX -= moveSpeed * cos(radYaw);
       camPosZ -= moveSpeed * sin(radYaw);
    }
    if (keyStates['d'] || keyStates['D']) {
       // Strafe right
       camPosX += moveSpeed * cos(radYaw);
       camPosZ += moveSpeed * sin(radYaw);
    }

    // Update camera rotation using arrow keys
    if (specialKeyStates[GLUT_KEY_LEFT])
       camYaw -= rotSpeed;
    if (specialKeyStates[GLUT_KEY_RIGHT])
       camYaw += rotSpeed;
    if (specialKeyStates[GLUT_KEY_UP]) {
       camPitch += rotSpeed;
       if (camPitch > 89.0f) camPitch = 89.0f;
    }
    if (specialKeyStates[GLUT_KEY_DOWN]) {
       camPitch -= rotSpeed;
       if (camPitch < -89.0f) camPitch = -89.0f;
    }

    glutPostRedisplay();
    glutTimerFunc(16, updateScene, 0); // roughly 60 FPS
}


//------------------- Draw Conveyor Belt ---------------------------
void drawConveyorBelt() {
    glColor3f(0.3f, 0.3f, 0.3f); // Gray color for belt surface
    glBegin(GL_QUADS);
        glVertex3f(-beltWidth/2, 2.0f, 0.0f);
        glVertex3f(-beltWidth/2, 2.0f, beltLength);
        glVertex3f( beltWidth/2, 2.0f, beltLength);
        glVertex3f( beltWidth/2, 2.0f, 0.0f);
    glEnd();
}

//------------------- Draw an Item ---------------------------
void drawItem(float zPos) {
    glPushMatrix();
        // Position the item: centered on the belt and raised above it
        glTranslatef(0.0f, 2.5f, zPos);
        glColor3f(1.0f, 0.0f, 0.0f);  // Red color for the item
        glutSolidCube(1.0);
    glPopMatrix();
}

//------------------- Loads Textures ---------------------------
void loadTextures() {
    glGenTextures(1, &floorTex);              // Generate a texture ID
    glBindTexture(GL_TEXTURE_2D, floorTex);    // Bind the texture for use
    loadTGA("concrete.tga");                  // Load the TGA file (ensure it's in the proper path)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
}

//------------------- Draw Textured Floor ---------------------------
void drawTexturedFloor() {
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, floorTex);

    // Set color to white so the texture appears correctly
    glColor3f(1.0f, 1.0f, 1.0f);

    // Draw a quad for the floor with texture coordinates
    glBegin(GL_QUADS);
        glTexCoord2f(0.0f, 0.0f); glVertex3f(-15.0f, 0.0f, -15.0f);
        glTexCoord2f(0.0f, 1.0f); glVertex3f(-15.0f, 0.0f,  15.0f);
        glTexCoord2f(1.0f, 1.0f); glVertex3f( 15.0f, 0.0f,  15.0f);
        glTexCoord2f(1.0f, 0.0f); glVertex3f( 15.0f, 0.0f, -15.0f);
    glEnd();

    glDisable(GL_TEXTURE_2D);
}

//------------------- Display Callback ---------------------------
void display() {
    float lpos[4] = {10., 10., 10., 1.}; // Light's position

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // Compute the look direction from yaw and pitch (convert degrees to radians)
    float radYaw   = camYaw * M_PI / 180.0f;
    float radPitch = camPitch * M_PI / 180.0f;
    float lookDirX = cos(radPitch) * sin(radYaw);
    float lookDirY = sin(radPitch);
    float lookDirZ = -cos(radPitch) * cos(radYaw);

    // Set the camera using gluLookAt:
    gluLookAt(camPosX, camPosY, camPosZ,
              camPosX + lookDirX, camPosY + lookDirY, camPosZ + lookDirZ,
              0.0, 1.0, 0.0);

    glLightfv(GL_LIGHT0, GL_POSITION, lpos);

    // Draw scene objects
    glDisable(GL_LIGHTING);
    drawTexturedFloor();
    glEnable(GL_LIGHTING);

    drawConveyorBelt();
    drawItem(beltOffset);
    float secondItemPos = beltOffset - 10.0f;
    if (secondItemPos < 0)
        secondItemPos += beltLength;
    drawItem(secondItemPos);

    glutSwapBuffers();
}


//------------------- Keyboard Callbacks ---------------------------
// When a key is pressed, record its state as true.
void keyboardDown(unsigned char key, int x, int y) {
    keyStates[key] = true;
    if (key == 27) // Escape key
        exit(0);
}

// When a key is released, record its state as false.
void keyboardUp(unsigned char key, int x, int y) {
    keyStates[key] = false;
}

// For special keys (arrow keys), record state on press.
void specialKeyDown(int key, int x, int y) {
    specialKeyStates[key] = true;
}

// And record when special keys are released.
void specialKeyUp(int key, int x, int y) {
    specialKeyStates[key] = false;
}

//------------------- OpenGL Initialization ---------------------------
void initialize() {
    glClearColor(1., 1., 1., 1.);         // Background color
    glEnable(GL_LIGHTING);                // Enable lighting
    glEnable(GL_LIGHT0);                  // Enable light source 0
    glEnable(GL_COLOR_MATERIAL);          // Enable color tracking
    glEnable(GL_DEPTH_TEST);              // Enable depth testing
    glEnable(GL_TEXTURE_2D);              // Enable texturing
    loadTextures();                       // Load our concrete texture
    glEnable(GL_NORMALIZE);               // Automatically normalize normals

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glFrustum(-5., 5., -5., 5., 5., 1000.); // Set up the camera frustum
}

//------------------- Main Function ---------------------------
int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(1000, 1000);
    glutInitWindowPosition(10, 10);
    glutCreateWindow("Conveyor Belt with Smooth Camera");

    initialize();

    glutDisplayFunc(display);
    glutKeyboardFunc(keyboardDown);
    glutKeyboardUpFunc(keyboardUp);
    glutSpecialFunc(specialKeyDown);
    glutSpecialUpFunc(specialKeyUp);
    glutTimerFunc(16, updateScene, 0);

    glutMainLoop();
    return 0;
}
