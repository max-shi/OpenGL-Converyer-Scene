//  ========================================================================
//  COSC363: Computer Graphics (2025);  University of Canterbury.
//
//  FILE NAME: Humanoid.cpp
//  See Lab02.pdf for details
//  ========================================================================

#include <cmath>
#include <iostream>
#include <fstream>
#include <GL/freeglut.h>
#include "loadTGA.h"
using namespace std;

//-- Globals ---------------------------------------------------------------
int cam_hgt = 4;    // Camera height
float angle = 10.;  // Rotation angle for viewing
GLuint floorTex;    // Texture ID for the floor

float beltOffset = 0.0f;     // Current position along the belt
float beltSpeed  = 0.1f;     // Speed of belt movement (units per update)
float beltLength = 30.0f;    // Total length of the conveyor belt
float beltWidth  = 5.0f;

void updateBelt(int value) {
    beltOffset += beltSpeed;
    if (beltOffset > beltLength)
        beltOffset = fmod(beltOffset, beltLength); // loop back

    glutPostRedisplay();
    glutTimerFunc(16, updateBelt, 0);
}

void drawConveyorBelt() {
    glColor3f(0.3f, 0.3f, 0.3f);
    glBegin(GL_QUADS);
    glVertex3f(-beltWidth/2, 2.0f, 0.0f);
    glVertex3f(-beltWidth/2, 2.0f, beltLength);
    glVertex3f( beltWidth/2, 2.0f, beltLength);
    glVertex3f( beltWidth/2, 2.0f, 0.0f);
    glEnd();
}

void drawItem(float zPos) {
    glPushMatrix();
    glTranslatef(0.0f, 2.5f, zPos);
    glColor3f(1.0f, 0.0f, 0.0f);
    glutSolidCube(1.0);
    glPopMatrix();
}


//-- Loads textures ---------------------------------------------------------
void loadTextures() {
    glGenTextures(1, &floorTex);              // Generate a texture ID
    glBindTexture(GL_TEXTURE_2D, floorTex);     // Bind the texture for use
    loadTGA("concrete.tga");                   // Load the TGA file (ensure it's in the proper path)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
}

//-- Draws a textured floor ------------------------------------------------
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

//-- Display: Main display function ----------------------------------------
void display() {
    float lpos[4] = {10., 10., 10., 1.}; // Light's position

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Set up the model view matrix and camera
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(0., cam_hgt, 10., 0., 4., 0., 0., 1., 0.);
    glLightfv(GL_LIGHT0, GL_POSITION, lpos);
    glRotatef(angle, 0., 1., 0.);

    // Disable lighting if desired so that the texture's colors are not altered,
    // then draw the textured floor, then re-enable lighting.
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

//-- Initialize OpenGL parameters ------------------------------------------
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

//-- Special key event callback --------------------------------------------
void special(int key, int x, int y) {
    if (key == GLUT_KEY_LEFT)  angle--;
    else if (key == GLUT_KEY_RIGHT) angle++;
    glutPostRedisplay();
}

//-- Main: Initialize glut window and register callbacks -------------------
int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_DEPTH);
    glutInitWindowSize(600, 600);
    glutInitWindowPosition(10, 10);
    glutCreateWindow("Main Application");
    initialize();
    glutDisplayFunc(display);
    glutTimerFunc(16, updateBelt, 0);
    glutSpecialFunc(special);
    glutMainLoop();
    return 0;
}
