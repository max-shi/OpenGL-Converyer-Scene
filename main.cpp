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

//-- Globals ---------------------------------------------------------------

// Textures
GLuint floorTex;
GLuint beltTex;
GLuint metalTex;

// Processed item globals
int numItems = 8;

// Conveyor belt globals (oriented along x-axis).
float beltOffset = 0.0f;          // Current offset along the belt (x-axis)
float beltSpeed = 1.f / 60.f;         // Speed of belt movement (units per update)
float beltXLength = 40.0f;        // Belt spans from x = -20 to x = +20
const float beltZMin = -5.0f;     // Belt z-start
const float beltZMax = -3.0f;     // Belt z-end
const float beltWidth = beltZMax - beltZMin;  // Width of the belt (z-axis)
const float beltThickness = 0.1f; // Thickness of the belt material
const float rollerRadius = 0.3f;  // Radius of the rollers
float rollerRotation = 0.0f;      // Current rotation angle of rollers (in degrees)

// Global instance of KeyboardUtilities.
KeyboardUtilities keyboardUtil;

//-- Particle System Globals ---------------------------------------------
const int MAX_PARTICLES = 500;
const float PARTICLE_LIFETIME = 1.5f;  // Maximum lifetime in seconds
const float SPARK_GRAVITY = -9.8f;     // Gravity acceleration (m/s²)
const float FLOOR_Y = 0.0f;            // Y position of the floor
const float BOUNCE_DAMPING = 0.6f;     // Energy loss on bounce

struct Particle {
    float x, y, z;           // Position
    float vx, vy, vz;        // Velocity
    float r, g, b;           // Color
    float lifetime;          // Remaining lifetime in seconds
    float maxLifetime;       // Original maximum lifetime
    float scale;             // Size of the particle
    bool active;             // Whether the particle is active
};

vector<Particle> particles(MAX_PARTICLES);
bool sparkGeneration = false;
float timeSinceLastEmission = 0.0f;
float emissionRate = 0.001f;  // Time between particle emissions in seconds

//------------------- Initialize Particle System ----------------------
void initParticleSystem() {
    for (int i = 0; i < MAX_PARTICLES; i++) {
        particles[i].active = false;
    }
}

