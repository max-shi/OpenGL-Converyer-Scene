//  ========================================================================
//  Max Shi - Cosc363
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

// Textures
GLuint floorTex;
GLuint beltTex;
GLuint metalTex;

// Conveyor belt globals (oriented along x-axis).
float beltOffset = 0.0f;          // Current offset along the belt (x-axis)
float beltSpeed = 0.025f;         // Speed of belt movement (units per update)
float beltXLength = 40.0f;        // Belt spans from x = -20 to x = +20
const float beltZMin = -5.0f;     // Belt z-start
const float beltZMax = -3.0f;     // Belt z-end
const float beltWidth = beltZMax - beltZMin;  // Width of the belt (z-axis)
const float beltThickness = 0.1f; // Thickness of the belt material
const float rollerRadius = 0.3f;  // Radius of the end rollers
float rollerRotation = 0.0f;      // Current rotation angle of rollers

// Global instance of KeyboardUtilities.
KeyboardUtilities keyboardUtil;

//------------------- Update Scene (Belt & Camera) -------------------------
void updateScene(int value) {
    // Update conveyor belt offset along the x-axis.
    beltOffset += beltSpeed;
    if (beltOffset > beltXLength)
        beltOffset = fmod(beltOffset, beltXLength); // Loop back.

    // Update camera movement
    keyboardUtil.update();

    glutPostRedisplay();
    glutTimerFunc(16, updateScene, 0); // Roughly 60 FPS.
}

//------------------- Draw Conveyor Belt ---------------------------
void drawConveyorBelt() {
    float xLeft = -20.0f, xRight = 20.0f;
    float yBase = 2.1f;
    float normalizedOffset = beltOffset/7.5;
    // Enable texture for belt
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, beltTex);

    // Draw the top moving surface of belt (with texture animation)
    glColor3f(0.7f, 0.7f, 0.7f); // Almost black for rubber belt
    glBegin(GL_QUADS);
        glTexCoord2f(0.0f - normalizedOffset, 0.0f); glVertex3f(xLeft, yBase + rollerRadius, beltZMin);
        glTexCoord2f(0.0f - normalizedOffset, 1.0f); glVertex3f(xLeft, yBase + rollerRadius, beltZMax);
        glTexCoord2f(8.0f - normalizedOffset, 1.0f); glVertex3f(xRight, yBase + rollerRadius, beltZMax);
        glTexCoord2f(8.0f - normalizedOffset, 0.0f); glVertex3f(xRight, yBase + rollerRadius, beltZMin);
    glEnd();

    // Draw the bottom return section of belt
    glBegin(GL_QUADS);
        glTexCoord2f(0.0f + normalizedOffset, 0.0f); glVertex3f(xLeft, yBase - rollerRadius, beltZMin);
        glTexCoord2f(8.0f + normalizedOffset, 0.0f); glVertex3f(xRight, yBase - rollerRadius, beltZMin);
        glTexCoord2f(8.0f + normalizedOffset, 1.0f); glVertex3f(xRight, yBase - rollerRadius, beltZMax);
        glTexCoord2f(0.0f + normalizedOffset, 1.0f); glVertex3f(xLeft, yBase - rollerRadius, beltZMax);
    glEnd();

    // Draw the side sections connecting top and bottom (left end)
    glBegin(GL_QUADS);
        glTexCoord2f(0.0f, 0.0f); glVertex3f(xLeft, yBase - rollerRadius, beltZMin);
        glTexCoord2f(0.0f, 0.1f); glVertex3f(xLeft, yBase - rollerRadius, beltZMax);
        glTexCoord2f(0.2f, 0.1f); glVertex3f(xLeft, yBase + rollerRadius, beltZMax);
        glTexCoord2f(0.2f, 0.0f); glVertex3f(xLeft, yBase + rollerRadius, beltZMin);
    glEnd();

    // Draw the side sections connecting top and bottom (right end)
    glBegin(GL_QUADS);
        glTexCoord2f(0.0f, 0.0f); glVertex3f(xRight, yBase - rollerRadius, beltZMin);
        glTexCoord2f(0.2f, 0.0f); glVertex3f(xRight, yBase + rollerRadius, beltZMin);
        glTexCoord2f(0.2f, 0.1f); glVertex3f(xRight, yBase + rollerRadius, beltZMax);
        glTexCoord2f(0.0f, 0.1f); glVertex3f(xRight, yBase - rollerRadius, beltZMax);
    glEnd();

    glDisable(GL_TEXTURE_2D);
    glDisable(GL_TEXTURE_2D);
}


