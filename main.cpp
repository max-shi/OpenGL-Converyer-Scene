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

// Variate Globals
bool isShadowPass = false;
bool wireframeMode = false;
float fireBlasterTimeSinceLastEmission = 0.0f;
float beltOffset = 0.0f;
float beltSpeed = 1.f / 60.f;
bool sparkGeneration = false;
float timeSinceLastEmission = 0.0f;
float emissionRate = 0.001f;
float rollerRotation = 0.0f;

// Lighting Globals
const GLfloat GLOBAL_LIGHT_AMBIENT[]  = {0.2f, 0.2f, 0.2f, 1.0f};
const GLfloat GLOBAL_LIGHT_DIFFUSE[]  = {0.8f, 0.8f, 0.8f, 1.0f};
const GLfloat GLOBAL_LIGHT_SPECULAR[] = {1.0f, 1.0f, 1.0f, 1.0f};
GLfloat GLOBAL_LIGHT_POSITION[4] = {0.0f, 30.0f, 50.0f, 1.0f};

const GLfloat MATAMBIENT[]   = {0.2f, 0.2f, 0.2f, 1.0f};
const GLfloat MATDIFFUSE[]   = {0.8f, 0.8f, 0.8f, 1.0f};
const GLfloat MATSPECULAR[]  = {1.0f, 1.0f, 1.0f, 1.0f};
const GLfloat MATSHININESS[] = {50.0f};

// Floor Texure ID
GLuint floorTex;
GLuint beltTex;
GLuint metalTex;
GLuint metalPlateTex;
GLuint brickTex;
GLuint metalWallTex;

GLuint skyboxTex[6];

// Item variables
int NUM_ITEMS = 8;

// Fire blaster variables
const bool FIREBLASTER_ACTIVE = true;
const float FIREBLASTER_X = -10.0f;
const float FIREBLASTER_MIN_X = -15.0f;
const float FIREBLASTER_MAX_X = -5.0f;
const float FIREBLASTER_EMISSION_RATE = 0.0005f;

// Belt variables
const float BELT_X_LENGTH = 40.0f;
const float BELT_Z_MIN = -5.0f;
const float BELT_Z_MAX = -3.0f;
const float BELT_WIDTH = BELT_Z_MAX - BELT_Z_MIN;
const float BELT_THICKNESS = 0.1f;
const float ROLLER_RADIUS = 0.3f;

// Roller variables
const float ROLLER_LENGTH = BELT_WIDTH;
const float ROLLER_Y_CENTER = 2.1f;
const float ROLLER_Z_CENTER = (BELT_Z_MIN + BELT_Z_MAX) / 2.0f;
const int   NUM_ROLLERS = 40;
const float ROLLER_SPACING = BELT_X_LENGTH / (NUM_ROLLERS - 1);

// Springbar variables
const float SPRINGBAR_X_LOCATION = -5.0f;
const float SPRINGBAR_TOP_Y    = 12.0f;
const float SPRINGBAR_BAR_LENGTH  = 12.0f;
const float SPRINGBAR_BAR_THICKNESS  = 0.4f;
const float SPRINGBAR_SIDE_POS = 6.0f;

// Spring variables
const float SPRING_RADIUS    = 0.8f;
const float SPRING_THICKNESS = 0.16f;
const float SPRING_CAP_SIZE         = 1.2f;
const float SPRING_NUM_COILS        = 10.0f;
const float SPRING_BASE_OFFSET  = 0.2f;
const float SPRING_BASE_OUTER_RADIUS = SPRING_RADIUS + 0.1f;
const float SPRING_BASE_HEIGHT  = 0.1f;

// Particle Variables
const int MAX_PARTICLES = 2000;
const float PARTICLE_LIFETIME = 1.5f;
const float SPARK_GRAVITY = -9.8f;
const float FLOOR_Y = 0.0f;
const float BOUNCE_DAMPING = 0.6f;

const float DEFAULT_FLOOR_Y = 0.0f;
const float CONVEYOR_FLOOR_Y = 2.0f;
const float CONVEYOR_X_MIN = -20.0f;
const float CONVEYOR_X_MAX = 20.0f;
const float CONVEYOR_Z_MIN = -5.0f;
const float CONVEYOR_Z_MAX = -1.0f;
const float SPACING = BELT_X_LENGTH / NUM_ITEMS;


// Models
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



void setCustomColor(GLfloat r, GLfloat g, GLfloat b) {
    if(isShadowPass)
        glColor3f(0.1f, 0.1f, 0.1f);
    else
        glColor3f(r, g, b);
}

void computeShadowMatrix(GLfloat shadowMat[4][4], GLfloat groundPlane[4], GLfloat lightPos[4]) {
    GLfloat dot = groundPlane[0]*lightPos[0] +
                  groundPlane[1]*lightPos[1] +
                  groundPlane[2]*lightPos[2] +
                  groundPlane[3]*lightPos[3];
    shadowMat[0][0] = dot - lightPos[0] * groundPlane[0];
    shadowMat[1][0] = - lightPos[0] * groundPlane[1];
    shadowMat[2][0] = - lightPos[0] * groundPlane[2];
    shadowMat[3][0] = - lightPos[0] * groundPlane[3];
    shadowMat[0][1] = - lightPos[1] * groundPlane[0];
    shadowMat[1][1] = dot - lightPos[1] * groundPlane[1];
    shadowMat[2][1] = - lightPos[1] * groundPlane[2];
    shadowMat[3][1] = - lightPos[1] * groundPlane[3];
    shadowMat[0][2] = - lightPos[2] * groundPlane[0];
    shadowMat[1][2] = - lightPos[2] * groundPlane[1];
    shadowMat[2][2] = dot - lightPos[2] * groundPlane[2];
    shadowMat[3][2] = - lightPos[2] * groundPlane[3];
    shadowMat[0][3] = - lightPos[3] * groundPlane[0];
    shadowMat[1][3] = - lightPos[3] * groundPlane[1];
    shadowMat[2][3] = - lightPos[3] * groundPlane[2];
    shadowMat[3][3] = dot - lightPos[3] * groundPlane[3];
}

// Helper functions for conditional texture binding
void bindTextureIfNeeded(GLuint tex) {
    if (!isShadowPass) {
        glBindTexture(GL_TEXTURE_2D, tex);
        glEnable(GL_TEXTURE_2D);
    }
}

void disableTextureIfNeeded() {
    if (!isShadowPass) {
        glDisable(GL_TEXTURE_2D);
    }
}

KeyboardUtilities keyboardUtil;

//------------------- Initialize Particle System ----------------------
void initParticleSystem() {
    for (int i = 0; i < MAX_PARTICLES; i++) {
        particles[i].active = false;
    }
}