//------------------- Create New Particle ----------------------------
void createParticle(float originX, float originY, float originZ) {
    // Find an inactive particle
    for (int i = 0; i < MAX_PARTICLES; i++) {
        if (!particles[i].active) {
            // Position at spark origin
            particles[i].x = originX;
            particles[i].y = originY;
            particles[i].z = originZ;

            // Random direction in xz-plane (horizontal)
            float angle = (float)(rand() % 360) * M_PI / 180.0f;
            float speed = 2.0f + (float)(rand() % 400) / 100.0f;  // 2.0 to 6.0

            particles[i].vx = speed * cos(angle);
            particles[i].vz = speed * sin(angle);
            particles[i].vy = 1.0f + (float)(rand() % 100) / 50.0f;  // Small upward velocity

            // Randomized color (orange/yellow/white for sparks)
            particles[i].r = 0.9f + (float)(rand() % 10) / 100.0f;
            particles[i].g = 0.5f + (float)(rand() % 50) / 100.0f;
            particles[i].b = (float)(rand() % 20) / 100.0f;

            // Randomized lifetime
            particles[i].maxLifetime = PARTICLE_LIFETIME * (0.5f + (float)(rand() % 50) / 100.0f);
            particles[i].lifetime = particles[i].maxLifetime;

            // Randomized scale
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
            // Update position based on velocity
            particles[i].x += particles[i].vx * deltaTime;
            particles[i].y += particles[i].vy * deltaTime;
            particles[i].z += particles[i].vz * deltaTime;

            // Apply gravity to vertical velocity
            particles[i].vy += SPARK_GRAVITY * deltaTime;

            // Check for collision with floor and bounce
            if (particles[i].y <= FLOOR_Y && particles[i].vy < 0) {
                particles[i].y = FLOOR_Y;
                particles[i].vy = -particles[i].vy * BOUNCE_DAMPING;

                // Also dampen horizontal velocity on bounce
                particles[i].vx *= 0.9f;
                particles[i].vz *= 0.9f;

                // If bounce is too small, stop bouncing
                if (fabs(particles[i].vy) < 0.5f) {
                    particles[i].vy = 0;
                }
            }

            // Update lifetime
            particles[i].lifetime -= deltaTime;

            // Deactivate if lifetime expired
            if (particles[i].lifetime <= 0) {
                particles[i].active = false;
            }
        }
    }

    // Emit new particles if transformation is happening
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
    // Disable lighting for particles (they'll be self-illuminated)
    glDisable(GL_LIGHTING);
    glDepthMask(GL_FALSE);  // Don't write to depth buffer (for transparency)
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);  // Additive blending for glow effect

    // Use point sprites for particles
    glEnable(GL_POINT_SMOOTH);
    glEnable(GL_POINT_SPRITE);

    glPointSize(10.0f);
    glBegin(GL_POINTS);

    for (int i = 0; i < MAX_PARTICLES; i++) {
        if (particles[i].active) {
            // Set color with alpha based on lifetime
            float lifeRatio = particles[i].lifetime / particles[i].maxLifetime;
            float alpha = lifeRatio;

            // Make particles fade from white to orange/red as they age
            float r = particles[i].r;
            float g = particles[i].g * lifeRatio;
            float b = particles[i].b * lifeRatio * 0.5f;

            glColor4f(r, g, b, alpha);
            glPointSize(particles[i].scale * 10.0f * lifeRatio);
            glVertex3f(particles[i].x, particles[i].y, particles[i].z);
        }
    }

    glEnd();

    // Reset state
    glDisable(GL_POINT_SPRITE);
    glDisable(GL_POINT_SMOOTH);
    glDisable(GL_BLEND);
    glDepthMask(GL_TRUE);
    glEnable(GL_LIGHTING);
}