//------------------- Draw Processed Item ---------------------------
void drawProcessedItem(float offset) {
    glPushMatrix();
        // Compute world x coordinate.
        float worldX = -20.0f + offset;
        // Position item: at computed x, fixed y above the conveyor surface, and centered in z.
        glTranslatef(worldX, 2.5f + rollerRadius, (beltZMin + beltZMax) / 2.0f);

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

//------------------- Draw Support Structure ---------------------------
void drawSupportStructure() {
    float xLeft = -20.0f, xRight = 20.0f;
    float zBack = beltZMin - 0.5f, zFront = beltZMax + 0.5f;
    float yBottom = 0.0f, yTop = 2.1f - rollerRadius;
    float legWidth = 0.3f;

    // Use metal texture for support structure
    glBindTexture(GL_TEXTURE_2D, metalTex);
    glEnable(GL_TEXTURE_2D);
    glColor3f(0.5f, 0.5f, 0.5f); // Medium gray for metal structure

    // Draw legs (vertical supports)
    // Left front leg
    glPushMatrix();
        glTranslatef(xLeft + legWidth/2, yBottom + (yTop-yBottom)/2, zFront - legWidth/2);
        glScalef(legWidth, yTop-yBottom, legWidth);
        glutSolidCube(1.0f);
    glPopMatrix();

    // Left back leg
    glPushMatrix();
        glTranslatef(xLeft + legWidth/2, yBottom + (yTop-yBottom)/2, zBack + legWidth/2);
        glScalef(legWidth, yTop-yBottom, legWidth);
        glutSolidCube(1.0f);
    glPopMatrix();

    // Right front leg
    glPushMatrix();
        glTranslatef(xRight - legWidth/2, yBottom + (yTop-yBottom)/2, zFront - legWidth/2);
        glScalef(legWidth, yTop-yBottom, legWidth);
        glutSolidCube(1.0f);
    glPopMatrix();

    // Right back leg
    glPushMatrix();
        glTranslatef(xRight - legWidth/2, yBottom + (yTop-yBottom)/2, zBack + legWidth/2);
        glScalef(legWidth, yTop-yBottom, legWidth);
        glutSolidCube(1.0f);
    glPopMatrix();

    // Add intermediate supports
    int numSupports = 3;
    float supportSpacing = (xRight - xLeft) / (numSupports + 1);

    for (int i = 1; i <= numSupports; i++) {
        float xPos = xLeft + i * supportSpacing;

        // Front support
        glPushMatrix();
            glTranslatef(xPos, yBottom + (yTop-yBottom)/2, zFront - legWidth/2);
            glScalef(legWidth, yTop-yBottom, legWidth);
            glutSolidCube(1.0f);
        glPopMatrix();

        // Back support
        glPushMatrix();
            glTranslatef(xPos, yBottom + (yTop-yBottom)/2, zBack + legWidth/2);
            glScalef(legWidth, yTop-yBottom, legWidth);
            glutSolidCube(1.0f);
        glPopMatrix();
    }

    // Horizontal crossbeams connecting legs (along z-axis, front and back)
    // Top front beam
    glPushMatrix();
        glTranslatef((xLeft + xRight)/2, yTop, zFront - legWidth/2);
        glScalef(xRight - xLeft, legWidth/2, legWidth);
        glutSolidCube(1.0f);
    glPopMatrix();

    // Top back beam
    glPushMatrix();
        glTranslatef((xLeft + xRight)/2, yTop, zBack + legWidth/2);
        glScalef(xRight - xLeft, legWidth/2, legWidth);
        glutSolidCube(1.0f);
    glPopMatrix();

    // Bottom front beam
    glPushMatrix();
        glTranslatef((xLeft + xRight)/2, yBottom + legWidth/2, zFront - legWidth/2);
        glScalef(xRight - xLeft, legWidth, legWidth);
        glutSolidCube(1.0f);
    glPopMatrix();

    // Bottom back beam
    glPushMatrix();
        glTranslatef((xLeft + xRight)/2, yBottom + legWidth/2, zBack + legWidth/2);
        glScalef(xRight - xLeft, legWidth, legWidth);
        glutSolidCube(1.0f);
    glPopMatrix();

    // Cross beams along x-axis (connecting front and back)
    // Left side - top
    glPushMatrix();
        glTranslatef(xLeft + legWidth/2, yTop, (zFront + zBack)/2);
        glScalef(legWidth, legWidth/2, zFront - zBack);
        glutSolidCube(1.0f);
    glPopMatrix();

    // Right side - top
    glPushMatrix();
        glTranslatef(xRight - legWidth/2, yTop, (zFront + zBack)/2);
        glScalef(legWidth, legWidth/2, zFront - zBack);
        glutSolidCube(1.0f);
    glPopMatrix();

    // Add platform to support the return belt
    glColor3f(0.4f, 0.4f, 0.4f); // Darker gray for platform
    glPushMatrix();
        glTranslatef((xLeft + xRight)/2, yTop - rollerRadius * 1.5f, (zFront + zBack)/2);
        glScalef(xRight - xLeft, legWidth/2, zFront - zBack - legWidth);
        glutSolidCube(1.0f);
    glPopMatrix();

    // Add diagonal braces for stability
    glColor3f(0.45f, 0.45f, 0.45f);

    // Front left diagonal
    glPushMatrix();
        glTranslatef(xLeft + 1.0f, yBottom + (yTop-yBottom)/2, zFront - legWidth/2);
        glRotatef(30.0f, 0.0f, 0.0f, 1.0f);
        glScalef(sqrt(pow(yTop-yBottom, 2) + pow(2.0f, 2)), legWidth/3, legWidth/2);
        glutSolidCube(1.0f);
    glPopMatrix();

    // Back left diagonal
    glPushMatrix();
        glTranslatef(xLeft + 1.0f, yBottom + (yTop-yBottom)/2, zBack + legWidth/2);
        glRotatef(30.0f, 0.0f, 0.0f, 1.0f);
        glScalef(sqrt(pow(yTop-yBottom, 2) + pow(2.0f, 2)), legWidth/3, legWidth/2);
        glutSolidCube(1.0f);
    glPopMatrix();

    // Front right diagonal
    glPushMatrix();
        glTranslatef(xRight - 1.0f, yBottom + (yTop-yBottom)/2, zFront - legWidth/2);
        glRotatef(-30.0f, 0.0f, 0.0f, 1.0f);
        glScalef(sqrt(pow(yTop-yBottom, 2) + pow(2.0f, 2)), legWidth/3, legWidth/2);
        glutSolidCube(1.0f);
    glPopMatrix();

    // Back right diagonal
    glPushMatrix();
        glTranslatef(xRight - 1.0f, yBottom + (yTop-yBottom)/2, zBack + legWidth/2);
        glRotatef(-30.0f, 0.0f, 0.0f, 1.0f);
        glScalef(sqrt(pow(yTop-yBottom, 2) + pow(2.0f, 2)), legWidth/3, legWidth/2);
        glutSolidCube(1.0f);
    glPopMatrix();

    glDisable(GL_TEXTURE_2D);
}

//------------------- Loads Textures ---------------------------
void loadTextures() {
    // Floor texture
    glGenTextures(1, &floorTex);
    glBindTexture(GL_TEXTURE_2D, floorTex);
    loadTGA("concrete.tga");
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    // Belt texture (assuming you have a rubber_belt.tga file)
    glGenTextures(1, &beltTex);
    glBindTexture(GL_TEXTURE_2D, beltTex);
    loadTGA("concrete.tga"); // You'll need to create this texture file
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    // Metal texture for rollers and supports
    glGenTextures(1, &metalTex);
    glBindTexture(GL_TEXTURE_2D, metalTex);
    loadTGA("concrete.tga"); // You'll need to create this texture file
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
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

    // Draw the enhanced conveyor system
    drawSupportStructure();
    drawConveyorBelt();

    // Draw items on the conveyor, spaced evenly.
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

    // Add controls for belt speed
    if (key == '+' || key == '=') {
        beltSpeed += 0.005f;
        if (beltSpeed > 0.1f) beltSpeed = 0.1f;
    }
    else if (key == '-' || key == '_') {
        beltSpeed -= 0.005f;
        if (beltSpeed < 0.0f) beltSpeed = 0.0f;
    }
    else if (key == 'r' || key == 'R') {
        // Reverse direction
        beltSpeed = -beltSpeed;
    }
    else if (key == 27) exit(0);
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
    glutCreateWindow("Industrial Conveyor Belt Simulation");

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