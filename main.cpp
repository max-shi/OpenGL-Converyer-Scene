//  ========================================================================
//  Max Shi - Cosc363
//  ========================================================================

#include <cmath>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <vector>
#include <GL/freeglut.h>
#include "loadTGA.h"
#include "keyboardUtilities.h"
using namespace std;

GLfloat globalLightAmbient[]  = {0.2f, 0.2f, 0.2f, 1.0f};
GLfloat globalLightDiffuse[]  = {0.8f, 0.8f, 0.8f, 1.0f};
GLfloat globalLightSpecular[] = {1.0f, 1.0f, 1.0f, 1.0f};
GLfloat globalLightPosition[] = {0.0f, 30.0f, 50.0f, 1.0f};

GLfloat matAmbient[]   = {0.2f, 0.2f, 0.2f, 1.0f};
GLfloat matDiffuse[]   = {0.8f, 0.8f, 0.8f, 1.0f};
GLfloat matSpecular[]  = {1.0f, 1.0f, 1.0f, 1.0f};
GLfloat matShininess[] = {50.0f};

bool isShadowPass = false;
void setCustomColor(GLfloat r, GLfloat g, GLfloat b) {
    if(isShadowPass)
        glColor3f(0.2f, 0.2f, 0.2f);
    else
        glColor3f(r, g, b);
}

void computeShadowMatrix(GLfloat shadowMat[4][4], GLfloat groundPlane[4], GLfloat lightPos[4]) {
    GLfloat dot = groundPlane[0]*lightPos[0] +
                  groundPlane[1]*lightPos[1] +
                  groundPlane[2]*lightPos[2] +
                  groundPlane[3]*lightPos[3];
    shadowMat[0][0] = dot - lightPos[0] * groundPlane[0];
    shadowMat[1][0] = 0.0f - lightPos[0] * groundPlane[1];
    shadowMat[2][0] = 0.0f - lightPos[0] * groundPlane[2];
    shadowMat[3][0] = 0.0f - lightPos[0] * groundPlane[3];
    shadowMat[0][1] = 0.0f - lightPos[1] * groundPlane[0];
    shadowMat[1][1] = dot - lightPos[1] * groundPlane[1];
    shadowMat[2][1] = 0.0f - lightPos[1] * groundPlane[2];
    shadowMat[3][1] = 0.0f - lightPos[1] * groundPlane[3];
    shadowMat[0][2] = 0.0f - lightPos[2] * groundPlane[0];
    shadowMat[1][2] = 0.0f - lightPos[2] * groundPlane[1];
    shadowMat[2][2] = dot - lightPos[2] * groundPlane[2];
    shadowMat[3][2] = 0.0f - lightPos[2] * groundPlane[3];
    shadowMat[0][3] = 0.0f - lightPos[3] * groundPlane[0];
    shadowMat[1][3] = 0.0f - lightPos[3] * groundPlane[1];
    shadowMat[2][3] = 0.0f - lightPos[3] * groundPlane[2];
    shadowMat[3][3] = dot - lightPos[3] * groundPlane[3];
}

GLuint floorTex;
GLuint beltTex;
GLuint metalTex;

int numItems = 8;

float beltOffset = 0.0f;
float beltSpeed = 1.f / 60.f;
float beltXLength = 40.0f;
const float beltZMin = -5.0f;
const float beltZMax = -3.0f;
const float beltWidth = beltZMax - beltZMin;
const float beltThickness = 0.1f;
const float rollerRadius = 0.3f;
float rollerRotation = 0.0f;

KeyboardUtilities keyboardUtil;

const int MAX_PARTICLES = 500;
const float PARTICLE_LIFETIME = 1.5f;
const float SPARK_GRAVITY = -9.8f;
const float FLOOR_Y = 0.0f;
const float BOUNCE_DAMPING = 0.6f;

struct Particle {
    float x, y, z;
    float vx, vy, vz;
    float r, g, b;
    float lifetime;
    float maxLifetime;
    float scale;
    bool active;
};

vector<Particle> particles(MAX_PARTICLES);
bool sparkGeneration = false;
float timeSinceLastEmission = 0.0f;
float emissionRate = 0.001f;

//------------------- Initialize Particle System ----------------------
void initParticleSystem() {
    for (int i = 0; i < MAX_PARTICLES; i++) {
        particles[i].active = false;
    }
}