//------------------- Create New Particle ----------------------------
void createParticle(float originX, float originY, float originZ) {
    for (int i = 0; i < MAX_PARTICLES/4; i++) {
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

//------------------- Create New Fire Particle ----------------------------
void createFireParticle(float originX, float originY, float originZ) {
    for (int i = 0; i < MAX_PARTICLES; i++) {
        if (!particles[i].active) {
            particles[i].x = originX;
            particles[i].y = originY;
            particles[i].z = originZ;
            float angle = (float)(rand() % 180 - 90) * M_PI / 180.0f;
            float speed = 0.5f + (float)(rand() % 200) / 100.0f;
            particles[i].vx = speed * cos(angle) * 0.5f;
            particles[i].vz = speed * sin(angle) * 0.5f;
            particles[i].vy = 1.0f + (float)(rand() % 200) / 100.0f;
            // More orange-red colors
            particles[i].r = 0.9f + (float)(rand() % 10) / 100.0f;
            particles[i].g = 0.3f + (float)(rand() % 40) / 100.0f;
            particles[i].b = (float)(rand() % 15) / 100.0f;
            particles[i].maxLifetime = PARTICLE_LIFETIME * (0.3f + (float)(rand() % 70) / 100.0f);
            particles[i].lifetime = particles[i].maxLifetime;
            particles[i].scale = 0.07f + (float)(rand() % 15) / 100.0f;
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
            // float currentFloorY = DEFAULT_FLOOR_Y;
            // if (particles[i].x >= CONVEYOR_X_MIN && particles[i].x <= CONVEYOR_X_MAX &&
            //     particles[i].z >= CONVEYOR_Z_MIN && particles[i].z <= CONVEYOR_Z_MAX) {
            //     currentFloorY = CONVEYOR_FLOOR_Y;
            // }
            if (particles[i].y <= DEFAULT_FLOOR_Y && particles[i].vy < 0) {
                particles[i].y = DEFAULT_FLOOR_Y;
                particles[i].vy = -particles[i].vy * BOUNCE_DAMPING;
                particles[i].vx *= 0.9f;
                particles[i].vz *= 0.9f;
                if (fabs(particles[i].vy) < 0.5f)
                    particles[i].vy = 0;
            }
            particles[i].lifetime -= deltaTime * 0.75;
            if (particles[i].lifetime <= 0)
                particles[i].active = false;
        }
    }
    if (sparkGeneration) {
        timeSinceLastEmission += deltaTime;
        while (timeSinceLastEmission >= emissionRate) {
            createParticle(0.0f, 2.5f + ROLLER_RADIUS, (BELT_Z_MIN + BELT_Z_MAX) / 2.0f);
            timeSinceLastEmission -= emissionRate;
        }
    }
    if (FIREBLASTER_ACTIVE) {
        fireBlasterTimeSinceLastEmission += deltaTime;
        while (fireBlasterTimeSinceLastEmission >= FIREBLASTER_EMISSION_RATE) {
            float randomX = FIREBLASTER_MIN_X + (float)(rand() % 100) / 100.0f * (FIREBLASTER_MAX_X - FIREBLASTER_MIN_X);
            createFireParticle(randomX, 8.0f, (BELT_Z_MIN + BELT_Z_MAX) / 2.0f);
            fireBlasterTimeSinceLastEmission -= FIREBLASTER_EMISSION_RATE;
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
    sparkGeneration = false;
    for (int i = 0; i < NUM_ITEMS; i++) {
        float itemOffset = fmod(beltOffset + i * SPACING, BELT_X_LENGTH);
        float worldX = -20.0f + itemOffset;
        if (worldX > -0.5f && worldX < 0.5f) {
            sparkGeneration = true;
            break;
        }
    }
}

//------------------- Draw Textured Cube ---------------------------
void drawTexturedCube(float width, float height, float depth) {
    float hw = width / 2.0f;
    float hh = height / 2.0f;
    float hd = depth / 2.0f;
    glBegin(GL_QUADS);
        glNormal3f(0.0f, 0.0f, 1.0f);
        glTexCoord2f(0.0f, 0.0f); glVertex3f(-hw, -hh, hd);
        glTexCoord2f(1.0f, 0.0f); glVertex3f(hw, -hh, hd);
        glTexCoord2f(1.0f, 1.0f); glVertex3f(hw, hh, hd);
        glTexCoord2f(0.0f, 1.0f); glVertex3f(-hw, hh, hd);
    glEnd();
    glBegin(GL_QUADS);
        glNormal3f(0.0f, 0.0f, -1.0f);
        glTexCoord2f(1.0f, 0.0f); glVertex3f(-hw, -hh, -hd);
        glTexCoord2f(1.0f, 1.0f); glVertex3f(-hw, hh, -hd);
        glTexCoord2f(0.0f, 1.0f); glVertex3f(hw, hh, -hd);
        glTexCoord2f(0.0f, 0.0f); glVertex3f(hw, -hh, -hd);
    glEnd();
    glBegin(GL_QUADS);
        glNormal3f(-1.0f, 0.0f, 0.0f);
        glTexCoord2f(0.0f, 0.0f); glVertex3f(-hw, -hh, -hd);
        glTexCoord2f(1.0f, 0.0f); glVertex3f(-hw, -hh, hd);
        glTexCoord2f(1.0f, 1.0f); glVertex3f(-hw, hh, hd);
        glTexCoord2f(0.0f, 1.0f); glVertex3f(-hw, hh, -hd);
    glEnd();
    glBegin(GL_QUADS);
        glNormal3f(1.0f, 0.0f, 0.0f);
        glTexCoord2f(0.0f, 0.0f); glVertex3f(hw, -hh, hd);
        glTexCoord2f(1.0f, 0.0f); glVertex3f(hw, -hh, -hd);
        glTexCoord2f(1.0f, 1.0f); glVertex3f(hw, hh, -hd);
        glTexCoord2f(0.0f, 1.0f); glVertex3f(hw, hh, hd);
    glEnd();
    glBegin(GL_QUADS);
        glNormal3f(0.0f, 1.0f, 0.0f);
        glTexCoord2f(0.0f, 0.0f); glVertex3f(-hw, hh, hd);
        glTexCoord2f(1.0f, 0.0f); glVertex3f(hw, hh, hd);
        glTexCoord2f(1.0f, 1.0f); glVertex3f(hw, hh, -hd);
        glTexCoord2f(0.0f, 1.0f); glVertex3f(-hw, hh, -hd);
    glEnd();
    glBegin(GL_QUADS);
        glNormal3f(0.0f, -1.0f, 0.0f);
        glTexCoord2f(0.0f, 0.0f); glVertex3f(-hw, -hh, -hd);
        glTexCoord2f(1.0f, 0.0f); glVertex3f(hw, -hh, -hd);
        glTexCoord2f(1.0f, 1.0f); glVertex3f(hw, -hh, hd);
        glTexCoord2f(0.0f, 1.0f); glVertex3f(-hw, -hh, hd);
    glEnd();
}

//------------------- Update Scene (Belt, Rollers & Camera) -------------------------
void updateScene(int value) {
    static float lastTime = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
    float currentTime = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
    float deltaTime = currentTime - lastTime;
    lastTime = currentTime;
    beltOffset += beltSpeed;
    if (beltOffset > BELT_X_LENGTH)
        beltOffset = fmod(beltOffset, BELT_X_LENGTH);
    rollerRotation += beltSpeed * 180.0f / (M_PI * ROLLER_RADIUS);
    keyboardUtil.update();
    displayParticleCheck();
    updateParticles(deltaTime);
    glutPostRedisplay();
    glutTimerFunc(16, updateScene, 0);
}

//------------------- Draw an Individual Roller ---------------------------
void drawRoller(float xPos) {
    glPushMatrix();
        glTranslatef(xPos, ROLLER_Y_CENTER, ROLLER_Z_CENTER);
        glTranslatef(0.0f, 0.0f, -ROLLER_LENGTH / 2.0f);
        glRotatef(rollerRotation, 0.0f, 0.0f, -1.0f);
        bindTextureIfNeeded(floorTex);
        setCustomColor(0.7f, 0.7f, 0.7f);
        GLUquadric* quad = gluNewQuadric();
        gluQuadricTexture(quad, GL_TRUE);
        gluQuadricNormals(quad, GLU_SMOOTH);
        gluCylinder(quad, ROLLER_RADIUS, ROLLER_RADIUS, ROLLER_LENGTH, 32, 4);
        glPushMatrix();
            glTranslatef(0.0f, 0.0f, ROLLER_LENGTH);
            gluDisk(quad, 0.0f, ROLLER_RADIUS, 32, 1);
        glPopMatrix();
        glPushMatrix();
            glRotatef(180.0f, 1.0f, 0.0f, 0.0f);
            gluDisk(quad, 0.0f, ROLLER_RADIUS, 32, 1);
        glPopMatrix();
        gluDeleteQuadric(quad);
        disableTextureIfNeeded();
    glPopMatrix();
}

//------------------- Draw Rollers ---------------------------
void drawRollers() {
    for (int i = 0; i < NUM_ROLLERS; i++) {
        float xPos = -20.0f + i * ROLLER_SPACING;
        drawRoller(xPos);
    }
}


//------------------- Draw the Overhead Bar and Supports ---------------------------
void drawSpringBars(float zPosition) {
    bindTextureIfNeeded(metalTex);
    setCustomColor(0.7f, 0.7f, 0.7f);
    glPushMatrix();
        glTranslatef(SPRINGBAR_X_LOCATION, SPRINGBAR_TOP_Y, zPosition);
        drawTexturedCube(SPRINGBAR_BAR_LENGTH, SPRINGBAR_BAR_THICKNESS, SPRINGBAR_BAR_THICKNESS);
    glPopMatrix();
    glPushMatrix();
        glTranslatef(-SPRINGBAR_SIDE_POS + SPRINGBAR_X_LOCATION, SPRINGBAR_TOP_Y / 2.0f, zPosition);
        drawTexturedCube(SPRINGBAR_BAR_THICKNESS, SPRINGBAR_TOP_Y, SPRINGBAR_BAR_THICKNESS);
    glPopMatrix();
    glPushMatrix();
        glTranslatef(SPRINGBAR_SIDE_POS + SPRINGBAR_X_LOCATION, SPRINGBAR_TOP_Y / 2.0f, zPosition);
        drawTexturedCube(SPRINGBAR_BAR_THICKNESS, SPRINGBAR_TOP_Y, SPRINGBAR_BAR_THICKNESS);
    glPopMatrix();
    disableTextureIfNeeded();
}
//TODO HEREHEREHEREHEREHEREHEREHEREHEREHEREHEREHEREHEREHEREHEREHEREHEREHEREHEREHEREHEREHEREHEREHEREHEREHEREHEREHEREHEREHEREHEREHEREHEREHEREHEREHEREHEREHERE
//TODO HEREHEREHEREHEREHEREHEREHEREHEREHEREHEREHEREHEREHEREHEREHEREHEREHEREHEREHEREHEREHEREHEREHEREHEREHEREHEREHEREHEREHEREHEREHEREHEREHEREHEREHEREHEREHERE
//TODO HEREHEREHEREHEREHEREHEREHEREHEREHEREHEREHEREHEREHEREHEREHEREHEREHEREHEREHEREHEREHEREHEREHEREHEREHEREHEREHEREHEREHEREHEREHEREHEREHEREHEREHEREHEREHERE
//TODO HEREHEREHEREHEREHEREHEREHEREHEREHEREHEREHEREHEREHEREHEREHEREHEREHEREHEREHEREHEREHEREHEREHEREHEREHEREHEREHEREHEREHEREHEREHEREHEREHEREHEREHEREHEREHERE
//TODO HEREHEREHEREHEREHEREHEREHEREHEREHEREHEREHEREHEREHEREHEREHEREHEREHEREHEREHEREHEREHEREHEREHEREHEREHEREHEREHEREHEREHEREHEREHEREHEREHEREHEREHEREHEREHERE
//TODO HEREHEREHEREHEREHEREHEREHEREHEREHEREHEREHEREHEREHEREHEREHEREHEREHEREHEREHEREHEREHEREHEREHEREHEREHEREHEREHEREHEREHEREHEREHEREHEREHEREHEREHEREHEREHERE
//------------------- Draw a Single Spring ---------------------------
void drawSpring(float x, float z, float timeOffset, float maxHeight, float minHeight) {
    float currentHeight = (minHeight + (maxHeight - minHeight) * (0.5f + 0.5f * sin(beltOffset * 3.0f + timeOffset))) * 2;
    bindTextureIfNeeded(metalTex);
    glPushMatrix();
        glTranslatef(x, 0.0f, z);
        GLUquadric* quad = gluNewQuadric();
        gluQuadricTexture(quad, GL_TRUE);
        gluQuadricNormals(quad, GLU_SMOOTH);
        setCustomColor(0.5f, 0.5f, 0.5f);
        glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
        gluDisk(quad, 0.0f, SPRING_BASE_OUTER_RADIUS, 16, 1);
        gluCylinder(quad, SPRING_BASE_OUTER_RADIUS, SPRING_BASE_OUTER_RADIUS, SPRING_BASE_HEIGHT, 16, 4);
        glTranslatef(0.0f, 0.0f, SPRING_BASE_HEIGHT);
        gluDisk(quad, 0.0f, SPRING_BASE_OUTER_RADIUS, 16, 1);
        glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
        setCustomColor(0.7f, 0.7f, 0.7f);
        glBegin(GL_TRIANGLE_STRIP);
        for (int i = 0; i <= 360 * (int)SPRING_NUM_COILS; i += 5) {
            float angle = i * M_PI / 180.0f;
            float y = SPRING_BASE_OFFSET + (i / (360.0f * SPRING_NUM_COILS)) * currentHeight;
            float xC = SPRING_RADIUS * cos(angle);
            float zC = SPRING_RADIUS * sin(angle);
            float normalX = xC / SPRING_RADIUS;
            float normalZ = zC / SPRING_RADIUS;
            glTexCoord2f((float)i / (360.0f * SPRING_NUM_COILS), 0.0f);
            glVertex3f(xC - normalX * SPRING_THICKNESS, y, zC - normalZ * SPRING_THICKNESS);
            glTexCoord2f((float)i / (360.0f * SPRING_NUM_COILS), 1.0f);
            glVertex3f(xC + normalX * SPRING_THICKNESS, y, zC + normalZ * SPRING_THICKNESS);
        }
        glEnd();
        glTranslatef(0.0f, currentHeight + SPRING_BASE_OFFSET, 0.0f);
        setCustomColor(0.5f, 0.5f, 0.5f);
        glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
        gluDisk(quad, 0.0f, SPRING_CAP_SIZE, 16, 1);
        float capHeight = 0.6f;
        gluCylinder(quad, SPRING_CAP_SIZE, SPRING_CAP_SIZE, capHeight, 16, 4);
        glTranslatef(0.0f, 0.0f, capHeight);
        gluDisk(quad, 0.0f, SPRING_CAP_SIZE, 16, 1);
        gluDeleteQuadric(quad);
    glPopMatrix();
    disableTextureIfNeeded();
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
    bindTextureIfNeeded(beltTex);
    setCustomColor(0.9f, 0.9f, 0.9f);
    glBegin(GL_QUADS);
        glTexCoord2f(0.0f - normalizedOffset, 0.0f); glVertex3f(xLeft, yBase + ROLLER_RADIUS, BELT_Z_MIN);
        glTexCoord2f(0.0f - normalizedOffset, 1.0f); glVertex3f(xLeft, yBase + ROLLER_RADIUS, BELT_Z_MAX);
        glTexCoord2f(8.0f - normalizedOffset, 1.0f); glVertex3f(xRight, yBase + ROLLER_RADIUS, BELT_Z_MAX);
        glTexCoord2f(8.0f - normalizedOffset, 0.0f); glVertex3f(xRight, yBase + ROLLER_RADIUS, BELT_Z_MIN);
    glEnd();
    glBegin(GL_QUADS);
        glTexCoord2f(0.0f + normalizedOffset, 0.0f); glVertex3f(xLeft, yBase - ROLLER_RADIUS, BELT_Z_MIN);
        glTexCoord2f(8.0f + normalizedOffset, 0.0f); glVertex3f(xRight, yBase - ROLLER_RADIUS, BELT_Z_MIN);
        glTexCoord2f(8.0f + normalizedOffset, 1.0f); glVertex3f(xRight, yBase - ROLLER_RADIUS, BELT_Z_MAX);
        glTexCoord2f(0.0f + normalizedOffset, 1.0f); glVertex3f(xLeft, yBase - ROLLER_RADIUS, BELT_Z_MAX);
    glEnd();
    glBegin(GL_QUADS);
        glTexCoord2f(0.0f, 0.0f); glVertex3f(xLeft, yBase - ROLLER_RADIUS, BELT_Z_MIN);
        glTexCoord2f(0.0f, 0.1f); glVertex3f(xLeft, yBase - ROLLER_RADIUS, BELT_Z_MAX);
        glTexCoord2f(0.2f, 0.1f); glVertex3f(xLeft, yBase + ROLLER_RADIUS, BELT_Z_MAX);
        glTexCoord2f(0.2f, 0.0f); glVertex3f(xLeft, yBase + ROLLER_RADIUS, BELT_Z_MIN);
    glEnd();
    glBegin(GL_QUADS);
        glTexCoord2f(0.0f, 0.0f); glVertex3f(xRight, yBase - ROLLER_RADIUS, BELT_Z_MIN);
        glTexCoord2f(0.2f, 0.0f); glVertex3f(xRight, yBase + ROLLER_RADIUS, BELT_Z_MIN);
        glTexCoord2f(0.2f, 0.1f); glVertex3f(xRight, yBase + ROLLER_RADIUS, BELT_Z_MAX);
        glTexCoord2f(0.0f, 0.1f); glVertex3f(xRight, yBase - ROLLER_RADIUS, BELT_Z_MAX);
    glEnd();
    disableTextureIfNeeded();
}

void drawProcessedItem(float offset) {
    glPushMatrix();
        float worldX = -20.0f + offset;
        glTranslatef(worldX, 2.5f + ROLLER_RADIUS, (BELT_Z_MIN + BELT_Z_MAX) / 2.0f);
        if (worldX < 0.0f) {
            float progress = (worldX + 20.0f) / 20.0f;
            float scaleFactor = 1.0f - 0.25f * progress;
            glScalef(scaleFactor, scaleFactor, scaleFactor);
            float r = 1.0f;
            float g = 0.1f + (0.5f * progress);
            float b = 0.0f;
            setCustomColor(r, g, b);
            float ingotWidth = 1.0f;
            float ingotHeight = 0.5f;
            float ingotDepth = 0.7f;
            if (!isShadowPass) {
                float emissiveStrength = 1.0f - (0.8f * progress);
                GLfloat emissiveColor[] = {r * emissiveStrength, g * emissiveStrength, 0.0f, 1.0f};
                glMaterialfv(GL_FRONT, GL_EMISSION, emissiveColor);
            }
            glBegin(GL_QUADS);
                glNormal3f(0.0f, -1.0f, 0.0f);
                glVertex3f(-ingotWidth/2, -ingotHeight/2, -ingotDepth/2);
                glVertex3f(ingotWidth/2, -ingotHeight/2, -ingotDepth/2);
                glVertex3f(ingotWidth/2, -ingotHeight/2, ingotDepth/2);
                glVertex3f(-ingotWidth/2, -ingotHeight/2, ingotDepth/2);

                // Top face
                glNormal3f(0.0f, 1.0f, 0.0f);
                glVertex3f(-ingotWidth/2.2, ingotHeight/2, -ingotDepth/2.2);
                glVertex3f(-ingotWidth/2.2, ingotHeight/2, ingotDepth/2.2);
                glVertex3f(ingotWidth/2.2, ingotHeight/2, ingotDepth/2.2);
                glVertex3f(ingotWidth/2.2, ingotHeight/2, -ingotDepth/2.2);

                // Front face
                glNormal3f(0.0f, 0.0f, 1.0f);
                glVertex3f(-ingotWidth/2, -ingotHeight/2, ingotDepth/2);
                glVertex3f(ingotWidth/2, -ingotHeight/2, ingotDepth/2);
                glVertex3f(ingotWidth/2.2, ingotHeight/2, ingotDepth/2.2);
                glVertex3f(-ingotWidth/2.2, ingotHeight/2, ingotDepth/2.2);

                // Back face
                glNormal3f(0.0f, 0.0f, -1.0f);
                glVertex3f(-ingotWidth/2, -ingotHeight/2, -ingotDepth/2);
                glVertex3f(-ingotWidth/2.2, ingotHeight/2, -ingotDepth/2.2);
                glVertex3f(ingotWidth/2.2, ingotHeight/2, -ingotDepth/2.2);
                glVertex3f(ingotWidth/2, -ingotHeight/2, -ingotDepth/2);

                // Left face
                glNormal3f(-1.0f, 0.0f, 0.0f);
                glVertex3f(-ingotWidth/2, -ingotHeight/2, -ingotDepth/2);
                glVertex3f(-ingotWidth/2, -ingotHeight/2, ingotDepth/2);
                glVertex3f(-ingotWidth/2.2, ingotHeight/2, ingotDepth/2.2);
                glVertex3f(-ingotWidth/2.2, ingotHeight/2, -ingotDepth/2.2);

                // Right face
                glNormal3f(1.0f, 0.0f, 0.0f);
                glVertex3f(ingotWidth/2, -ingotHeight/2, -ingotDepth/2);
                glVertex3f(ingotWidth/2.2, ingotHeight/2, -ingotDepth/2.2);
                glVertex3f(ingotWidth/2.2, ingotHeight/2, ingotDepth/2.2);
                glVertex3f(ingotWidth/2, -ingotHeight/2, ingotDepth/2);
            glEnd();

            // Reset emission if we enabled it
            if (!isShadowPass) {
                GLfloat noEmission[] = {0.0f, 0.0f, 0.0f, 1.0f};
                glMaterialfv(GL_FRONT, GL_EMISSION, noEmission);
            }
        } else {
            glTranslatef(0.0f, -ROLLER_RADIUS, 0.0f);
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
            glBegin(GL_QUAD_STRIP);
                for (int i = 0; i <= segments; i++) {
                    float theta = 2.0f * M_PI * i / segments;
                    float xBottom = radius * cos(theta);
                    float zBottom = radius * sin(theta);
                    float twistedTheta = theta + twist * M_PI / 180.0f;
                    float xTop = radius * cos(twistedTheta);
                    float zTop = radius * sin(twistedTheta);
                    glNormal3f(cos(theta), 0.0f, sin(theta));
                    glVertex3f(xBottom, 0.0f, zBottom);
                    glNormal3f(cos(twistedTheta), 0.0f, sin(twistedTheta));
                    glVertex3f(xTop, cylinderHeight, zTop);
                }
            glEnd();
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
    float zBack = BELT_Z_MIN - 0.5f, zFront = BELT_Z_MAX + 0.5f;
    float yBottom = 0.0f, yTop = 2.1f - ROLLER_RADIUS;
    float legWidth = 0.3f;
    bindTextureIfNeeded(metalPlateTex);
    setCustomColor(1.f, 1.f, 1.f);
    glPushMatrix();
        glTranslatef(xLeft + legWidth/2, yBottom + (yTop-yBottom)/2, zFront - legWidth/2);
        drawTexturedCube(legWidth, yTop-yBottom, legWidth);
    glPopMatrix();
    glPushMatrix();
        glTranslatef(xLeft + legWidth/2, yBottom + (yTop-yBottom)/2, zBack + legWidth/2);
        drawTexturedCube(legWidth, yTop-yBottom, legWidth);
    glPopMatrix();
    glPushMatrix();
        glTranslatef(xRight - legWidth/2, yBottom + (yTop-yBottom)/2, zFront - legWidth/2);
        drawTexturedCube(legWidth, yTop-yBottom, legWidth);
    glPopMatrix();
    glPushMatrix();
        glTranslatef(xRight - legWidth/2, yBottom + (yTop-yBottom)/2, zBack + legWidth/2);
        drawTexturedCube(legWidth, yTop-yBottom, legWidth);
    glPopMatrix();
    int numSupports = 3;
    float supportSpacing = (xRight - xLeft) / (numSupports + 1);
    for (int i = 1; i <= numSupports; i++) {
        float xPos = xLeft + i * supportSpacing;
        glPushMatrix();
            glTranslatef(xPos, yBottom + (yTop-yBottom)/2, zFront - legWidth/2);
            drawTexturedCube(legWidth, yTop-yBottom, legWidth);
        glPopMatrix();
        glPushMatrix();
            glTranslatef(xPos, yBottom + (yTop-yBottom)/2, zBack + legWidth/2);
            drawTexturedCube(legWidth, yTop-yBottom, legWidth);
        glPopMatrix();
    }
    glPushMatrix();
        glTranslatef((xLeft + xRight)/2, yTop, zFront - legWidth/2);
        drawTexturedCube(xRight - xLeft, legWidth/2, legWidth);
    glPopMatrix();
    glPushMatrix();
        glTranslatef((xLeft + xRight)/2, yTop, zBack + legWidth/2);
        drawTexturedCube(xRight - xLeft, legWidth/2, legWidth);
    glPopMatrix();
    glPushMatrix();
        glTranslatef((xLeft + xRight)/2, yBottom + legWidth/2, zFront - legWidth/2);
        drawTexturedCube(xRight - xLeft, legWidth, legWidth);
    glPopMatrix();
    glPushMatrix();
        glTranslatef((xLeft + xRight)/2, yBottom + legWidth/2, zBack + legWidth/2);
        drawTexturedCube(xRight - xLeft, legWidth, legWidth);
    glPopMatrix();
    glPushMatrix();
        glTranslatef(xLeft + legWidth/2, yTop, (zFront + zBack)/2);
        drawTexturedCube(legWidth, legWidth/2, zFront - zBack);
    glPopMatrix();
    glPushMatrix();
        glTranslatef(xRight - legWidth/2, yTop, (zFront + zBack)/2);
        drawTexturedCube(legWidth, legWidth/2, zFront - zBack);
    glPopMatrix();
    setCustomColor(0.4f, 0.4f, 0.4f);
    glPushMatrix();
        glTranslatef(xLeft + 1.0f, yBottom + (yTop-yBottom)/2, zFront - legWidth/2);
        glRotatef(30.0f, 0.0f, 0.0f, 1.0f);
        drawTexturedCube(sqrt(pow(yTop-yBottom, 2) + pow(2.0f, 2)), legWidth/3, legWidth/2);
    glPopMatrix();
    glPushMatrix();
        glTranslatef(xLeft + 1.0f, yBottom + (yTop-yBottom)/2, zBack + legWidth/2);
        glRotatef(30.0f, 0.0f, 0.0f, 1.0f);
        drawTexturedCube(sqrt(pow(yTop-yBottom, 2) + pow(2.0f, 2)), legWidth/3, legWidth/2);
    glPopMatrix();
    glPushMatrix();
        glTranslatef(xRight - 1.0f, yBottom + (yTop-yBottom)/2, zFront - legWidth/2);
        glRotatef(-30.0f, 0.0f, 0.0f, 1.0f);
        drawTexturedCube(sqrt(pow(yTop-yBottom, 2) + pow(2.0f, 2)), legWidth/3, legWidth/2);
    glPopMatrix();
    glPushMatrix();
        glTranslatef(xRight - 1.0f, yBottom + (yTop-yBottom)/2, zBack + legWidth/2);
        glRotatef(-30.0f, 0.0f, 0.0f, 1.0f);
        drawTexturedCube(sqrt(pow(yTop-yBottom, 2) + pow(2.0f, 2)), legWidth/3, legWidth/2);
    glPopMatrix();
    disableTextureIfNeeded();
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
    bindTextureIfNeeded(metalTex);
    setCustomColor(0.6f, 0.6f, 0.6f);
    glPushMatrix();
        glTranslatef(baseX, baseY + baseHeight/2, baseZ);
        drawTexturedCube(baseWidth, baseHeight, baseDepth);
    glPopMatrix();
    glPushMatrix();
        glTranslatef(baseX, baseY + baseHeight, baseZ + armLength/2);
        drawTexturedCube(baseWidth/1.5f, baseWidth/1.5f, armLength);
    glPopMatrix();
    glPushMatrix();
        glTranslatef(baseX, baseY + baseHeight - baseWidth/3, BELT_Z_MIN + BELT_WIDTH/2);
        drawTexturedCube(baseWidth/1.5f, baseWidth*1.5f, baseWidth/1.5f);
    glPopMatrix();
    glPushMatrix();
        glTranslatef(baseX, baseY + baseHeight - baseWidth - pressPosition, BELT_Z_MIN + BELT_WIDTH/2);
        {
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
        }
    glPopMatrix();
    disableTextureIfNeeded();
    glPushMatrix();
        glTranslatef(baseX + baseWidth/2 + 0.1f, baseY + baseHeight*0.8f, baseZ);
        if (pressPosition < 0)
            setCustomColor(1.0f, 0.2f, 0.2f);
        else
            setCustomColor(0.2f, 1.0f, 0.2f);
        glutSolidSphere(0.2f, 16, 16);
    glPopMatrix();
}

//------------------- Draw Silo ---------------------------
void drawSilo(float x, float z) {
    const float siloHeight = 20.0f;
    const float siloRadius = 4.5f;
    const float roofHeight = 3.0f;
    const int segments = 32;
    bindTextureIfNeeded(metalPlateTex);
    setCustomColor(0.85f, 0.85f, 0.85f);
    glPushMatrix();
        glTranslatef(x, 0.0f, z);
        GLUquadric* quad = gluNewQuadric();
        gluQuadricTexture(quad, GL_TRUE);
        gluQuadricNormals(quad, GLU_SMOOTH);
        glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
        gluCylinder(quad, siloRadius, siloRadius, siloHeight, segments, 8);
        gluDisk(quad, 0.0f, siloRadius, segments, 1);
        glTranslatef(0.0f, 0.0f, siloHeight);
        gluCylinder(quad, siloRadius, 0.0f, roofHeight, segments, 8);
        gluDeleteQuadric(quad);
    glPopMatrix();
    disableTextureIfNeeded();
    setCustomColor(0.6f, 0.6f, 0.6f);
    glPushMatrix();
        bindTextureIfNeeded(metalPlateTex);
        glTranslatef(x, siloHeight + roofHeight - 0.5f, z);
        glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
        {
            GLUquadric* pipeQuad = gluNewQuadric();
            gluQuadricTexture(pipeQuad, GL_TRUE);
            gluQuadricNormals(pipeQuad, GLU_SMOOTH);
            gluCylinder(pipeQuad, 0.3f, 0.3f, 1.0f, 16, 2);
            gluDeleteQuadric(pipeQuad);
        }
        disableTextureIfNeeded();
    glPopMatrix();
    setCustomColor(0.65f, 0.65f, 0.65f);
    glPushMatrix();
        glTranslatef(x, 0.1f, z);
        drawTexturedCube(siloRadius * 2.2f, 0.2f, siloRadius * 2.2f);
    glPopMatrix();
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

//------------------- Draw Upgrader ---------------------------
void drawUpgrader(float upgraderX) {
    float baseZ = BELT_Z_MIN - 1.0f;
    float baseY = 0.0f;
    float baseWidth = 1.0f;
    float baseHeight = 4.7f;
    float armWidth = 1.0f;
    bindTextureIfNeeded(floorTex);
    setCustomColor(0.7f, 0.7f, 0.7f);
    glPushMatrix();
        glTranslatef(upgraderX, baseY + baseHeight/2, baseZ);
        drawTexturedCube(baseWidth, baseHeight, baseWidth);
    glPopMatrix();
    glPushMatrix();
        glTranslatef(upgraderX, baseY + baseHeight, (baseZ + (BELT_Z_MIN + BELT_Z_MAX)/2) / 2);
        drawTexturedCube(armWidth, armWidth, (BELT_Z_MAX - BELT_Z_MIN) + 2.0f);
    glPopMatrix();
    disableTextureIfNeeded();
}

//------------------- Draw Upgrader Beam ---------------------------
void drawUpgraderBeam(float upgraderX, GLfloat a, GLfloat b, GLfloat c, float offsetAmount) {
    float baseY = 0.0f;
    float baseHeight = 4.7f;
    float beamHeight = 3.5f;
    float beamWidth = 0.5f;
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_LIGHTING);
    float pulseIntensity = 0.6f + 0.4f * sin(beltOffset * 5.0f + offsetAmount);
    glColor4f(a, b, c, 0.4f * pulseIntensity);
    glPushMatrix();
        glTranslatef(upgraderX, baseY + baseHeight/2 + beamHeight/2, (BELT_Z_MIN + BELT_Z_MAX)/2);
        drawTexturedCube(beamWidth, beamHeight, BELT_Z_MAX - BELT_Z_MIN + 0.2f);
    glPopMatrix();
    glColor4f(0.4f, 0.9f, 1.0f, 0.7f * pulseIntensity);
    glPushMatrix();
        glTranslatef(upgraderX, baseY + baseHeight/2 + beamHeight/2, (BELT_Z_MIN + BELT_Z_MAX)/2);
        drawTexturedCube(beamWidth * 0.5f, beamHeight * 0.7f, (BELT_Z_MAX - BELT_Z_MIN) * 0.8f);
    glPopMatrix();
    glEnable(GL_LIGHTING);
    glDisable(GL_BLEND);
}

//------------------- Draw Fire Blaster ---------------------------
void drawFireBlaster() {
    float craneHeight = 10.0f;
    float armWidth = 2.f;
    float baseWidth = 1.0f;

    // Draw the left crane arm
    bindTextureIfNeeded(metalTex);
    setCustomColor(0.7f, 0.7f, 0.7f);
    glPushMatrix();
    glTranslatef(-10.f, craneHeight/2, 0.f);
    drawTexturedCube(armWidth, craneHeight, armWidth);
    glPopMatrix();

    // Draw the right crane arm
    glPushMatrix();
    glTranslatef(-10.f, craneHeight/2, -8.f);
    drawTexturedCube(armWidth, craneHeight, armWidth);
    glPopMatrix();

    // Draw the connecting beam across the top
    glPushMatrix();
    glTranslatef(FIREBLASTER_X, craneHeight, (BELT_Z_MIN + BELT_Z_MAX)/2);
    glRotatef(-90.0f, 0.0f, 1.0f, 0.0f);
    drawTexturedCube(FIREBLASTER_MAX_X - FIREBLASTER_MIN_X, armWidth, armWidth);
    glPopMatrix();

    // Draw the central fire blaster unit
    glPushMatrix();
    glTranslatef(FIREBLASTER_X, craneHeight - 1.0f, (BELT_Z_MIN + BELT_Z_MAX)/2);
    drawTexturedCube(FIREBLASTER_MAX_X - FIREBLASTER_MIN_X, 1.5f, baseWidth);
    glPopMatrix();
    disableTextureIfNeeded();
}


//------------------- Draws Kiln ---------------------------
void drawKiln() {
    float kilnBaseX = -20.f;
    float kilnBaseY = 0.f;
    float kilnBaseZ = -4.f;
    const float kilnWidth  = 4.0f;
    const float kilnHeight = 6.0f;
    const float kilnDepth  = 4.0f;
    const float doorWidth  = 2.0f;
    const float doorHeight = 5.5f;
    float angle = 90.f;
    bindTextureIfNeeded(brickTex);
    setCustomColor(0.8f, 0.3f, 0.3f);
    glPushMatrix();
        glTranslatef(kilnBaseX, kilnBaseY + kilnHeight / 2.0f, kilnBaseZ);
        glRotatef(angle, 0.0f, 1.0f, 0.0f);
        drawTexturedCube(kilnWidth, kilnHeight, kilnDepth);
    glPopMatrix();
    glPushMatrix();
        glTranslatef(kilnBaseX, kilnBaseY, kilnBaseZ);
        glRotatef(angle, 0.0f, 1.0f, 0.0f);
        glTranslatef(0.0f, kilnHeight / 4.0f, kilnDepth / 2.0f + 0.01f);
        setCustomColor(0.2f, 0.2f, 0.2f);
        glScalef(doorWidth, doorHeight, 0.1f);
        glutSolidCube(1.0f);
    glPopMatrix();
    disableTextureIfNeeded();
}

//------------------- Draws Packer ---------------------------
void drawPacker() {
    float packerBaseX = 24.f;
    float packerBaseY = 0.f;
    float packerBaseZ = -4.f;
    const float packerWidth  = 8.0f;
    const float packerHeight = 6.0f;
    const float packerDepth  = 9.0f;
    const float doorWidth  = 2.0f;
    const float doorHeight = 5.5f;
    float angle = -90.f;
    bindTextureIfNeeded(metalWallTex);
    setCustomColor(0.2f, 0.8f, 0.5f);
    glPushMatrix();
        glTranslatef(packerBaseX, packerBaseY + packerHeight / 2.0f, packerBaseZ);
        glRotatef(angle, 0.0f, 1.0f, 0.0f);
        drawTexturedCube(packerWidth, packerHeight, packerDepth);
    glPopMatrix();
    glPushMatrix();
        glTranslatef(packerBaseX, packerBaseY, packerBaseZ);
        glRotatef(angle, 0.0f, 1.0f, 0.0f);
        glTranslatef(0.0f, packerHeight / 4.0f, packerDepth / 2.0f + 0.01f);
        setCustomColor(0.2f, 0.2f, 0.2f);
        glScalef(doorWidth, doorHeight, 0.1f);
        glutSolidCube(1.0f);
    glPopMatrix();
    disableTextureIfNeeded();
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
    glGenTextures(1, &metalPlateTex);
    glBindTexture(GL_TEXTURE_2D, metalPlateTex);
    loadTGA("metalPlate.tga");
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glGenTextures(1, &brickTex);
    glBindTexture(GL_TEXTURE_2D, brickTex);
    loadTGA("brick.tga");
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glGenTextures(1, &metalWallTex);
    glBindTexture(GL_TEXTURE_2D, metalWallTex);
    loadTGA("metalWall.tga");
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glGenTextures(6, skyboxTex);
    glBindTexture(GL_TEXTURE_2D, skyboxTex[0]);
    loadTGA("negx.tga");
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, skyboxTex[1]);
    loadTGA("negy.tga");
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, skyboxTex[2]);
    loadTGA("negz.tga");
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, skyboxTex[3]);
    loadTGA("posx.tga");
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, skyboxTex[4]);
    loadTGA("posy.tga");
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, skyboxTex[5]);
    loadTGA("posz.tga");
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

//------------------- Draw the Skybox ---------------------------
void drawSkybox() {
    float size = 500.0f;
    float halfSize = size / 2.0f;
    glEnable(GL_TEXTURE_2D);
    bindTextureIfNeeded(skyboxTex[0]);
    glBegin(GL_QUADS);
        glTexCoord2f(0.0f, 1.0f); glVertex3f(-halfSize, -halfSize, -halfSize);
        glTexCoord2f(1.0f, 1.0f); glVertex3f(-halfSize, -halfSize, halfSize);
        glTexCoord2f(1.0f, 0.0f); glVertex3f(-halfSize, halfSize, halfSize);
        glTexCoord2f(0.0f, 0.0f); glVertex3f(-halfSize, halfSize, -halfSize);
    glEnd();
    disableTextureIfNeeded();
    bindTextureIfNeeded(skyboxTex[1]);
    glBegin(GL_QUADS);
        glTexCoord2f(0.0f, 1.0f); glVertex3f(-halfSize, -halfSize, halfSize);
        glTexCoord2f(1.0f, 1.0f); glVertex3f(halfSize, -halfSize, halfSize);
        glTexCoord2f(1.0f, 0.0f); glVertex3f(halfSize, -halfSize, -halfSize);
        glTexCoord2f(0.0f, 0.0f); glVertex3f(-halfSize, -halfSize, -halfSize);
    glEnd();
    disableTextureIfNeeded();
    bindTextureIfNeeded(skyboxTex[2]);
    glBegin(GL_QUADS);
        glTexCoord2f(0.0f, 1.0f); glVertex3f(halfSize, -halfSize, -halfSize);
        glTexCoord2f(1.0f, 1.0f); glVertex3f(-halfSize, -halfSize, -halfSize);
        glTexCoord2f(1.0f, 0.0f); glVertex3f(-halfSize, halfSize, -halfSize);
        glTexCoord2f(0.0f, 0.0f); glVertex3f(halfSize, halfSize, -halfSize);
    glEnd();
    disableTextureIfNeeded();
    bindTextureIfNeeded(skyboxTex[3]);
    glBegin(GL_QUADS);
        glTexCoord2f(1.0f, 1.0f); glVertex3f(halfSize, -halfSize, -halfSize);
        glTexCoord2f(0.0f, 1.0f); glVertex3f(halfSize, -halfSize, halfSize);
        glTexCoord2f(0.0f, 0.0f); glVertex3f(halfSize, halfSize, halfSize);
        glTexCoord2f(1.0f, 0.0f); glVertex3f(halfSize, halfSize, -halfSize);
    glEnd();
    disableTextureIfNeeded();
    bindTextureIfNeeded(skyboxTex[4]);
    glBegin(GL_QUADS);
        glTexCoord2f(0.0f, 0.0f); glVertex3f(-halfSize, halfSize, -halfSize);
        glTexCoord2f(0.0f, 1.0f); glVertex3f(-halfSize, halfSize, halfSize);
        glTexCoord2f(1.0f, 1.0f); glVertex3f(halfSize, halfSize, halfSize);
        glTexCoord2f(1.0f, 0.0f); glVertex3f(halfSize, halfSize, -halfSize);
    glEnd();
    disableTextureIfNeeded();
    bindTextureIfNeeded(skyboxTex[5]);
    glBegin(GL_QUADS);
        glTexCoord2f(1.0f, 1.0f); glVertex3f(halfSize, -halfSize, halfSize);
        glTexCoord2f(0.0f, 1.0f); glVertex3f(-halfSize, -halfSize, halfSize);
        glTexCoord2f(0.0f, 0.0f); glVertex3f(-halfSize, halfSize, halfSize);
        glTexCoord2f(1.0f, 0.0f); glVertex3f(halfSize, halfSize, halfSize);
    glEnd();
    disableTextureIfNeeded();
}

//------------------- Draw Textured Floor ---------------------------
void drawTexturedFloor() {
    bindTextureIfNeeded(floorTex);
    setCustomColor(1.0f, 1.0f, 1.0f);
    glBegin(GL_QUADS);
        glTexCoord2f(0.0f, 0.0f); glVertex3f(-50.0f, 0.0f, -50.0f);
        glTexCoord2f(0.0f, 1.0f); glVertex3f(-50.0f, 0.0f,  50.0f);
        glTexCoord2f(1.0f, 1.0f); glVertex3f( 50.0f, 0.0f,  50.0f);
        glTexCoord2f(1.0f, 0.0f); glVertex3f( 50.0f, 0.0f, -50.0f);
    glEnd();
    disableTextureIfNeeded();
}

//------------------- Display Callback ---------------------------
void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    if (wireframeMode) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    } else {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
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
    glPushMatrix();
        float m[16];
        glGetFloatv(GL_MODELVIEW_MATRIX, m);
        m[12] = m[13] = m[14] = 0.0f;
        glLoadMatrixf(m);
        glDepthMask(GL_FALSE);
        drawSkybox();
        glDepthMask(GL_TRUE);
    glPopMatrix();
    if (!wireframeMode) {
        glLightfv(GL_LIGHT0, GL_POSITION, GLOBAL_LIGHT_POSITION);
    }
    glDisable(GL_LIGHTING);
    drawTexturedFloor();
    if (!wireframeMode) {
        glEnable(GL_LIGHTING);
    }
    drawSupportStructure();
    drawRollers();
    drawPressDevice();
    drawBackgroundSprings();
    drawConveyorBelt();
    drawSilos();
    drawKiln();
    drawPacker();
    float spacing = BELT_X_LENGTH / NUM_ITEMS;
    for (int i = 0; i < NUM_ITEMS; i++) {
        float itemOffset = fmod(beltOffset + i * spacing, BELT_X_LENGTH);
        drawProcessedItem(itemOffset);
    }
    drawFireBlaster();
    drawParticles();
    drawUpgrader(3.0f);
    drawUpgraderBeam(3.0f, 0.2, 0.8, 1.0, 0);
    drawUpgrader(4.0f);
    drawUpgraderBeam(4.0f, 0.8, 0.8, 0.2, 0.9);
    drawUpgrader(5.0f);
    drawUpgraderBeam(5.0f, 0.8, 0.2, 0.8, -1.0);
    GLfloat groundPlane[4] = {0.0f, 1.0f, 0.0f, 0.0f};
    GLfloat shadowMat[4][4];
    computeShadowMatrix(shadowMat, groundPlane, GLOBAL_LIGHT_POSITION);
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
        drawKiln();
        drawPacker();
        drawUpgrader(3.0f);
        drawUpgrader(4.0f);
        drawUpgrader(5.0f);
        for (int i = 0; i < NUM_ITEMS; i++) {
            float itemOffset = fmod(beltOffset + i * spacing, BELT_X_LENGTH);
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
    else if (key == 'q' || key == 'Q') {
        wireframeMode = !wireframeMode;
        glutPostRedisplay();
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
    glLightfv(GL_LIGHT0, GL_AMBIENT, GLOBAL_LIGHT_AMBIENT);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, GLOBAL_LIGHT_DIFFUSE);
    glLightfv(GL_LIGHT0, GL_SPECULAR, GLOBAL_LIGHT_SPECULAR);
    glMaterialfv(GL_FRONT, GL_AMBIENT, MATAMBIENT);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, MATDIFFUSE);
    glMaterialfv(GL_FRONT, GL_SPECULAR, MATSPECULAR);
    glMaterialfv(GL_FRONT, GL_SHININESS, MATSHININESS);
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
