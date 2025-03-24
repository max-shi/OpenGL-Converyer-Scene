//  ========================================================================
//  Max Shi (msh254) assignment for COSC363
//  ========================================================================

#include <cmath>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <GL/freeglut.h>
#include "loadTGA.h"
#include "keyboardUtilities.h"
using namespace std;

//-- Globals ---------------------------------------------------------------

// Texture for floor.
GLuint floorTex;

// Conveyor belt globals (oriented along x-axis).
float beltOffset = 0.0f;         // Current offset along the belt (x-axis)
float beltSpeed  = 0.025f;         // Speed of belt movement (units per update)
float beltXLength = 40.0f;        // Belt spans from x = -20 to x = +20.
const float beltZMin = -5.0f;     // Belt z-start.
const float beltZMax = -3.0f;      // Belt z-end.

// Global instance of KeyboardUtilities.
KeyboardUtilities keyboardUtil;

//------------------- Update Scene (Belt & Camera) -------------------------
void updateScene(int value) {
    // Update conveyor belt offset along the x-axis.
    beltOffset += beltSpeed;
    if (beltOffset > beltXLength)
        beltOffset = fmod(beltOffset, beltXLength); // Loop back.

    // Update camera movement.
    keyboardUtil.update();

    glutPostRedisplay();
    glutTimerFunc(16, updateScene, 0); // Roughly 60 FPS.
}

//------------------- Draw Conveyor Belt ---------------------------
void drawConveyorBelt() {
    glColor3f(0.3f, 0.3f, 0.3f); // Gray color.
    glBegin(GL_QUADS);
        glVertex3f(-20.0f, 2.0f, beltZMin);
        glVertex3f(-20.0f, 2.0f, beltZMax);
        glVertex3f( 20.0f, 2.0f, beltZMax);
        glVertex3f( 20.0f, 2.0f, beltZMin);
    glEnd();
}

//------------------- Draw Processed Item ---------------------------
// Items are placed along the belt.
// Compute worldX = -20 + offset, where offset ranges over [0, beltXLength].
// If worldX < 0, the item is drawn as a cube whose scale interpolates
// from 1.0 at x=-20 to 0.75 at x=-3 (and remains 0.75 for -3 ≤ worldX < 0).
// If worldX >= 0, the item is drawn as a teapot scaled to 0.75.
void drawProcessedItem(float offset) {
    glPushMatrix();
        // Compute world x coordinate.
        float worldX = -20.0f + offset;
        // Position item: at computed x, fixed y, and centered in z.
        glTranslatef(worldX, 2.5f, (beltZMin + beltZMax) / 2.0f);

        if (worldX < 0.0f) {
            float scaleFactor;
            if (worldX <= -3.0f) {
                // Linear interpolation from 1.0 at x=-20 to 0.75 at x=-3.
                // Total range is 17 units.
                scaleFactor = 1.0f - 0.25f * ((worldX + 20.0f) / 17.0f);
            } else {
                scaleFactor = 0.75f;
            }
            glScalef(scaleFactor, scaleFactor, scaleFactor);
            glColor3f(0.0f, 0.0f, 1.0f); // Render cube in blue.
            glutSolidCube(1.0);
        } else {
            // For worldX >= 0, instantly switch to teapot.
            glScalef(0.75f, 0.75f, 0.75f);
            glColor3f(1.0f, 1.0f, 0.0f);
            glutSolidTeapot(1.0);
        }
    glPopMatrix();
}

//------------------- Draw Support Box Underneath Conveyor ---------------------------
void drawSupportBox() {
    // Support box spans x = -20 to 20, z from beltZMin to beltZMax, and y from 0 to 2.
    float xLeft = -20.0f, xRight = 20.0f;
    float zBack = beltZMin, zFront = beltZMax;
    float yBottom = 0.0f, yTop = 2.0f;

    glColor3f(0.6f, 0.4f, 0.2f); // Brownish color.
    glBegin(GL_QUADS);
        // Front face.
        glVertex3f(xLeft, yBottom, zFront);
        glVertex3f(xRight, yBottom, zFront);
        glVertex3f(xRight, yTop, zFront);
        glVertex3f(xLeft, yTop, zFront);
        // Back face.
        glVertex3f(xRight, yBottom, zBack);
        glVertex3f(xLeft, yBottom, zBack);
        glVertex3f(xLeft, yTop, zBack);
        glVertex3f(xRight, yTop, zBack);
        // Left face.
        glVertex3f(xLeft, yBottom, zBack);
        glVertex3f(xLeft, yBottom, zFront);
        glVertex3f(xLeft, yTop, zFront);
        glVertex3f(xLeft, yTop, zBack);
        // Right face.
        glVertex3f(xRight, yBottom, zFront);
        glVertex3f(xRight, yBottom, zBack);
        glVertex3f(xRight, yTop, zBack);
        glVertex3f(xRight, yTop, zFront);
        // Top face.
        glVertex3f(xLeft, yTop, zBack);
        glVertex3f(xLeft, yTop, zFront);
        glVertex3f(xRight, yTop, zFront);
        glVertex3f(xRight, yTop, zBack);
        // Bottom face.
        glVertex3f(xLeft, yBottom, zFront);
        glVertex3f(xLeft, yBottom, zBack);
        glVertex3f(xRight, yBottom, zBack);
        glVertex3f(xRight, yBottom, zFront);
    glEnd();
}

