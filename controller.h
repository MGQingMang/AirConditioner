#ifndef _CONTROLLER_H
#define _CONTROLLER_H

#include <iostream>
#include "global.h"
#include "schedule.h"
#include "json.hpp"
#include"head.h"
#include"BillCreater.h"
#include"ListCreater.h"
#include"ReportManagement.h"
using namespace std;
using json = nlohmann::json;

//typedef struct _roominfo {
//	string defMode;
//	int defUpTemp;
//	int defDownTemp;
//	int defTemp;
//	string defSpeed;
//	double defFeerate;
//	double formerFee;	
//}RoomInfo;

class Controller {
private:
	Scheduler scheduler;
	ReportManagement rm;
	BillCreater bc;
	ListCreater lc;

public:
	int setSpeed(int roomId, string speed);
	int setTemp(int roomId, int temp);
	int restartWind(int roomId, string speed, int temp);
	int stopWind(int roomId);
	int turnOn(int roomId, string speed, int temp);
	int turnOff(int roomId);
	Fee updateRoomTempCalcFee(int roomId, double temp);
	int getAcSpeed(int roomId);
	int getAcTemp(int roomId);
	int forwardTime();


	void PowerOn();//打开空调电源，初始化调度对象	
	void sendParm();//设置开机参数	

	json getRoomState();//查询房间状态

	//string getTime(void);
	List getList(int roomId);
	Bill getBill(int roomId);
	Report askForReport(string startTime, string endTime);

};




#endif