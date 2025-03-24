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

// Textures
GLuint floorTex;
GLuint beltTex;
GLuint metalTex;

// Conveyor belt globals (oriented along x-axis).
float beltOffset = 0.0f;         // Current offset along the belt (x-axis)
float beltSpeed = 0.025f;        // Speed of belt movement (units per update)
float beltXLength = 40.0f;       // Belt spans from x = -20 to x = +20.
const float beltZMin = -5.0f;    // Belt z-start.
const float beltZMax = -3.0f;    // Belt z-end.
const float beltWidth = beltZMax - beltZMin;
const float beltThickness = 0.1f;
const float rollerRadius = 0.5f;
const int rollerSegments = 24;   // Number of segments for cylindrical objects

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

//------------------- Helper Functions for Cylindrical Objects -------------
// Function to draw a cylinder with end caps
void drawCylinder(float radius, float length, int segments) {
    // Draw the main cylindrical body
    GLUquadricObj *quadric = gluNewQuadric();
    gluQuadricDrawStyle(quadric, GLU_FILL);
    gluQuadricNormals(quadric, GLU_SMOOTH);
    gluQuadricTexture(quadric, GL_TRUE);

    // No rotation here - we'll handle orientation in the calling function
    // Draw cylindrical body along Z axis by default
    gluCylinder(quadric, radius, radius, length, segments, 1);

    // Draw end caps
    gluDisk(quadric, 0.0f, radius, segments, 1);
    glPushMatrix();
    glTranslatef(0.0f, 0.0f, length);
    gluDisk(quadric, 0.0f, radius, segments, 1);
    glPopMatrix();

    gluDeleteQuadric(quadric);
}

//------------------- Draw Conveyor Belt Components -----------------------
// Draw the main belt surface
void drawBeltSurface() {
    // Enable texturing for the belt
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, beltTex);

    glColor3f(0.1f, 0.1f, 0.1f); // Dark black color for the belt

    float beltLeft = -20.0f;
    float beltRight = 20.0f;
    float texOffset = beltOffset / beltXLength; // Calculate texture offset for animation

    // Top surface of belt
    glBegin(GL_QUADS);
        glNormal3f(0.0f, 1.0f, 0.0f);
        glTexCoord2f(0.0f + texOffset, 0.0f); glVertex3f(beltLeft, 2.1f + rollerRadius, beltZMin);
        glTexCoord2f(0.0f + texOffset, 1.0f); glVertex3f(beltLeft, 2.1f + rollerRadius, beltZMax);
        glTexCoord2f(5.0f + texOffset, 1.0f); glVertex3f(beltRight, 2.1f + rollerRadius, beltZMax);
        glTexCoord2f(5.0f + texOffset, 0.0f); glVertex3f(beltRight, 2.1f + rollerRadius, beltZMin);
    glEnd();

    // Bottom surface of belt (returning underneath)
    glBegin(GL_QUADS);
        glNormal3f(0.0f, -1.0f, 0.0f);
        glTexCoord2f(0.0f + texOffset, 0.0f); glVertex3f(beltLeft, 2.1f - rollerRadius, beltZMin);
        glTexCoord2f(5.0f + texOffset, 0.0f); glVertex3f(beltRight, 2.1f - rollerRadius, beltZMin);
        glTexCoord2f(5.0f + texOffset, 1.0f); glVertex3f(beltRight, 2.1f - rollerRadius, beltZMax);
        glTexCoord2f(0.0f + texOffset, 1.0f); glVertex3f(beltLeft, 2.1f - rollerRadius, beltZMax);
    glEnd();

    // Left edge of belt
    glBegin(GL_QUADS);
        glNormal3f(-1.0f, 0.0f, 0.0f);
        glTexCoord2f(0.0f, 0.0f); glVertex3f(beltLeft, 2.1f - rollerRadius, beltZMin);
        glTexCoord2f(1.0f, 0.0f); glVertex3f(beltLeft, 2.1f - rollerRadius, beltZMax);
        glTexCoord2f(1.0f, 0.1f); glVertex3f(beltLeft, 2.1f + rollerRadius, beltZMax);
        glTexCoord2f(0.0f, 0.1f); glVertex3f(beltLeft, 2.1f + rollerRadius, beltZMin);
    glEnd();

    // Right edge of belt
    glBegin(GL_QUADS);
        glNormal3f(1.0f, 0.0f, 0.0f);
        glTexCoord2f(0.0f, 0.0f); glVertex3f(beltRight, 2.1f - rollerRadius, beltZMax);
        glTexCoord2f(1.0f, 0.0f); glVertex3f(beltRight, 2.1f - rollerRadius, beltZMin);
        glTexCoord2f(1.0f, 0.1f); glVertex3f(beltRight, 2.1f + rollerRadius, beltZMin);
        glTexCoord2f(0.0f, 0.1f); glVertex3f(beltRight, 2.1f + rollerRadius, beltZMax);
    glEnd();

    // Front curved section around roller
    glBegin(GL_QUAD_STRIP);
        glNormal3f(0.0f, 0.0f, 1.0f);
        for (int i = 0; i <= 8; i++) {
            float angle = M_PI * i / 8.0f;
            float y = 2.1f + rollerRadius * cos(angle);
            float z = beltZMax + rollerRadius * sin(angle);

            glTexCoord2f(0.0f + texOffset, (float)i/8.0f); glVertex3f(beltLeft, y, z);
            glTexCoord2f(5.0f + texOffset, (float)i/8.0f); glVertex3f(beltRight, y, z);
        }
    glEnd();

    // Back curved section around roller
    glBegin(GL_QUAD_STRIP);
        glNormal3f(0.0f, 0.0f, -1.0f);
        for (int i = 0; i <= 8; i++) {
            float angle = M_PI * i / 8.0f;
            float y = 2.1f + rollerRadius * cos(angle);
            float z = beltZMin - rollerRadius * sin(angle);

            glTexCoord2f(5.0f + texOffset, (float)i/8.0f); glVertex3f(beltRight, y, z);
            glTexCoord2f(0.0f + texOffset, (float)i/8.0f); glVertex3f(beltLeft, y, z);
        }
    glEnd();

    glDisable(GL_TEXTURE_2D);
}