//------------------- Loads Textures ---------------------------
void loadTextures() {
    glGenTextures(1, &floorTex);
    glBindTexture(GL_TEXTURE_2D, floorTex);
    loadTGA("concrete.tga");
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
}

//------------------- Draw Textured Floor ---------------------------
void drawTexturedFloor() {
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, floorTex);
    glColor3f(1.0f, 1.0f, 1.0f);
    // Floor now extends from x = -30 to 30 and z = -30 to 30.
    glBegin(GL_QUADS);
        glTexCoord2f(0.0f, 0.0f); glVertex3f(-30.0f, 0.0f, -30.0f);
        glTexCoord2f(0.0f, 1.0f); glVertex3f(-30.0f, 0.0f,  30.0f);
        glTexCoord2f(1.0f, 1.0f); glVertex3f( 30.0f, 0.0f,  30.0f);
        glTexCoord2f(1.0f, 0.0f); glVertex3f( 30.0f, 0.0f, -30.0f);
    glEnd();
    glDisable(GL_TEXTURE_2D);
}

//------------------- Display Callback ---------------------------
void display() {
    float lpos[4] = {10., 10., 10., 1.};
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // Compute the look direction using the camera's yaw and pitch.
    float radYaw = keyboardUtil.getYaw() * M_PI / 180.0f;
    float radPitch = keyboardUtil.getPitch() * M_PI / 180.0f;
    float lookDirX = cos(radPitch) * sin(radYaw);
    float lookDirY = sin(radPitch);
    float lookDirZ = -cos(radPitch) * cos(radYaw);

    // Set the camera using gluLookAt.
    gluLookAt(keyboardUtil.getPosX(), keyboardUtil.getPosY(), keyboardUtil.getPosZ(),
              keyboardUtil.getPosX() + lookDirX,
              keyboardUtil.getPosY() + lookDirY,
              keyboardUtil.getPosZ() + lookDirZ,
              0.0, 1.0, 0.0);

    glLightfv(GL_LIGHT0, GL_POSITION, lpos);

    // Draw the scene.
    glDisable(GL_LIGHTING);
    drawTexturedFloor();
    glEnable(GL_LIGHTING);
    drawSupportBox();
    drawConveyorBelt();

    // Draw 8 items on the conveyor, spaced evenly.
    int numItems = 9;
    float spacing = beltXLength / numItems; // spacing along the belt
    for (int i = 0; i < numItems; i++) {
        float itemOffset = fmod(beltOffset + i * spacing, beltXLength);
        drawProcessedItem(itemOffset);
    }

    glutSwapBuffers();
}

//------------------- Keyboard and Special Key Callbacks ---------------------------
void keyboardDownCallback(unsigned char key, int x, int y) {
    keyboardUtil.keyboardDown(key);
    if (key == 27) exit(0);
}

void keyboardUpCallback(unsigned char key, int x, int y) {
    keyboardUtil.keyboardUp(key);
}

void specialKeyDownCallback(int key, int x, int y) {
    keyboardUtil.specialKeyDown(key);
}

void specialKeyUpCallback(int key, int x, int y) {
    keyboardUtil.specialKeyUp(key);
}

//------------------- OpenGL Initialization ---------------------------
void initialize() {
    glClearColor(1., 1., 1., 1.);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);
    loadTextures();
    glEnable(GL_NORMALIZE);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glFrustum(-5., 5., -5., 5., 5., 1000.);
}

//------------------- Main Function ---------------------------
int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(1000, 1000);
    glutInitWindowPosition(10, 10);
    glutCreateWindow("Conveyor Belt");

    initialize();

    glutDisplayFunc(display);
    glutKeyboardFunc(keyboardDownCallback);
    glutKeyboardUpFunc(keyboardUpCallback);
    glutSpecialFunc(specialKeyDownCallback);
    glutSpecialUpFunc(specialKeyUpCallback);
    glutTimerFunc(16, updateScene, 0);

    glutMainLoop();
    return 0;
}
