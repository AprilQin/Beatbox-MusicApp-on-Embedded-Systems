#include <stdbool.h>
#include "joystick.h"

#include "file.h"

void joystickInit(){
	writeToFile(GPIO, "27"); //Push In
	writeToFile(GPIO, "26"); //UP
	writeToFile(GPIO, "46"); //DOWNs
	writeToFile(GPIO, "47"); //LEFT
	writeToFile(GPIO, "65"); //RIGHT
}

bool joystick_pressedIn(){
	int res = readFromFile(JSIN);
	return res == 0;	
}

bool joystick_pressedUp(){
	int res = readFromFile(JSUP);
	return res == 0;
}

bool joystick_pressedDw(){
	int res = readFromFile(JSDW);
	return res == 0;
}

bool joystick_pressedRt(){
	int res = readFromFile(JSRT);
	return res == 0;
}

bool joystick_pressedLt(){
	int res = readFromFile(JSLT);
	return res == 0;
}

void joystick_waitfor_release(){
	while (	joystick_pressedIn() ||
			joystick_pressedUp() ||
			joystick_pressedDw() ||
			joystick_pressedLt() ||
			joystick_pressedRt()	){

		continue;
	}
}	