void displayParticleCheck() {
    float spacing = beltXLength / numItems;
    sparkGeneration = false;  // Reset flag

    for (int i = 0; i < numItems; i++) {
        float itemOffset = fmod(beltOffset + i * spacing, beltXLength);
        float worldX = -20.0f + itemOffset;

        // If an item is crossing the transformation point (within a small range of x=0)
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

    // Update conveyor belt offset along the x-axis.
    beltOffset += beltSpeed;
    if (beltOffset > beltXLength)
        beltOffset = fmod(beltOffset, beltXLength); // Loop back.
    // Update roller rotation angle.
    // The relationship is: angular displacement = linear displacement / radius (in radians)
    // Converting to degrees: angle = beltSpeed * (180/(π*rollerRadius))
    rollerRotation += beltSpeed * 180.0f / (M_PI * rollerRadius);

    // Update camera movement
    keyboardUtil.update();
    displayParticleCheck();
    updateParticles(deltaTime);
    glutPostRedisplay();
    glutTimerFunc(16, updateScene, 0); // Roughly 60 FPS.
}

//------------------- Draw an Individual Roller ---------------------------
void drawRoller(float xPos) {
    float rollerLength = beltWidth;  // Roller length equals belt's width.
    float yCenter = 2.1f;           // Height at which rollers sit
    float zCenter = (beltZMin + beltZMax) / 2.0f;  // Center of the belt in z

    glPushMatrix();
    // Position the roller at the appropriate x, y, and z
    glTranslatef(xPos, yCenter, zCenter);

    // Shift so the cylinder extends from z=0 to z=rollerLength in local coords
    glTranslatef(0.0f, 0.0f, -rollerLength / 2.0f);

    // Apply rotation about the roller’s axis (z-axis) for spinning
    glRotatef(rollerRotation, 0.0f, 0.0f, -1.0f);

    // Bind the metal texture (or simply use a color) for the roller.
    glBindTexture(GL_TEXTURE_2D, metalTex);
    glEnable(GL_TEXTURE_2D);
    glColor3f(0.7f, 0.7f, 0.7f);

    // Create a quadric object for drawing the cylinder.
    GLUquadric* quad = gluNewQuadric();
    gluQuadricTexture(quad, GL_TRUE);
    gluQuadricNormals(quad, GLU_SMOOTH);

    // 1) Draw the cylindrical surface, from z=0 to z=rollerLength
    gluCylinder(quad, rollerRadius, rollerRadius, rollerLength, 32, 4);

    // 2) Draw the top disk (at z=rollerLength)
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
    float spacing = beltXLength / (numRollers - 1); // beltXLength is 40.0f (from -20 to 20)
    for (int i = 0; i < numRollers; i++) {
        float xPos = -20.0f + i * spacing;
        drawRoller(xPos);
    }
}
//------------------- Draw the Overhead Bar and Supports ---------------------------
void drawSpringBars(float zPosition)
{
    float xLocation = -5.0f;
    float topY    = 12.0f;
    float barLen  = 12.0f;
    float barThk  = 0.4f;
    float sidePos = 6.0f;

    // Bind metal texture
    glBindTexture(GL_TEXTURE_2D, metalTex);
    glEnable(GL_TEXTURE_2D);
    glColor3f(0.7f, 0.7f, 0.7f); // Light gray

    //-------------- Overhead bar --------------
    glPushMatrix();
        glTranslatef(xLocation, topY, zPosition);      // move up to topY
        glScalef(barLen, barThk, barThk);         // make the bar thicker/longer
        glutSolidCube(1.0f);
    glPopMatrix();

    //-------------- Left vertical support --------------
    glPushMatrix();
        glTranslatef(-sidePos + xLocation, topY / 2.0f, zPosition); // half of topY in height
        glScalef(barThk, topY, barThk);
        glutSolidCube(1.0f);
    glPopMatrix();

    //-------------- Right vertical support --------------
    glPushMatrix();
        glTranslatef(sidePos + xLocation, topY / 2.0f, zPosition);
        glScalef(barThk, topY, barThk);
        glutSolidCube(1.0f);
    glPopMatrix();

    glDisable(GL_TEXTURE_2D);
}

//------------------- Draw a Single Spring ---------------------------
void drawSpring(float x, float z, float timeOffset, float maxHeight, float minHeight)
{
    // Scale factor for the spring coils
    float springRadius    = 0.8f;
    float springThickness = 0.16f; // originally 0.08f
    float capSize         = 1.2f; // originally 0.6f

    float numCoils        = 10.0f;
    float coilBaseOffset  = 0.2f; // originally 0.1f
    float unscaledHeight = minHeight + (maxHeight - minHeight) *
                           (0.5f + 0.5f * sin(beltOffset * 3.0f + timeOffset));
    float currentHeight = unscaledHeight * 2.;

    // Bind metal texture for the spring
    glBindTexture(GL_TEXTURE_2D, metalTex);
    glEnable(GL_TEXTURE_2D);

    glPushMatrix();
        // Place the spring at (x, 0, z)
        glTranslatef(x, 0.0f, z);

        // Create a quadric for cylinders/disks
        GLUquadric* quad = gluNewQuadric();
        gluQuadricTexture(quad, GL_TRUE);
        gluQuadricNormals(quad, GLU_SMOOTH);

        //------------------ Base of the spring ------------------
        glColor3f(0.5f, 0.5f, 0.5f); // darker gray
        glRotatef(90.0f, 1.0f, 0.0f, 0.0f); // flip to draw disk on XZ-plane

        // Make the base disk and a short cylinder for the base (scaled up)
        float baseOuterRadius = springRadius + 0.1f;
        float baseHeight  = 0.1f;

        gluDisk(quad, 0.0f, baseOuterRadius, 16, 1);
        gluCylinder(quad, baseOuterRadius, baseOuterRadius, baseHeight, 16, 4);
        glTranslatef(0.0f, 0.0f, baseHeight);
        gluDisk(quad, 0.0f, baseOuterRadius, 16, 1);

        glRotatef(-90.0f, 1.0f, 0.0f, 0.0f); // rotate back for vertical coil

        //------------------ Helical spring coil ------------------
        glColor3f(0.7f, 0.7f, 0.7f); // lighter gray

        glBegin(GL_TRIANGLE_STRIP);
        for (int i = 0; i <= 360 * (int)numCoils; i += 5)
        {
            float angle = i * M_PI / 180.0f;
            float y = coilBaseOffset + (i / (360.0f * numCoils)) * currentHeight;
            float xC = springRadius * cos(angle);
            float zC = springRadius * sin(angle);

            // Normal for outward direction
            float normalX = xC / springRadius;
            float normalZ = zC / springRadius;

            // "Inner" vertex
            glNormal3f(normalX, 0.0f, normalZ);
            glTexCoord2f((float)i / (360.0f * numCoils), 0.0f);
            glVertex3f(xC - normalX * springThickness, y, zC - normalZ * springThickness);

            // "Outer" vertex
            glNormal3f(normalX, 0.0f, normalZ);
            glTexCoord2f((float)i / (360.0f * numCoils), 1.0f);
            glVertex3f(xC + normalX * springThickness, y, zC + normalZ * springThickness);
        }
        glEnd();

        //------------------ Top cap at the end of the coil ------------------
        glTranslatef(0.0f, currentHeight + coilBaseOffset, 0.0f);
        glColor3f(0.5f, 0.5f, 0.5f); // darker gray again
        glRotatef(90.0f, 1.0f, 0.0f, 0.0f);

        // Draw top disk
        gluDisk(quad, 0.0f, capSize, 16, 1);

        // A short cylinder (scaled up by 3) for the cap thickness
        float capHeight = 0.6f; // originally 0.3f
        gluCylinder(quad, capSize, capSize, capHeight, 16, 4);

        glTranslatef(0.0f, 0.0f, capHeight);
        gluDisk(quad, 0.0f, capSize, 16, 1);

        gluDeleteQuadric(quad);
    glPopMatrix();

    glDisable(GL_TEXTURE_2D);
}

//------------------- Draw All Background Springs ---------------------------
void drawBackgroundSprings()
{
    // Left spring: overhead bar at z = -20, coil at x = -5
    drawSpringBars(-20.0f);
    drawSpring(-7.5f, -20.0f, 0.0f, 6.f, 2.0f);
    drawSpring(-5.0f, -20.0f, 2.0f, 6.f, 1.5f);
    drawSpring(-2.5f, -20.0f, 4.0f, 6.f, 2.5f);
    drawSpring(0.0f, -20.0f, 6.0f, 6.f, 2.5f);
    drawSpring(-10.0f, -20.0f, -2.2f, 6.f, 2.5f);
}


//------------------- Draw Conveyor Belt ---------------------------
void drawConveyorBelt() {
    float xLeft = -20.0f, xRight = 20.0f;
    float yBase = 2.1f;
    float normalizedOffset = beltOffset / 7.5f;
    // Enable texture for belt
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, beltTex);

    // Draw the top moving surface of belt (with texture animation)
    glColor3f(0.9f, 0.9f, 0.9f); // Almost black for rubber belt
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
}