//------------------- Create New Particle ----------------------------
void createParticle(float originX, float originY, float originZ) {
    for (int i = 0; i < MAX_PARTICLES; i++) {
        if (!particles[i].active) {
            particles[i].x = originX;
            particles[i].y = originY;
            particles[i].z = originZ;
            float angle = (float)(rand() % 360) * M_PI / 180.0f;
            float speed = 2.0f + (float)(rand() % 400) / 100.0f;
            particles[i].vx = speed * cos(angle);
            particles[i].vz = speed * sin(angle);
            particles[i].vy = 1.0f + (float)(rand() % 100) / 50.0f;
            particles[i].r = 0.9f + (float)(rand() % 10) / 100.0f;
            particles[i].g = 0.5f + (float)(rand() % 50) / 100.0f;
            particles[i].b = (float)(rand() % 20) / 100.0f;
            particles[i].maxLifetime = PARTICLE_LIFETIME * (0.5f + (float)(rand() % 50) / 100.0f);
            particles[i].lifetime = particles[i].maxLifetime;
            particles[i].scale = 0.05f + (float)(rand() % 10) / 100.0f;
            particles[i].active = true;
            return;
        }
    }
}

//------------------- Update Particles ------------------------------
void updateParticles(float deltaTime) {
    for (int i = 0; i < MAX_PARTICLES; i++) {
        if (particles[i].active) {
            particles[i].x += particles[i].vx * deltaTime;
            particles[i].y += particles[i].vy * deltaTime;
            particles[i].z += particles[i].vz * deltaTime;
            particles[i].vy += SPARK_GRAVITY * deltaTime;
            if (particles[i].y <= FLOOR_Y && particles[i].vy < 0) {
                particles[i].y = FLOOR_Y;
                particles[i].vy = -particles[i].vy * BOUNCE_DAMPING;
                particles[i].vx *= 0.9f;
                particles[i].vz *= 0.9f;
                if (fabs(particles[i].vy) < 0.5f) {
                    particles[i].vy = 0;
                }
            }
            particles[i].lifetime -= deltaTime;
            if (particles[i].lifetime <= 0) {
                particles[i].active = false;
            }
        }
    }
    if (sparkGeneration) {
        timeSinceLastEmission += deltaTime;
        while (timeSinceLastEmission >= emissionRate) {
            createParticle(0.0f, 2.5f + rollerRadius, (beltZMin + beltZMax) / 2.0f);
            timeSinceLastEmission -= emissionRate;
        }
    }
}