// Draw the rollers at each end of the conveyor
void drawRollers() {
    // Enable texturing for the rollers
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, metalTex);

    glColor3f(0.7f, 0.7f, 0.7f); // Metallic gray color

    // Calculate roller positions
    float frontRollerPos = beltZMax;
    float backRollerPos = beltZMin;
    float rollerHeight = 2.1f;
    float rollerLength = beltXLength + 0.4f; // Slightly wider than the belt

    // Front end roller (perpendicular to belt direction)
    glPushMatrix();
        glTranslatef(0.0f, rollerHeight, frontRollerPos);
        // Rotate to align with X-axis (perpendicular to belt movement direction)
        glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
        // Rotate the roller based on belt movement
        glRotatef(fmod(beltOffset * 360.0f / (2.0f * M_PI * rollerRadius), 360.0f), 0.0f, 0.0f, 1.0f);
        drawCylinder(rollerRadius, rollerLength, rollerSegments);
    glPopMatrix();

    // Back end roller (perpendicular to belt direction)
    glPushMatrix();
        glTranslatef(0.0f, rollerHeight, backRollerPos);
        // Rotate to align with X-axis (perpendicular to belt movement direction)
        glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
        // Rotate the roller based on belt movement
        glRotatef(fmod(beltOffset * 360.0f / (2.0f * M_PI * rollerRadius), 360.0f), 0.0f, 0.0f, 1.0f);
        drawCylinder(rollerRadius, rollerLength, rollerSegments);
    glPopMatrix();

    // Add idler rollers underneath (supporting the bottom portion of belt)
    int numIdlers = 6;
    float idlerSpacing = (frontRollerPos - backRollerPos) / (numIdlers + 1);
    float idlerRadius = rollerRadius / 2.0f;

    for (int i = 1; i <= numIdlers; i++) {
        float idlerPos = backRollerPos + i * idlerSpacing;
        glPushMatrix();
            glTranslatef(0.0f, rollerHeight - rollerRadius - idlerRadius, idlerPos);
            // Rotate to align with X-axis (perpendicular to belt direction)
            glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
            // Rotate the idler based on belt movement
            glRotatef(fmod(-beltOffset * 360.0f / (2.0f * M_PI * idlerRadius), 360.0f), 0.0f, 0.0f, 1.0f);
            drawCylinder(idlerRadius, rollerLength * 0.8f, rollerSegments);
        glPopMatrix();
    }

    glDisable(GL_TEXTURE_2D);
}

