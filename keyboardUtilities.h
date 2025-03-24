#ifndef KEYBOARD_UTILITIES_H
#define KEYBOARD_UTILITIES_H

#include <GL/freeglut.h>

class KeyboardUtilities {
public:
    KeyboardUtilities();

    // Call this every frame to update camera movement based on pressed keys.
    void update();

    // Getters for camera parameters.
    float getPosX() const;
    float getPosY() const;
    float getPosZ() const;
    float getYaw() const;
    float getPitch() const;

    // Keyboard event handling.
    void keyboardDown(unsigned char key);
    void keyboardUp(unsigned char key);
    void specialKeyDown(int key);
    void specialKeyUp(int key);

private:
    // Camera position and orientation.
    float posX, posY, posZ;
    float yaw, pitch; // in degrees

    // Movement and rotation speeds.
    const float moveSpeed;
    const float rotSpeed;

    // Key state arrays for normal keys and special (arrow) keys.
    bool keyStates[256];
    bool specialKeyStates[256];
};

#endif