//------------------- Draw Particles --------------------------------
void drawParticles() {
    glDisable(GL_LIGHTING);
    glDepthMask(GL_FALSE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glEnable(GL_POINT_SMOOTH);
    glEnable(GL_POINT_SPRITE);
    glPointSize(10.0f);
    glBegin(GL_POINTS);
    for (int i = 0; i < MAX_PARTICLES; i++) {
        if (particles[i].active) {
            float lifeRatio = particles[i].lifetime / particles[i].maxLifetime;
            float alpha = lifeRatio;
            float r = particles[i].r;
            float g = particles[i].g * lifeRatio;
            float b = particles[i].b * lifeRatio * 0.5f;
            glColor4f(r, g, b, alpha);
            glPointSize(particles[i].scale * 10.0f * lifeRatio);
            glVertex3f(particles[i].x, particles[i].y, particles[i].z);
        }
    }
    glEnd();
    glDisable(GL_POINT_SPRITE);
    glDisable(GL_POINT_SMOOTH);
    glDisable(GL_BLEND);
    glDepthMask(GL_TRUE);
    glEnable(GL_LIGHTING);
}

void displayParticleCheck() {
    float spacing = beltXLength / numItems;
    sparkGeneration = false;
    for (int i = 0; i < numItems; i++) {
        float itemOffset = fmod(beltOffset + i * spacing, beltXLength);
        float worldX = -20.0f + itemOffset;
        if (worldX > -0.5f && worldX < 0.5f) {
            sparkGeneration = true;
            break;
        }
    }
}

//------------------- Update Scene (Belt, Rollers & Camera) -------------------------
void updateScene(int value) {
    static float lastTime = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
    float currentTime = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
    float deltaTime = currentTime - lastTime;
    lastTime = currentTime;
    beltOffset += beltSpeed;
    if (beltOffset > beltXLength)
        beltOffset = fmod(beltOffset, beltXLength);
    rollerRotation += beltSpeed * 180.0f / (M_PI * rollerRadius);
    keyboardUtil.update();
    displayParticleCheck();
    updateParticles(deltaTime);
    glutPostRedisplay();
    glutTimerFunc(16, updateScene, 0);
}

//------------------- Draw an Individual Roller ---------------------------
void drawRoller(float xPos) {
    float rollerLength = beltWidth;
    float yCenter = 2.1f;
    float zCenter = (beltZMin + beltZMax) / 2.0f;
    glPushMatrix();
        glTranslatef(xPos, yCenter, zCenter);
        glTranslatef(0.0f, 0.0f, -rollerLength / 2.0f);
        glRotatef(rollerRotation, 0.0f, 0.0f, -1.0f);
        glBindTexture(GL_TEXTURE_2D, floorTex);
        glEnable(GL_TEXTURE_2D);
        setCustomColor(0.7f, 0.7f, 0.7f);
        GLUquadric* quad = gluNewQuadric();
        gluQuadricTexture(quad, GL_TRUE);
        gluQuadricNormals(quad, GLU_SMOOTH);
        gluCylinder(quad, rollerRadius, rollerRadius, rollerLength, 32, 4);
        glPushMatrix();
            glTranslatef(0.0f, 0.0f, rollerLength);
            gluDisk(quad, 0.0f, rollerRadius, 32, 1);
        glPopMatrix();
        glPushMatrix();
            glRotatef(180.0f, 1.0f, 0.0f, 0.0f);
            gluDisk(quad, 0.0f, rollerRadius, 32, 1);
        glPopMatrix();
        gluDeleteQuadric(quad);
        glDisable(GL_TEXTURE_2D);
    glPopMatrix();
}

//------------------- Draw Rollers ---------------------------
void drawRollers() {
    int numRollers = 40;
    float spacing = beltXLength / (numRollers - 1);
    for (int i = 0; i < numRollers; i++) {
        float xPos = -20.0f + i * spacing;
        drawRoller(xPos);
    }
}

//------------------- Draw the Overhead Bar and Supports ---------------------------
void drawSpringBars(float zPosition) {
    float xLocation = -5.0f;
    float topY    = 12.0f;
    float barLen  = 12.0f;
    float barThk  = 0.4f;
    float sidePos = 6.0f;
    glBindTexture(GL_TEXTURE_2D, metalTex);
    glEnable(GL_TEXTURE_2D);
    setCustomColor(0.7f, 0.7f, 0.7f);
    glPushMatrix();
        glTranslatef(xLocation, topY, zPosition);
        glScalef(barLen, barThk, barThk);
        glutSolidCube(1.0f);
    glPopMatrix();
    glPushMatrix();
        glTranslatef(-sidePos + xLocation, topY / 2.0f, zPosition);
        glScalef(barThk, topY, barThk);
        glutSolidCube(1.0f);
    glPopMatrix();
    glPushMatrix();
        glTranslatef(sidePos + xLocation, topY / 2.0f, zPosition);
        glScalef(barThk, topY, barThk);
        glutSolidCube(1.0f);
    glPopMatrix();
    glDisable(GL_TEXTURE_2D);
}

//------------------- Draw a Single Spring ---------------------------
void drawSpring(float x, float z, float timeOffset, float maxHeight, float minHeight) {
    float springRadius    = 0.8f;
    float springThickness = 0.16f;
    float capSize         = 1.2f;
    float numCoils        = 10.0f;
    float coilBaseOffset  = 0.2f;
    float unscaledHeight = minHeight + (maxHeight - minHeight) *
                           (0.5f + 0.5f * sin(beltOffset * 3.0f + timeOffset));
    float currentHeight = unscaledHeight * 2.;
    glBindTexture(GL_TEXTURE_2D, metalTex);
    glEnable(GL_TEXTURE_2D);
    glPushMatrix();
        glTranslatef(x, 0.0f, z);
        GLUquadric* quad = gluNewQuadric();
        gluQuadricTexture(quad, GL_TRUE);
        gluQuadricNormals(quad, GLU_SMOOTH);
        setCustomColor(0.5f, 0.5f, 0.5f);
        glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
        float baseOuterRadius = springRadius + 0.1f;
        float baseHeight  = 0.1f;
        gluDisk(quad, 0.0f, baseOuterRadius, 16, 1);
        gluCylinder(quad, baseOuterRadius, baseOuterRadius, baseHeight, 16, 4);
        glTranslatef(0.0f, 0.0f, baseHeight);
        gluDisk(quad, 0.0f, baseOuterRadius, 16, 1);
        glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
        setCustomColor(0.7f, 0.7f, 0.7f);
        glBegin(GL_TRIANGLE_STRIP);
        for (int i = 0; i <= 360 * (int)numCoils; i += 5) {
            float angle = i * M_PI / 180.0f;
            float y = coilBaseOffset + (i / (360.0f * numCoils)) * currentHeight;
            float xC = springRadius * cos(angle);
            float zC = springRadius * sin(angle);
            float normalX = xC / springRadius;
            float normalZ = zC / springRadius;
            glNormal3f(normalX, 0.0f, normalZ);
            glTexCoord2f((float)i / (360.0f * numCoils), 0.0f);
            glVertex3f(xC - normalX * springThickness, y, zC - normalZ * springThickness);
            glNormal3f(normalX, 0.0f, normalZ);
            glTexCoord2f((float)i / (360.0f * numCoils), 1.0f);
            glVertex3f(xC + normalX * springThickness, y, zC + normalZ * springThickness);
        }
        glEnd();
        glTranslatef(0.0f, currentHeight + coilBaseOffset, 0.0f);
        setCustomColor(0.5f, 0.5f, 0.5f);
        glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
        gluDisk(quad, 0.0f, capSize, 16, 1);
        float capHeight = 0.6f;
        gluCylinder(quad, capSize, capSize, capHeight, 16, 4);
        glTranslatef(0.0f, 0.0f, capHeight);
        gluDisk(quad, 0.0f, capSize, 16, 1);
        gluDeleteQuadric(quad);
    glPopMatrix();
    glDisable(GL_TEXTURE_2D);
}

void drawBackgroundSprings() {
    drawSpringBars(-20.0f);
    drawSpring(-7.5f, -20.0f, 0.0f, 6.f, 2.0f);
    drawSpring(-5.0f, -20.0f, 2.0f, 6.f, 1.5f);
    drawSpring(-2.5f, -20.0f, 4.0f, 6.f, 2.5f);
    drawSpring(0.0f, -20.0f, 6.0f, 6.f, 2.5f);
    drawSpring(-10.0f, -20.0f, -2.2f, 6.f, 2.5f);
}

void drawConveyorBelt() {
    float xLeft = -20.0f, xRight = 20.0f;
    float yBase = 2.1f;
    float normalizedOffset = beltOffset / 7.5f;
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, beltTex);
    setCustomColor(0.9f, 0.9f, 0.9f);
    glBegin(GL_QUADS);
        glTexCoord2f(0.0f - normalizedOffset, 0.0f); glVertex3f(xLeft, yBase + rollerRadius, beltZMin);
        glTexCoord2f(0.0f - normalizedOffset, 1.0f); glVertex3f(xLeft, yBase + rollerRadius, beltZMax);
        glTexCoord2f(8.0f - normalizedOffset, 1.0f); glVertex3f(xRight, yBase + rollerRadius, beltZMax);
        glTexCoord2f(8.0f - normalizedOffset, 0.0f); glVertex3f(xRight, yBase + rollerRadius, beltZMin);
    glEnd();
    glBegin(GL_QUADS);
        glTexCoord2f(0.0f + normalizedOffset, 0.0f); glVertex3f(xLeft, yBase - rollerRadius, beltZMin);
        glTexCoord2f(8.0f + normalizedOffset, 0.0f); glVertex3f(xRight, yBase - rollerRadius, beltZMin);
        glTexCoord2f(8.0f + normalizedOffset, 1.0f); glVertex3f(xRight, yBase - rollerRadius, beltZMax);
        glTexCoord2f(0.0f + normalizedOffset, 1.0f); glVertex3f(xLeft, yBase - rollerRadius, beltZMax);
    glEnd();
    glBegin(GL_QUADS);
        glTexCoord2f(0.0f, 0.0f); glVertex3f(xLeft, yBase - rollerRadius, beltZMin);
        glTexCoord2f(0.0f, 0.1f); glVertex3f(xLeft, yBase - rollerRadius, beltZMax);
        glTexCoord2f(0.2f, 0.1f); glVertex3f(xLeft, yBase + rollerRadius, beltZMax);
        glTexCoord2f(0.2f, 0.0f); glVertex3f(xLeft, yBase + rollerRadius, beltZMin);
    glEnd();
    glBegin(GL_QUADS);
        glTexCoord2f(0.0f, 0.0f); glVertex3f(xRight, yBase - rollerRadius, beltZMin);
        glTexCoord2f(0.2f, 0.0f); glVertex3f(xRight, yBase + rollerRadius, beltZMin);
        glTexCoord2f(0.2f, 0.1f); glVertex3f(xRight, yBase + rollerRadius, beltZMax);
        glTexCoord2f(0.0f, 0.1f); glVertex3f(xRight, yBase - rollerRadius, beltZMax);
    glEnd();
    glDisable(GL_TEXTURE_2D);
}

