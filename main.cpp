//  ========================================================================
//  COSC363: Computer Graphics (2025);  University of Canterbury.
//
//  FILE NAME: Humanoid.cpp
//  See Lab02.pdf for details
//  ========================================================================

#include <cmath>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <GL/freeglut.h>
#include "loadTGA.h"
using namespace std;

//-- Globals ---------------------------------------------------------------
GLuint floorTex;    // Texture ID for the floor

// Conveyor belt globals
float beltOffset = 0.0f;     // Current position along the belt
float beltSpeed  = 0.1f;      // Speed of belt movement (units per update)
float beltLength = 30.0f;     // Total length of the conveyor belt
float beltWidth  = 5.0f;

// Camera globals for first-person movement
float camPosX = 0.0f, camPosY = 4.0f, camPosZ = 10.0f;
float camYaw   = 0.0f;        // Yaw in degrees; 0 means looking along -Z
float camPitch = 0.0f;        // Pitch in degrees; 0 means horizontal view
const float moveSpeed = 0.5f; // Movement speed

//------------------- Conveyor Belt Update -------------------------
void updateBelt(int value) {
    beltOffset += beltSpeed;
    if (beltOffset > beltLength)
        beltOffset = fmod(beltOffset, beltLength); // loop back

    glutPostRedisplay();
    glutTimerFunc(16, updateBelt, 0);
}

//------------------- Draw Conveyor Belt ---------------------------
void drawConveyorBelt() {
    // Gray color for belt surface
    glColor3f(0.3f, 0.3f, 0.3f);
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
    loadTGA("concrete.tga");                  // Load the TGA file (ensure it’s in the proper path)
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

    // Draw the scene: floor, conveyor belt, and moving items
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

//------------------- Normal Keyboard Callback ---------------------------
void keyboard(unsigned char key, int x, int y) {
    float radYaw = camYaw * M_PI / 180.0f;
    switch(key) {
        case 'w': case 'W':
            // Move forward along the horizontal projection of the look direction
            camPosX += moveSpeed * sin(radYaw);
            camPosZ += -moveSpeed * cos(radYaw);
            break;
        case 's': case 'S':
            // Move backward
            camPosX -= moveSpeed * sin(radYaw);
            camPosZ -= -moveSpeed * cos(radYaw);
            break;
        case 'a': case 'A':
            // Strafe left (perpendicular to the look direction)
            camPosX -= moveSpeed * cos(radYaw);
            camPosZ -= moveSpeed * sin(radYaw);
            break;
        case 'd': case 'D':
            // Strafe right
            camPosX += moveSpeed * cos(radYaw);
            camPosZ += moveSpeed * sin(radYaw);
            break;
        case 27: // Escape key
            exit(0);
            break;
    }
    glutPostRedisplay();
}

//------------------- Special Key Callback (Arrow Keys) ---------------------------
void specialKeys(int key, int x, int y) {
    switch(key) {
        case GLUT_KEY_LEFT:
            camYaw -= 2.0f;  // Rotate view left
            break;
        case GLUT_KEY_RIGHT:
            camYaw += 2.0f;  // Rotate view right
            break;
        case GLUT_KEY_UP:
            camPitch += 2.0f;  // Tilt view upward
            if (camPitch > 89.0f) camPitch = 89.0f; // Limit pitch
            break;
        case GLUT_KEY_DOWN:
            camPitch -= 2.0f;  // Tilt view downward
            if (camPitch < -89.0f) camPitch = -89.0f;
            break;
    }
    glutPostRedisplay();
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
    glutInitWindowSize(600, 600);
    glutInitWindowPosition(10, 10);
    glutCreateWindow("Conveyor Belt with Camera");

    initialize();

    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(specialKeys);
    glutTimerFunc(16, updateBelt, 0);

    glutMainLoop();
    return 0;
}