//------------------- Draw Processed Item ---------------------------
void drawProcessedItem(float offset) {
    glPushMatrix();
        // Compute world x coordinate.
        float worldX = -20.0f + offset;
        // Position item: at computed x, fixed y above the conveyor surface, and centered in z.
        glTranslatef(worldX, 2.5f + rollerRadius, (beltZMin + beltZMax) / 2.0f);

        if (worldX < 0.0f) {
            // Before the press (x < 0): Draw a blue cube
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
        } else if (worldX < 9.0f) {
            // After the press (0 <= x < 9): Draw a disk/cylinder that grows in height
            float heightScale = 0.1f + (worldX / 8.0f) * 0.9f;  // Height grows from 0.1 to 1.0
            float radiusScale = 0.75f;  // Keep the radius consistent

            glScalef(radiusScale, heightScale, radiusScale);

            // Create a quadric for the cylinder
            GLUquadric* quad = gluNewQuadric();
            gluQuadricNormals(quad, GLU_SMOOTH);

            // Rotate to align with y-axis (cylinder stands upright)
            glRotatef(90.0f, 1.0f, 0.0f, 0.0f);

            // Color transitions from blue to red as it grows
            float blueComponent = 1.0f - (worldX / 9.0f);
            float redComponent = worldX / 9.0f;
            glColor3f(redComponent, 0.2f, blueComponent);

            // Draw cylinder
            gluCylinder(quad, 1.0f, 1.0f, 1.0f, 24, 2);

            // Draw top disk
            glPushMatrix();
            glTranslatef(0.0f, 0.0f, 1.0f);
            gluDisk(quad, 0.0f, 1.0f, 24, 1);
            glPopMatrix();

            // Draw bottom disk
            glPushMatrix();
            gluDisk(quad, 0.0f, 1.0f, 24, 1);
            glPopMatrix();

            gluDeleteQuadric(quad);
        } else {
            // After the kiln (x >= 9): Draw a yellow teapot
            glScalef(0.75f, 0.75f, 0.75f);
            glColor3f(1.0f, 1.0f, 0.0f);
            glutSolidTeapot(1.0);
        }
    glPopMatrix();
}