void drawProcessedItem(float offset) {
    glPushMatrix();
        float worldX = -20.0f + offset;
        glTranslatef(worldX, 2.5f + rollerRadius, (beltZMin + beltZMax) / 2.0f);
        if (worldX < 0.0f) {
            // For items on the left, scale and draw a solid cube (blue).
            float scaleFactor;
            if (worldX <= -3.0f) {
                scaleFactor = 1.0f - 0.25f * ((worldX + 20.0f) / 17.0f);
            } else {
                scaleFactor = 0.75f;
            }
            glScalef(scaleFactor, scaleFactor, scaleFactor);
            setCustomColor(0.0f, 0.0f, 1.0f);
            glutSolidCube(1.0);
        } else {
            // For items on the right, draw a twisted cylindrical item (yellow) with proper normals.
            glTranslatef(0.0f, -rollerRadius, 0.0f);
            float growth = (worldX < 9.0f) ? (worldX / 9.0f) : 1.0f;
            float minHeight = 0.1f;
            float fullHeight = 1.0f;
            float cylinderHeight = minHeight + growth * (fullHeight - minHeight);
            float radius = 0.5f;
            float twist = 0.0f;
            float maxTwist = 90.0f;
            if (worldX >= 9.0f) {
                float twistFactor = (worldX - 9.0f) / (16.0f - 9.0f);
                if (twistFactor > 1.0f) twistFactor = 1.0f;
                twist = twistFactor * maxTwist;
            }
            int segments = 32;
            setCustomColor(0.9f, 0.9f, 0.0f);

            // Draw the twisted cylindrical side with proper normals.
            glBegin(GL_QUAD_STRIP);
                for (int i = 0; i <= segments; i++) {
                    float theta = 2.0f * M_PI * i / segments;
                    float xBottom = radius * cos(theta);
                    float zBottom = radius * sin(theta);
                    float twistedTheta = theta + twist * M_PI / 180.0f;
                    float xTop = radius * cos(twistedTheta);
                    float zTop = radius * sin(twistedTheta);

                    // Normal for bottom vertex (points outward).
                    glNormal3f(cos(theta), 0.0f, sin(theta));
                    glVertex3f(xBottom, 0.0f, zBottom);

                    // Normal for top vertex (accounting for twist).
                    glNormal3f(cos(twistedTheta), 0.0f, sin(twistedTheta));
                    glVertex3f(xTop, cylinderHeight, zTop);
                }
            glEnd();

            // Draw the bottom cap (flat, with normal pointing down).
            glBegin(GL_TRIANGLE_FAN);
                glNormal3f(0.0f, -1.0f, 0.0f);
                glVertex3f(0.0f, 0.0f, 0.0f);
                for (int i = 0; i <= segments; i++) {
                    float theta = 2.0f * M_PI * i / segments;
                    float x = radius * cos(theta);
                    float z = radius * sin(theta);
                    glVertex3f(x, 0.0f, z);
                }
            glEnd();

            // Draw the top cap (flat, with normal pointing up).
            glBegin(GL_TRIANGLE_FAN);
                glNormal3f(0.0f, 1.0f, 0.0f);
                glVertex3f(0.0f, cylinderHeight, 0.0f);
                for (int i = 0; i <= segments; i++) {
                    float theta = 2.0f * M_PI * i / segments;
                    float twistedTheta = theta + twist * M_PI / 180.0f;
                    float x = radius * cos(twistedTheta);
                    float z = radius * sin(twistedTheta);
                    glVertex3f(x, cylinderHeight, z);
                }
            glEnd();
        }
    glPopMatrix();
}


