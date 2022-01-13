#ifndef _SCHEDULE_H
#define _SCHEDULE_H

#include <iostream>
#include "global.h"
#include "json.hpp"
#include <time.h>
#include "DBfacade.h"
using namespace std;
using json = nlohmann::json;

class ServeObj {
private:
	int roomId;
	int speed;
	int temp;
public:
	int getServeObj();
	int serveRequest(int roomId, int speed, int temp);
	int stopRequest();
	int setId();
};

typedef struct _node {
	int roomId;
	int serveObjId;
	int speed;
	int temp;
	double time;	//等待时间递减，服务时间递增
	struct _node *next;
}Node;

class Queue {
protected:
	Node *head;
	Node *insert(int roomId, int serveObjId, int speed, int temp, double time);
	Node *update(int roomId, int serveObjId, int speed, int temp, double time);
	Queue();	
	
public:
	Node *find(int roomId);
	Node *removeFrom(int roomId);
	Node *removeFrom(Node *targ);
	//int deleteFrom(int roomId);
	//int deleteFrom(Node *targ);
	int getlen();

	void read();

	Node *create();
};

class WaitQueue : public Queue {
public:
	int addToWait(int roomId, int speed, int temp, double waitTime);
	int updateWait(int roomId, int temp);
	Node *findTime0SpeedMaxTimeMin();
	Node *findTimeMin();
	int timeDown(double time);
};

class ServiceQueue : public Queue {
public:
	int addToServe(int roomId, int serveObjId, int speed, int temp, double serveTime);
	int updateServe(int roomId, int serveObjId, int temp);
	Node *findSpeedMinTimeMax();
	Node *findTimeMax();
	int timeUp(double time);
	int getRoomAcStatus(int roomId, string type);
};

class Scheduler {
private:
	ServeObj serveObj[y];
	WaitQueue waitQueue;
	ServiceQueue serviceQueue;
	DBfacade dbFacade;

	int ready;
	int defaultMode;
	int tempHighLimit;
	int tempLowLimit;
	int defaultTemp;
	int defaultFeeRate;
	int defaultSpeed;

	double lastTime;

	int findServeObj(int roomId);
	int scheduleAlgorithm(int roomId, int speed, int temp);

public:
	int setSpeed(int roomId, int speed);
	int setTemp(int roomId, int temp);
	int startWind(int roomId, int speed, int temp, int flag);
	int stopWind(int roomId, int flag);
	int getAcStatus(int roomId, string type);
	Fee updateRoomTempGetFee(int roomId, double temp);
	int stopFee(int roomId);
	int resetFee(int roomId);

	Scheduler();

	int forwardTime();
	int haveFreeServeObj();

	Node *createWaitQueue();
	Node *createServiceQueue();

	void PowerOn();
	int createScheduler();
	void sendParm();
	json getRoomState();
};

#endif