//------------------- Draw Kiln Structure ---------------------------
void drawKiln() {
    // Kiln position
    float kilnX = 9.0f;
    float kilnY = 2.5f;
    float kilnZ = (beltZMin + beltZMax) / 2.0f;

    // Kiln dimensions
    float kilnWidth = 4.0f;
    float kilnHeight = 5.0f;
    float kilnDepth = beltWidth + 1.0f;

    // Use brick-like texture if available, otherwise use color
    glBindTexture(GL_TEXTURE_2D, metalTex);
    glEnable(GL_TEXTURE_2D);

    // Draw kiln structure (a simple arch over the conveyor)
    glColor3f(0.8f, 0.3f, 0.2f);  // Brick-red color

    // Left side wall
    glPushMatrix();
    glTranslatef(kilnX, kilnY + kilnHeight/2, beltZMin - kilnDepth/4);
    glScalef(kilnWidth/2, kilnHeight, kilnDepth/2);
    glutSolidCube(1.0f);
    glPopMatrix();

    // Right side wall
    glPushMatrix();
    glTranslatef(kilnX, kilnY + kilnHeight/2, beltZMax + kilnDepth/4);
    glScalef(kilnWidth/2, kilnHeight, kilnDepth/2);
    glutSolidCube(1.0f);
    glPopMatrix();

    // Top arch
    glPushMatrix();
    glTranslatef(kilnX, kilnY + kilnHeight, kilnZ);
    glScalef(kilnWidth/2, kilnHeight/5, kilnDepth);
    glutSolidCube(1.0f);
    glPopMatrix();

    // Draw glowing interior (suggesting heat)
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_LIGHTING);

    // Interior glow (pulsating orange-red)
    float pulseIntensity = 0.7f + 0.3f * sin(beltOffset * 10.0f);
    glColor4f(1.0f, 0.3f * pulseIntensity, 0.0f, 0.8f);

    // Draw the glowing interior as a slightly smaller cube
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);

    glPushMatrix();
    glTranslatef(kilnX, kilnY + kilnHeight/2, kilnZ);
    glScalef(kilnWidth/3, kilnHeight*0.8f, kilnDepth*0.8f);
    glutSolidCube(1.0f);
    glPopMatrix();

    glDisable(GL_BLEND);
    glEnable(GL_LIGHTING);
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