void drawSupportStructure() {
    float xLeft = -20.0f, xRight = 20.0f;
    float zBack = beltZMin - 0.5f, zFront = beltZMax + 0.5f;
    float yBottom = 0.0f, yTop = 2.1f - rollerRadius;
    float legWidth = 0.3f;
    glBindTexture(GL_TEXTURE_2D, floorTex);
    glEnable(GL_TEXTURE_2D);
    setCustomColor(1.f, 1.f, 1.f);
    glPushMatrix();
        glTranslatef(xLeft + legWidth/2, yBottom + (yTop-yBottom)/2, zFront - legWidth/2);
        glScalef(legWidth, yTop-yBottom, legWidth);
        glutSolidCube(1.0f);
    glPopMatrix();
    glPushMatrix();
        glTranslatef(xLeft + legWidth/2, yBottom + (yTop-yBottom)/2, zBack + legWidth/2);
        glScalef(legWidth, yTop-yBottom, legWidth);
        glutSolidCube(1.0f);
    glPopMatrix();
    glPushMatrix();
        glTranslatef(xRight - legWidth/2, yBottom + (yTop-yBottom)/2, zFront - legWidth/2);
        glScalef(legWidth, yTop-yBottom, legWidth);
        glutSolidCube(1.0f);
    glPopMatrix();
    glPushMatrix();
        glTranslatef(xRight - legWidth/2, yBottom + (yTop-yBottom)/2, zBack + legWidth/2);
        glScalef(legWidth, yTop-yBottom, legWidth);
        glutSolidCube(1.0f);
    glPopMatrix();
    int numSupports = 3;
    float supportSpacing = (xRight - xLeft) / (numSupports + 1);
    for (int i = 1; i <= numSupports; i++) {
        float xPos = xLeft + i * supportSpacing;
        glPushMatrix();
            glTranslatef(xPos, yBottom + (yTop-yBottom)/2, zFront - legWidth/2);
            glScalef(legWidth, yTop-yBottom, legWidth);
            glutSolidCube(1.0f);
        glPopMatrix();
        glPushMatrix();
            glTranslatef(xPos, yBottom + (yTop-yBottom)/2, zBack + legWidth/2);
            glScalef(legWidth, yTop-yBottom, legWidth);
            glutSolidCube(1.0f);
        glPopMatrix();
    }
    glPushMatrix();
        glTranslatef((xLeft + xRight)/2, yTop, zFront - legWidth/2);
        glScalef(xRight - xLeft, legWidth/2, legWidth);
        glutSolidCube(1.0f);
    glPopMatrix();
    glPushMatrix();
        glTranslatef((xLeft + xRight)/2, yTop, zBack + legWidth/2);
        glScalef(xRight - xLeft, legWidth/2, legWidth);
        glutSolidCube(1.0f);
    glPopMatrix();
    glPushMatrix();
        glTranslatef((xLeft + xRight)/2, yBottom + legWidth/2, zFront - legWidth/2);
        glScalef(xRight - xLeft, legWidth, legWidth);
        glutSolidCube(1.0f);
    glPopMatrix();
    glPushMatrix();
        glTranslatef((xLeft + xRight)/2, yBottom + legWidth/2, zBack + legWidth/2);
        glScalef(xRight - xLeft, legWidth, legWidth);
        glutSolidCube(1.0f);
    glPopMatrix();
    glPushMatrix();
        glTranslatef((xLeft + xRight)/2, yTop, zFront - legWidth/2);
        glScalef(xRight - xLeft, legWidth/2, legWidth);
        glutSolidCube(1.0f);
    glPopMatrix();
    glPushMatrix();
        glTranslatef((xLeft + xRight)/2, yTop, zBack + legWidth/2);
        glScalef(xRight - xLeft, legWidth/2, legWidth);
        glutSolidCube(1.0f);
    glPopMatrix();
    glPushMatrix();
        glTranslatef((xLeft + xRight)/2, yBottom + legWidth/2, zFront - legWidth/2);
        glScalef(xRight - xLeft, legWidth, legWidth);
        glutSolidCube(1.0f);
    glPopMatrix();
    glPushMatrix();
        glTranslatef((xLeft + xRight)/2, yBottom + legWidth/2, zBack + legWidth/2);
        glScalef(xRight - xLeft, legWidth, legWidth);
        glutSolidCube(1.0f);
    glPopMatrix();
    glPushMatrix();
        glTranslatef(xLeft + legWidth/2, yTop, (zFront + zBack)/2);
        glScalef(legWidth, legWidth/2, zFront - zBack);
        glutSolidCube(1.0f);
    glPopMatrix();
    glPushMatrix();
        glTranslatef(xRight - legWidth/2, yTop, (zFront + zBack)/2);
        glScalef(legWidth, legWidth/2, zFront - zBack);
        glutSolidCube(1.0f);
    glPopMatrix();
    setCustomColor(0.4f, 0.4f, 0.4f);
    glPushMatrix();
        glTranslatef((xLeft + xRight)/2, yTop - rollerRadius * 1.5f, (zFront + zBack)/2);
        glScalef(xRight - xLeft, legWidth/2, zFront - zBack - legWidth);
        glutSolidCube(1.0f);
    glPopMatrix();
    setCustomColor(0.45f, 0.45f, 0.45f);
    glPushMatrix();
        glTranslatef(xLeft + 1.0f, yBottom + (yTop-yBottom)/2, zFront - legWidth/2);
        glRotatef(30.0f, 0.0f, 0.0f, 1.0f);
        glScalef(sqrt(pow(yTop-yBottom, 2) + pow(2.0f, 2)), legWidth/3, legWidth/2);
        glutSolidCube(1.0f);
    glPopMatrix();
    glPushMatrix();
        glTranslatef(xLeft + 1.0f, yBottom + (yTop-yBottom)/2, zBack + legWidth/2);
        glRotatef(30.0f, 0.0f, 0.0f, 1.0f);
        glScalef(sqrt(pow(yTop-yBottom, 2) + pow(2.0f, 2)), legWidth/3, legWidth/2);
        glutSolidCube(1.0f);
    glPopMatrix();
    glPushMatrix();
        glTranslatef(xRight - 1.0f, yBottom + (yTop-yBottom)/2, zFront - legWidth/2);
        glRotatef(-30.0f, 0.0f, 0.0f, 1.0f);
        glScalef(sqrt(pow(yTop-yBottom, 2) + pow(2.0f, 2)), legWidth/3, legWidth/2);
        glutSolidCube(1.0f);
    glPopMatrix();
    glPushMatrix();
        glTranslatef(xRight - 1.0f, yBottom + (yTop-yBottom)/2, zBack + legWidth/2);
        glRotatef(-30.0f, 0.0f, 0.0f, 1.0f);
        glScalef(sqrt(pow(yTop-yBottom, 2) + pow(2.0f, 2)), legWidth/3, legWidth/2);
        glutSolidCube(1.0f);
    glPopMatrix();
    glDisable(GL_TEXTURE_2D);
}

