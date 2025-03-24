#include "keyboardUtilities.h"
#include <cmath>

KeyboardUtilities::KeyboardUtilities()
    : posX(0.0f), posY(4.0f), posZ(10.0f),
      yaw(0.f), pitch(-0.f),
      moveSpeed(0.2f), rotSpeed(1.6f)
{
    for (int i = 0; i < 256; i++) {
        keyStates[i] = false;
        specialKeyStates[i] = false;
    }
}

void KeyboardUtilities::update() {
    float radYaw = yaw * M_PI / 180.0f;

    if (keyStates['w'] || keyStates['W']) {
        posX += moveSpeed * sin(radYaw);
        posZ += -moveSpeed * cos(radYaw);
    }
    if (keyStates['s'] || keyStates['S']) {
        posX -= moveSpeed * sin(radYaw);
        posZ -= -moveSpeed * cos(radYaw);
    }
    if (keyStates['a'] || keyStates['A']) {
        posX -= moveSpeed * cos(radYaw);
        posZ -= moveSpeed * sin(radYaw);
    }
    if (keyStates['d'] || keyStates['D']) {
        posX += moveSpeed * cos(radYaw);
        posZ += moveSpeed * sin(radYaw);
    }

    if (specialKeyStates[GLUT_KEY_LEFT])
        yaw -= rotSpeed;
    if (specialKeyStates[GLUT_KEY_RIGHT])
        yaw += rotSpeed;
    if (specialKeyStates[GLUT_KEY_UP]) {
        pitch += rotSpeed;
        if (pitch > 89.0f)
            pitch = 89.0f;
    }
    if (specialKeyStates[GLUT_KEY_DOWN]) {
        pitch -= rotSpeed;
        if (pitch < -89.0f)
            pitch = -89.0f;
    }
}

float KeyboardUtilities::getPosX() const { return posX; }
float KeyboardUtilities::getPosY() const { return posY; }
float KeyboardUtilities::getPosZ() const { return posZ; }
float KeyboardUtilities::getYaw() const  { return yaw; }
float KeyboardUtilities::getPitch() const { return pitch; }

void KeyboardUtilities::keyboardDown(unsigned char key) {
    keyStates[key] = true;
}

void KeyboardUtilities::keyboardUp(unsigned char key) {
    keyStates[key] = false;
}

void KeyboardUtilities::specialKeyDown(int key) {
    specialKeyStates[key] = true;
}

void KeyboardUtilities::specialKeyUp(int key) {
    specialKeyStates[key] = false;
}