//------------------- Draw Press Device ---------------------------
void drawPressDevice() {
    // Press device parameters
    float baseX = 0.0f;              // X position centered at origin
    float baseZ = -6.f;             // Z position behind conveyor belt
    float baseY = 0.0f;              // Base at floor level
    float baseWidth = 2.0f;          // Width of base
    float baseDepth = 2.0f;          // Depth of base
    float baseHeight = 7.f;         // Height of vertical support
    float armLength = 3.0f;          // Length of horizontal arm
    float pistonRadius = 0.4f;       // Radius of piston
    float headSize = 0.8f;           // Size of press head

    float pressCycle = fmod(beltOffset * (2.0f * M_PI / 5.0f), 2.0f * M_PI);
    float pressPosition = 0.5f * sin(pressCycle + 0.6);

    // Use metal texture for press device
    glBindTexture(GL_TEXTURE_2D, metalTex);
    glEnable(GL_TEXTURE_2D);
    glColor3f(0.6f, 0.6f, 0.6f);  // Slightly lighter gray than support structure

    // Draw base of press
    glPushMatrix();
        glTranslatef(baseX, baseY + baseHeight/2, baseZ);
        glScalef(baseWidth, baseHeight, baseDepth);
        glutSolidCube(1.0f);
    glPopMatrix();

    // Draw horizontal arm extending from the top of the base toward conveyor
    glPushMatrix();
        glTranslatef(baseX, baseY + baseHeight, baseZ + armLength/2);
        glScalef(baseWidth/1.5f, baseWidth/1.5f, armLength);
        glutSolidCube(1.0f);
    glPopMatrix();

    // Draw vertical piston housing at end of arm
    glPushMatrix();
        glTranslatef(baseX, baseY + baseHeight - baseWidth/3, beltZMin + beltWidth/2);
        glScalef(baseWidth/1.5f, baseWidth*1.5f, baseWidth/1.5f);
        glutSolidCube(1.0f);
    glPopMatrix();

    // Draw piston that moves up and down
    glPushMatrix();
        // Position at end of arm, with y-position affected by press cycle
        glTranslatef(baseX, baseY + baseHeight - baseWidth - pressPosition, beltZMin + beltWidth/2);

        // Create a quadric object for drawing the cylinder
        GLUquadric* quad = gluNewQuadric();
        gluQuadricTexture(quad, GL_TRUE);
        gluQuadricNormals(quad, GLU_SMOOTH);

        // Rotate to align with y-axis
        glRotatef(90.0f, 1.0f, 0.0f, 0.0f);

        // Draw piston shaft
        glColor3f(0.7f, 0.7f, 0.7f);  // Lighter color for piston
        gluCylinder(quad, pistonRadius, pistonRadius, 2.0f, 16, 4);

        // Draw press head at bottom of piston
        glColor3f(0.5f, 0.5f, 0.5f);  // Darker color for press head
        glTranslatef(0.0f, 0.0f, 2.0f);
        glRotatef(180.0f, 1.0f, 0.0f, 0.0f);  // Flip to draw disk at bottom
        gluDisk(quad, 0.0f, headSize, 16, 1);
        gluCylinder(quad, headSize, headSize, headSize/2, 16, 4);
        glTranslatef(0.0f, 0.0f, headSize/2);
        gluDisk(quad, 0.0f, headSize, 16, 1);

        gluDeleteQuadric(quad);
    glPopMatrix();

    // Draw indicator light on side of press (changes color based on press position)
    glDisable(GL_TEXTURE_2D);
    glPushMatrix();
        glTranslatef(baseX + baseWidth/2 + 0.1f, baseY + baseHeight*0.8f, baseZ);
        // Light color based on press position (red when down, green when up)
        if (pressPosition < 0) {
            glColor3f(1.0f, 0.2f, 0.2f);  // Red when pressing down
        } else {
            glColor3f(0.2f, 1.0f, 0.2f);  // Green when retracting
        }
        glutSolidSphere(0.2f, 16, 16);
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

    // Draw the support structure and rollers.
    drawSupportStructure();
    // Draw rollers so that the belt appears to wrap around them.
    drawRollers();
    drawPressDevice();
    drawBackgroundSprings();
    drawKiln();
    // Draw the conveyor belt.
    drawConveyorBelt();

    // Draw items on the conveyor, spaced evenly.
    float spacing = beltXLength / numItems; // spacing along the belt
    for (int i = 0; i < numItems; i++) {
        float itemOffset = fmod(beltOffset + i * spacing, beltXLength);
        drawProcessedItem(itemOffset);
    }
    drawParticles();
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