//------------------- Draw Press Device ---------------------------
void drawPressDevice() {
    float baseX = 0.0f;
    float baseZ = -6.f;
    float baseY = 0.0f;
    float baseWidth = 2.0f;
    float baseDepth = 2.0f;
    float baseHeight = 7.f;
    float armLength = 3.0f;
    float pistonRadius = 0.4f;
    float headSize = 0.8f;
    float pressCycle = fmod(beltOffset * (2.0f * M_PI / 5.0f), 2.0f * M_PI);
    float pressPosition = 0.5f * sin(pressCycle + 0.6);
    glBindTexture(GL_TEXTURE_2D, metalTex);
    glEnable(GL_TEXTURE_2D);
    setCustomColor(0.6f, 0.6f, 0.6f);
    glPushMatrix();
        glTranslatef(baseX, baseY + baseHeight/2, baseZ);
        glScalef(baseWidth, baseHeight, baseDepth);
        glutSolidCube(1.0f);
    glPopMatrix();
    glPushMatrix();
        glTranslatef(baseX, baseY + baseHeight, baseZ + armLength/2);
        glScalef(baseWidth/1.5f, baseWidth/1.5f, armLength);
        glutSolidCube(1.0f);
    glPopMatrix();
    glPushMatrix();
        glTranslatef(baseX, baseY + baseHeight - baseWidth/3, beltZMin + beltWidth/2);
        glScalef(baseWidth/1.5f, baseWidth*1.5f, baseWidth/1.5f);
        glutSolidCube(1.0f);
    glPopMatrix();
    glPushMatrix();
        glTranslatef(baseX, baseY + baseHeight - baseWidth - pressPosition, beltZMin + beltWidth/2);
        GLUquadric* quad = gluNewQuadric();
        gluQuadricTexture(quad, GL_TRUE);
        gluQuadricNormals(quad, GLU_SMOOTH);
        glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
        setCustomColor(0.7f, 0.7f, 0.7f);
        gluCylinder(quad, pistonRadius, pistonRadius, 2.0f, 16, 4);
        setCustomColor(0.5f, 0.5f, 0.5f);
        glTranslatef(0.0f, 0.0f, 2.0f);
        glRotatef(180.0f, 1.0f, 0.0f, 0.0f);
        gluDisk(quad, 0.0f, headSize, 16, 1);
        gluCylinder(quad, headSize, headSize, headSize/2, 16, 4);
        glTranslatef(0.0f, 0.0f, headSize/2);
        gluDisk(quad, 0.0f, headSize, 16, 1);
        gluDeleteQuadric(quad);
    glPopMatrix();
    glDisable(GL_TEXTURE_2D);
    glPushMatrix();
        glTranslatef(baseX + baseWidth/2 + 0.1f, baseY + baseHeight*0.8f, baseZ);
        if (pressPosition < 0) {
            setCustomColor(1.0f, 0.2f, 0.2f);
        } else {
            setCustomColor(0.2f, 1.0f, 0.2f);
        }
        glutSolidSphere(0.2f, 16, 16);
    glPopMatrix();
    glDisable(GL_TEXTURE_2D);
}