// Draw the support structure and frame for the conveyor
void drawConveyorSupport() {
    float supportHeight = 2.0f;
    float legWidth = 0.3f;
    float baseHeight = 0.2f;

    glBindTexture(GL_TEXTURE_2D, metalTex);
    glEnable(GL_TEXTURE_2D);

    glColor3f(0.5f, 0.5f, 0.55f); // Dark metallic color for frame

    // Main support legs
    for (float x : {-19.5f, -10.0f, 0.0f, 10.0f, 19.5f}) {
        // Front leg
        glPushMatrix();
            glTranslatef(x, supportHeight/2.0f, beltZMax + 0.2f);
            glScalef(legWidth, supportHeight, legWidth);
            glutSolidCube(1.0);
        glPopMatrix();

        // Back leg
        glPushMatrix();
            glTranslatef(x, supportHeight/2.0f, beltZMin - 0.2f);
            glScalef(legWidth, supportHeight, legWidth);
            glutSolidCube(1.0);
        glPopMatrix();
    }

    // Horizontal supports connecting legs along x-axis
    for (float z : {beltZMin - 0.2f, beltZMax + 0.2f}) {
        // Lower horizontal support
        glPushMatrix();
            glTranslatef(0.0f, 0.6f, z);
            glScalef(39.5f, legWidth, legWidth);
            glutSolidCube(1.0);
        glPopMatrix();

        // Upper horizontal support
        glPushMatrix();
            glTranslatef(0.0f, 1.8f, z);
            glScalef(39.5f, legWidth, legWidth);
            glutSolidCube(1.0);
        glPopMatrix();
    }

    // Horizontal supports along z-axis
    for (float x : {-19.5f, 19.5f}) {
        // Lower horizontal support
        glPushMatrix();
            glTranslatef(x, 0.6f, (beltZMin + beltZMax) / 2.0f);
            glScalef(legWidth, legWidth, beltWidth + 0.4f);
            glutSolidCube(1.0);
        glPopMatrix();

        // Upper horizontal support
        glPushMatrix();
            glTranslatef(x, 1.8f, (beltZMin + beltZMax) / 2.0f);
            glScalef(legWidth, legWidth, beltWidth + 0.4f);
            glutSolidCube(1.0);
        glPopMatrix();
    }

    // Cross braces for stability along x-axis
    for (float xStart : {-19.5f, -10.0f, 0.0f, 10.0f}) {
        float xEnd = xStart + 9.5f;

        // Front cross brace
        glBegin(GL_QUADS);
            glVertex3f(xStart, 0.6f + legWidth/2.0f, beltZMax + 0.2f);
            glVertex3f(xEnd, 1.8f - legWidth/2.0f, beltZMax + 0.2f);
            glVertex3f(xEnd, 1.8f + legWidth/2.0f, beltZMax + 0.2f);
            glVertex3f(xStart, 0.6f - legWidth/2.0f, beltZMax + 0.2f);
        glEnd();

        // Back cross brace
        glBegin(GL_QUADS);
            glVertex3f(xStart, 0.6f + legWidth/2.0f, beltZMin - 0.2f);
            glVertex3f(xEnd, 1.8f - legWidth/2.0f, beltZMin - 0.2f);
            glVertex3f(xEnd, 1.8f + legWidth/2.0f, beltZMin - 0.2f);
            glVertex3f(xStart, 0.6f - legWidth/2.0f, beltZMin - 0.2f);
        glEnd();
    }

    // Motor housing at one end (right side of X axis, side of the conveyor)
    glColor3f(0.3f, 0.3f, 0.3f); // Darker color for motor
    glPushMatrix();
        glTranslatef(21.0f, 2.1f, (beltZMin + beltZMax) / 2.0f);
        glScalef(1.0f, 1.2f, 1.3f);
        glutSolidCube(1.0);
    glPopMatrix();

    // Drive shaft from motor to belt roller
    glPushMatrix();
        glTranslatef(20.2f, 2.1f, (beltZMin + beltZMax) / 2.0f);
        glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
        drawCylinder(0.15f, 0.8f, 12);
    glPopMatrix();

    // Bearing housings at roller endpoints
    glColor3f(0.6f, 0.6f, 0.6f);

    // Front roller bearings
    for (float x : {-20.2f, 20.2f}) {
        glPushMatrix();
            glTranslatef(x, 2.1f, beltZMax);
            glScalef(0.5f, 0.7f, 0.3f);
            glutSolidCube(1.0);
        glPopMatrix();
    }

    // Back roller bearings
    for (float x : {-20.2f, 20.2f}) {
        glPushMatrix();
            glTranslatef(x, 2.1f, beltZMin);
            glScalef(0.5f, 0.7f, 0.3f);
            glutSolidCube(1.0);
        glPopMatrix();
    }

    // Base along the floor
    glColor3f(0.4f, 0.4f, 0.45f);
    glPushMatrix();
        glTranslatef(0.0f, baseHeight/2.0f, (beltZMin + beltZMax) / 2.0f);
        glScalef(40.0f, baseHeight, beltWidth + 1.5f);
        glutSolidCube(1.0);
    glPopMatrix();

    glDisable(GL_TEXTURE_2D);
}

//------------------- Draw Full Conveyor Belt System ---------------------
void drawConveyorBeltSystem() {
    // Draw all components of the enhanced conveyor belt
    drawConveyorSupport();
    drawBeltSurface();
    drawRollers();
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
        // Adjusted y-position to account for thicker belt
        glTranslatef(worldX, 2.1f + rollerRadius + 0.5f, (beltZMin + beltZMax) / 2.0f);

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

//------------------- Loads Textures ---------------------------
void loadTextures() {
    // Floor texture
    glGenTextures(1, &floorTex);
    glBindTexture(GL_TEXTURE_2D, floorTex);
    loadTGA("concrete.tga");
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    // Belt texture (assuming you have a rubber.tga file)
    glGenTextures(1, &beltTex);
    glBindTexture(GL_TEXTURE_2D, beltTex);
    loadTGA("concrete.tga");  // Replace with your conveyor belt texture
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    // Metal texture (assuming you have a metal.tga file)
    glGenTextures(1, &metalTex);
    glBindTexture(GL_TEXTURE_2D, metalTex);
    loadTGA("concrete.tga");  // Replace with your metal texture
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

    // Draw the enhanced conveyor belt
    drawConveyorBeltSystem();

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
    glutCreateWindow("Enhanced Conveyor Belt");

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