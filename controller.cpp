#include <string>
#include "controller.h"
using namespace std;

int Controller::setSpeed(int roomId, string speed) {
	return scheduler.setSpeed(roomId, toSpeed(speed));
}

int Controller::setTemp(int roomId, int temp) {
	return scheduler.setTemp(roomId, temp);
}

int Controller::restartWind(int roomId, string speed, int temp) {
	return scheduler.startWind(roomId, toSpeed(speed), temp, 0);
}

int Controller::stopWind(int roomId) {
	int ret =  scheduler.stopWind(roomId, 0);
	scheduler.stopFee(roomId);
	scheduler.haveFreeServeObj();
	return ret;
}

int Controller::turnOn(int roomId, string speed, int temp) {
	scheduler.resetFee(roomId);
	int ret = scheduler.startWind(roomId, toSpeed(speed), temp, 1);
	
	return ret;
}

int Controller::turnOff(int roomId) {
	int ret = scheduler.stopWind(roomId, 1);
	scheduler.stopFee(roomId);
	scheduler.haveFreeServeObj();
	return ret;
}

int Controller::forwardTime() {
	return scheduler.forwardTime();
}

Fee Controller::updateRoomTempCalcFee(int roomId, double temp) {
	return scheduler.updateRoomTempGetFee(roomId, temp);
}

int Controller::getAcSpeed(int roomId) {
	return scheduler.getAcStatus(roomId, "speed");
}

int Controller::getAcTemp(int roomId) {
	return scheduler.getAcStatus(roomId, "temp");
}

void Controller::PowerOn() {
	scheduler.PowerOn();
}
void Controller::sendParm() {
	scheduler.sendParm();
}
json Controller::getRoomState() {
	//todo
	return scheduler.getRoomState();

}