//------------------- Draw Silo ---------------------------
void drawSilo(float x, float z) {
    const float siloHeight = 20.0f;
    const float siloRadius = 4.5f;
    const float roofHeight = 3.0f;
    const int segments = 32;

    glBindTexture(GL_TEXTURE_2D, metalTex);
    glEnable(GL_TEXTURE_2D);

    // Draw the cylindrical body of the silo
    setCustomColor(0.85f, 0.85f, 0.85f);
    glPushMatrix();
        glTranslatef(x, 0.0f, z);
        GLUquadric* quad = gluNewQuadric();
        gluQuadricTexture(quad, GL_TRUE);
        gluQuadricNormals(quad, GLU_SMOOTH);

        // Draw the main cylinder
        glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
        gluCylinder(quad, siloRadius, siloRadius, siloHeight, segments, 8);

        // Draw the bottom cap
        gluDisk(quad, 0.0f, siloRadius, segments, 1);

        // Draw the conical roof
        glTranslatef(0.0f, 0.0f, siloHeight);
        gluCylinder(quad, siloRadius, 0.0f, roofHeight, segments, 8);

        gluDeleteQuadric(quad);
    glPopMatrix();

    // Draw a small pipe/vent on top
    setCustomColor(0.6f, 0.6f, 0.6f);
    glPushMatrix();
        glTranslatef(x, siloHeight + roofHeight - 0.5f, z);
        glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);

        GLUquadric* pipeQuad = gluNewQuadric();
        gluQuadricTexture(pipeQuad, GL_TRUE);
        gluQuadricNormals(pipeQuad, GLU_SMOOTH);

        gluCylinder(pipeQuad, 0.3f, 0.3f, 1.0f, 16, 2);
        gluDeleteQuadric(pipeQuad);
    glPopMatrix();

    // Draw a small platform around the base
    setCustomColor(0.65f, 0.65f, 0.65f);
    glPushMatrix();
        glTranslatef(x, 0.1f, z);
        glScalef(siloRadius * 2.2f, 0.2f, siloRadius * 2.2f);
        glutSolidCube(1.0f);
    glPopMatrix();

    glDisable(GL_TEXTURE_2D);
}

//------------------- Draw All Silos ---------------------------
void drawSilos() {
    float startX = 15.0f;
    float z = -20.0f;
    float spacing = 9.0f;

    for (int i = 0; i < 4; i++) {
        float x = startX + i * spacing;
        drawSilo(x, z);
    }
}


