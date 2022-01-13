#include <iostream>
#include "global.h"

int main() {
	server();

	return 0;
}

int toSpeed(string speed) {
	int speedNum = 0;
	if (speed == "high") {
		speedNum = 3;
	}
	else if (speed == "mid") {
		speedNum = 2;
	}
	else if (speed == "low") {
		speedNum = 1;
	}
	return speedNum;
}

string speedStr(int speed) {
	string str = "off";
	if (speed == 1) str = "low";
	else if (speed == 2) str = "mid";
	else if (speed == 3) str = "high";
	return str;
}