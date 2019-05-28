#ifndef JOYSTICK_H
#define JOYSTICK_H

#define GPIO "/sys/class/gpio/export"

#define JSIN "/sys/class/gpio/gpio27/value"
#define JSUP "/sys/class/gpio/gpio26/value"
#define JSDW "/sys/class/gpio/gpio46/value"
#define JSLT "/sys/class/gpio/gpio65/value"
#define JSRT "/sys/class/gpio/gpio47/value"


void joystickInit();
bool joystick_pressedIn();
bool joystick_pressedUp();
bool joystick_pressedDw();
bool joystick_pressedRt();
bool joystick_pressedLt();
void joystick_waitfor_release();
#endif