//------------------- Loads Textures ---------------------------
void loadTextures() {

    glGenTextures(1, &floorTex);
    glBindTexture(GL_TEXTURE_2D, floorTex);
    loadTGA("concrete.tga");
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    glGenTextures(1, &beltTex);
    glBindTexture(GL_TEXTURE_2D, beltTex);
    loadTGA("concrete.tga");
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    glGenTextures(1, &metalTex);
    glBindTexture(GL_TEXTURE_2D, metalTex);
    loadTGA("metal.tga");
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
    setCustomColor(1.0f, 1.0f, 1.0f);
    glBegin(GL_QUADS);
        glTexCoord2f(0.0f, 0.0f); glVertex3f(-50.0f, 0.0f, -50.0f);
        glTexCoord2f(0.0f, 1.0f); glVertex3f(-50.0f, 0.0f,  50.0f);
        glTexCoord2f(1.0f, 1.0f); glVertex3f( 50.0f, 0.0f,  50.0f);
        glTexCoord2f(1.0f, 0.0f); glVertex3f( 50.0f, 0.0f, -50.0f);
    glEnd();
    glDisable(GL_TEXTURE_2D);
}


//------------------- Display Callback ---------------------------
void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    float radYaw = keyboardUtil.getYaw() * M_PI / 180.0f;
    float radPitch = keyboardUtil.getPitch() * M_PI / 180.0f;
    float lookDirX = cos(radPitch) * sin(radYaw);
    float lookDirY = sin(radPitch);
    float lookDirZ = -cos(radPitch) * cos(radYaw);
    gluLookAt(keyboardUtil.getPosX(), keyboardUtil.getPosY(), keyboardUtil.getPosZ(),
              keyboardUtil.getPosX() + lookDirX,
              keyboardUtil.getPosY() + lookDirY,
              keyboardUtil.getPosZ() + lookDirZ,
              0.0, 1.0, 0.0);
    glLightfv(GL_LIGHT0, GL_POSITION, globalLightPosition);
    glDisable(GL_LIGHTING);
    drawTexturedFloor();
    glEnable(GL_LIGHTING);
    drawSupportStructure();
    drawRollers();
    drawPressDevice();
    drawBackgroundSprings();
    drawConveyorBelt();
    drawSilos();
    float spacing = beltXLength / numItems;
    for (int i = 0; i < numItems; i++) {
        float itemOffset = fmod(beltOffset + i * spacing, beltXLength);
        drawProcessedItem(itemOffset);
    }
    drawParticles();
    GLfloat groundPlane[4] = {0.0f, 1.0f, 0.0f, 0.0f};
    GLfloat shadowMat[4][4];
    computeShadowMatrix(shadowMat, groundPlane, globalLightPosition);
    glPushAttrib(GL_ENABLE_BIT | GL_COLOR_BUFFER_BIT | GL_POLYGON_BIT);
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(-2.0f, -2.0f);
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    glColor4f(0.0f, 0.0f, 0.0f, 0.5f);
    glPushMatrix();
        glMultMatrixf((GLfloat *)shadowMat);
        isShadowPass = true;
        drawSupportStructure();
        drawRollers();
        drawPressDevice();
        drawBackgroundSprings();
        drawConveyorBelt();
        drawSilos();
        for (int i = 0; i < numItems; i++) {
            float itemOffset = fmod(beltOffset + i * spacing, beltXLength);
            drawProcessedItem(itemOffset);
        }
        isShadowPass = false;
    glPopMatrix();
    glPopAttrib();
    glutSwapBuffers();
}

//------------------- Keyboard and Special Key Callbacks ---------------------------
void keyboardDownCallback(unsigned char key, int x, int y) {
    keyboardUtil.keyboardDown(key);
    if (key == '+' || key == '=') {
        beltSpeed += 0.005f;
        if (beltSpeed > 0.1f) beltSpeed = 0.1f;
    }
    else if (key == '-' || key == '_') {
        beltSpeed -= 0.005f;
        if (beltSpeed < 0.0f) beltSpeed = 0.0f;
    }
    else if (key == 'r' || key == 'R') {
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

    // Set up the global light properties.
    glLightfv(GL_LIGHT0, GL_AMBIENT, globalLightAmbient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, globalLightDiffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, globalLightSpecular);
    // Set the default material.
    glMaterialfv(GL_FRONT, GL_AMBIENT, matAmbient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, matDiffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, matSpecular);
    glMaterialfv(GL_FRONT, GL_SHININESS, matShininess);

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
    initParticleSystem();
    glutDisplayFunc(display);
    glutKeyboardFunc(keyboardDownCallback);
    glutKeyboardUpFunc(keyboardUpCallback);
    glutSpecialFunc(specialKeyDownCallback);
    glutSpecialUpFunc(specialKeyUpCallback);
    glutTimerFunc(16, updateScene, 0);
    glutMainLoop();
    return 0;
}
