#include "schedule.h"

double feeRate[4] = { 0, 0.016, 0.033, 0.05 };
//string speedStr[4] = { "off", "low", "mid", "high" };

int ServeObj::getServeObj() {
	return roomId;
}

int ServeObj::serveRequest(int roomId, int speed, int temp) {
	this->roomId = roomId;
	this->speed = speed;
	this->temp = temp;
	return roomId;
}

int ServeObj::stopRequest() {
	roomId = -1;
	speed = -1;
	temp = -1;
	return 0;
}

int ServeObj::setId() {
	roomId = -1;
	return 0;
}

int Scheduler::findServeObj(int roomId) {
	
	for (int i = 0; i < y; i++) {
		if (serveObj[i].getServeObj() == roomId) {
			return i;
		}
	}
	return -1;
}

int Scheduler::scheduleAlgorithm(int roomId, int speed, int temp) {
	//对一个请求执行调度	
	cout << "正在对房间" << roomId << "的请求进行调度" << endl;
	Node *exists = serviceQueue.find(roomId);
	if (exists != NULL) {
		printf("房间%d已经在被服务中！\n", roomId);
		return -2;
	}
	exists = waitQueue.find(roomId);
	if (exists != NULL) {
		printf("房间%d已经正在等待！\n", roomId);
		return -2;
	}
	//1 寻找有无空闲服务对象，若有，将其分配给请求
	if (serviceQueue.getlen() < y) {
		int serveObjId = findServeObj(-1);
		serviceQueue.addToServe(roomId, serveObjId, speed, temp, 0);
		serveObj[serveObjId].serveRequest(roomId, speed, temp);
		dbFacade.schedule(roomId, to_string(time(NULL)));
		printf("找到空闲的服务对象：%d\n", serveObjId);
		return serveObjId;
	}
	//2 若无空闲服务对象，使用优先级调度，寻找优先级更低的服务对象
	//遍历服务队列，按照风速最低->服务时间最长，找到一个被替换的请求
	Node *replaced = serviceQueue.findSpeedMinTimeMax();
	//2.1 若新请求风速更高，立即服务新请求，被替换的请求需等待
	if (speed > replaced->speed) {
		int serveObjId = replaced->serveObjId;
		//将其移出服务队列并停止服务，加入等待队列，设置等待时间s
		serviceQueue.removeFrom(replaced);
		serveObj[serveObjId].stopRequest();
		waitQueue.addToWait(replaced->roomId, replaced->speed, replaced->temp, s);
		this->stopFee(replaced->roomId);
		//将新请求加入服务队列，设置服务时间0，令服务对象服务该请求
		serviceQueue.addToServe(roomId, serveObjId, speed, temp, 0);
		serveObj[serveObjId].serveRequest(roomId, speed, temp);
		dbFacade.schedule(roomId, to_string(time(NULL)));
		printf("请求将被服务对象 %d 服务；房间%d的请求将等待\n", serveObjId, replaced->roomId);
		delete(replaced);
		return serveObjId;
	}
	//2.2 若新请求风速相等，令其等待s秒后立即服务
	else if (speed == replaced->speed) {
		waitQueue.addToWait(roomId, speed, temp, s);
		printf("请求已加入等待队列，%.2lf秒后服务\n",s);
		return -1;
	}
	//2.3 若新请求风速更低，令其无限等待直到有服务对象空闲
	else {
		waitQueue.addToWait(roomId, speed, temp, INT_MAX);
		printf("请求已加入等待队列，等待空闲的服务对象\n");
		return -1;
	}
}

int Scheduler::forwardTime() {
	if (lastTime > 0) {
		//对等待队列、服务队列中的请求进行时间推移
		double newTime = clock() / CLOCKS_PER_SEC;
		double passTime = newTime - lastTime;
		lastTime = newTime;
		serviceQueue.timeUp(passTime);
		waitQueue.timeDown(passTime);
		printf("时间推进：%.2lf秒\n", passTime);
		//2.2.2 当有请求的等待时间减小到0时（可能多个）
		Node *chosed = waitQueue.findTime0SpeedMaxTimeMin();
		if (chosed != NULL) {
			//寻找等待队列中风速最高->等待时间最小的请求，移出等待队列
			waitQueue.removeFrom(chosed);
			//寻找服务队列中服务时间最长的请求,将其移出服务队列，停止服务,加入等待队列，设置等待时间s
			Node *replaced = serviceQueue.findTimeMax();
			serviceQueue.removeFrom(replaced);
			int serveObjId = replaced->serveObjId;
			serveObj[serveObjId].stopRequest();
			waitQueue.addToWait(replaced->roomId, replaced->speed, replaced->temp, s);
			this->stopFee(replaced->roomId);
			//将被选择的请求加入服务队列，设置服务时间0,令服务对象服务该请求	
			serviceQueue.addToServe(chosed->roomId, serveObjId, chosed->speed, chosed->temp, 0);
			serveObj[serveObjId].serveRequest(chosed->roomId, chosed->speed, chosed->temp);
			dbFacade.schedule(chosed->roomId, to_string(time(NULL)));
			printf("房间%d的等待时间为%.2lf，将开始服务\n", chosed->roomId, chosed->time);
			printf("房间%d的请求将等待\n", replaced->roomId);
			delete(replaced);
			delete(chosed);
			return 1;
		}	
	}
	else {
		lastTime = clock() / CLOCKS_PER_SEC;
	}
	return 0;
}

int Scheduler::haveFreeServeObj() {
	//当有服务对象被请求停止送风时，服务对象有空闲
	printf("有服务对象空闲，执行调度！\n");
	Node *chosed = waitQueue.findTimeMin();
	if (chosed != NULL) {
		//若有请求正在等待，寻找等待队列中等待时间最长者,移出等待队列
		waitQueue.removeFrom(chosed);
		//加入服务队列，设置服务时间0，寻找空闲的服务对象，服务该请求
		int serveObjId = findServeObj(-1);
		serviceQueue.addToServe(chosed->roomId, serveObjId, chosed->speed, chosed->temp, 0);
		serveObj[serveObjId].serveRequest(chosed->roomId, chosed->speed, chosed->temp);
		dbFacade.schedule(chosed->roomId, to_string(time(NULL)));
		printf("房间%d的请求被服务\n", chosed->roomId);
		delete(chosed);
		return 1;
	}
	return 0;
}
	

int Scheduler::setSpeed(int roomId, int speed) {
	printf("房间%d请求改变风速%d\n", roomId, speed);
	Node *n = serviceQueue.removeFrom(roomId);
	if (n == NULL) {
		n = waitQueue.removeFrom(roomId);
		if (n == NULL) {
			printf("房间%d不在队列中！\n", roomId);
			return -1;
		}
		//若此房间正在等待，将其从等待队列中移出
	}
	else {
		//若此房间正在被服务，将其从服务队列中移出，停止服务
		int serveObjId = n->serveObjId;
		serveObj[serveObjId].stopRequest();
	}
	//对其进行调度
	int ret = scheduleAlgorithm(roomId, speed, n->temp);
	delete(n);
	return ret;
}

int Scheduler::setTemp(int roomId, int temp) {
	printf("房间%d请求改变温度%d\n", roomId, temp);
	if (waitQueue.updateWait(roomId, temp) == 0) {
		return 0;
	}
	if (serviceQueue.updateServe(roomId, -1, temp) == 0) {
		return 0;
	}
	printf("房间%d不在队列中！\n", roomId);
	return -1;
}

int Scheduler::startWind(int roomId, int speed, int temp, int flag) {
	printf("房间%d请求送风，风速：%d，温度：%d\n", roomId, speed, temp);
	if (flag == 1) {
		dbFacade.useAC(roomId, to_string(time(NULL)));
	}
	if (speed <= 0) {
		speed = defaultSpeed;
	}
	if (temp < 18) {
		temp = DefaultTemp;
	}
	int ret = scheduleAlgorithm(roomId, speed, temp);
	return ret;
}

int Scheduler::stopWind(int roomId, int flag) {
	if (flag == 1) {
		//dbFacade.useAC(roomId, to_string(time(NULL)));
	}
	else {
		dbFacade.reachTargetTemp(roomId, to_string(time(NULL)));
	}
	printf("房间%d请求停止送风\n", roomId);
	Node *stopped = serviceQueue.removeFrom(roomId);
	if (stopped == NULL) {
		printf("服务队列中未找到\n");
		stopped = waitQueue.removeFrom(roomId);
		if (stopped != NULL) {
			printf("在等待队列中找到\n");
			return 0;
		}
		else {
			printf("房间%d不在服务、等待队列中！\n", roomId);
			return -1;
		}
	}
	else {
		printf("在服务队列中找到\n");
		serveObj[stopped->serveObjId].stopRequest();
		printf("服务对象%d被释放\n", stopped->serveObjId);
		delete(stopped);
		return 0;
	}
}

int Scheduler::getAcStatus(int roomId, string type) {
	printf("房间%d请求获取状态\n", roomId);
	return serviceQueue.getRoomAcStatus(roomId, type);
}

Scheduler::Scheduler() {
	lastTime = 0;
	for (int i = 0; i < y; i++) {
		serveObj[i].setId();
	}
}

Fee Scheduler::updateRoomTempGetFee(int roomId, double temp) {
	//判断房间空调是否开启、是否送风
	string isOn = "", isServing = "";
	Node *room = serviceQueue.find(roomId);
	if (room == NULL) {
		room = waitQueue.find(roomId);
		if (room == NULL) {
			printf("房间%d不在队列中！\n", roomId);
			isServing = "off";
			isOn = "off";
		}
		else {
			isServing = "off";
			isOn = "on";
		}
	}
	else {
		isServing = "on";
		isOn = "on";
	}

	time_t newTime = time(NULL);

	//从数据库中获取房间状态信息（时间、风速、上次费用）
	roomStates info = dbFacade.queryRoomStatesRet(roomId);
	double fee = info.fee;
	double totFee = info.totalFee;
	time_t lastTime = atoi(info.time.c_str());
	string lastSpeed = info.targetSpeed;
	string lastIsServing = info.isServing;

	//计算新费用
	time_t passTime = newTime - lastTime;
	double thisFee = 0;
	if (lastIsServing == "on") {
		thisFee = feeRate[toSpeed(lastSpeed)] * passTime;
	}
	fee += thisFee;
	totFee += thisFee;
	printf("正在计算房间%d的费用\n", roomId);
	cout << lastSpeed << endl;
	printf("passtime: %lld\n", passTime);
	printf("fee: %.2lf\n", thisFee);

	//写入新的房间状态信息
	
	info.roomId = roomId;
	info.time = to_string(newTime);
	if (temp > 0) {
		info.roomTemp = temp;
	}
	info.segmentFee = thisFee;
	info.fee = fee;
	info.totalFee = totFee;	
	info.isOn = isOn;
	info.isServing = isServing;
	if (room != NULL) {
		info.targetSpeed = speedStr(room->speed);
		info.targetTemp = room->temp;
		info.serveObjId = room->serveObjId;
	}
	else {
		info.serveObjId = -1;
	}
	dbFacade.insertRoomStates(info);

	//返回费用
	Fee ret;
	ret.fee = fee;
	ret.totFee = totFee;
	return ret;
}

int Scheduler::stopFee(int roomId) {
	time_t newTime = time(NULL);

	//从数据库中获取房间状态信息（时间、风速、上次费用）
	roomStates info = dbFacade.queryRoomStatesRet(roomId);
	double fee = info.fee;
	double totFee = info.totalFee;
	time_t lastTime = atoi(info.time.c_str());
	string lastSpeed = info.targetSpeed;

	//计算新费用
	time_t passTime = newTime - lastTime;
	double thisFee = 0;
	thisFee = feeRate[toSpeed(lastSpeed)] * passTime;
	fee += thisFee;
	totFee += thisFee;
	printf("房间%d停止送风，正在计算费用\n", roomId);
	cout << lastSpeed << endl;
	printf("passtime: %lld\n", passTime);
	printf("fee: %.2lf\n", thisFee);

	//写入新的房间状态信息
	info.roomId = roomId;
	info.time = to_string(newTime);
	info.segmentFee = thisFee;
	info.fee = fee;
	info.totalFee = totFee;
	info.isOn = "off";
	info.isServing = "off";
	info.targetSpeed = "off";
	info.serveObjId = -1;
	dbFacade.insertRoomStates(info);

	return 0;
}

int Scheduler::resetFee(int roomId) {
	time_t newTime = time(NULL);

	//从数据库中获取房间状态信息（时间、风速、上次费用）
	roomStates info = dbFacade.queryRoomStatesRet(roomId);
	double fee = info.fee;
	double totFee = info.totalFee;
	time_t lastTime = atoi(info.time.c_str());
	string lastSpeed = info.targetSpeed;

	printf("房间%d开机，重置本次使用空调的费用\n", roomId);

	//写入新的房间状态信息
	info.roomId = roomId;
	info.time = to_string(newTime);
	info.segmentFee = 0;
	info.fee = 0;
	info.isOn = "on";
	info.isServing = "off";
	info.targetSpeed = "off";
	info.serveObjId = -1;
	
	dbFacade.insertRoomStates(info);

	return 0;
}


json Scheduler::getRoomState() {
	int row, col;
	char** dbresult;
	dbFacade.getRoomId(&row, &col, &dbresult);
	/*for (int i = 0; i < col * (row + 1); i++) {
		printf("%s\n", dbresult[i]);
	}*/

	json roomState;
	int j = -1;
	//for (int i = 0; i < row; i++) {
	//	int index = (i + 1) * 11;
	//	if (dbresult[index + 10] == "on") {
	//		j++;

	//		string s_roomId = dbresult[index + 0];
	//		string s_time = dbresult[index + 1];
	//		string s_isOn = dbresult[index + 10];
	//		string s_isServing = dbresult[index + 8];
	//		string s_CurrentTem = dbresult[index + 2];
	//		string s_TargetTem = dbresult[index + 7];
	//		string s_TargetSpeed = dbresult[index + 6];
	//		string s_totalFee = dbresult[index + 4];

	//		roomState["data"][j]["roomId"] = s_roomId;
	//		roomState["data"][j]["state_time"] = s_time;
	//		roomState["data"][j]["isOn"] = s_isOn;
	//		roomState["data"][j]["isServing"] = s_isServing;
	//		roomState["data"][j]["CurrentTem"] = s_CurrentTem;
	//		roomState["data"][j]["TargetTem"] = s_TargetTem;
	//		roomState["data"][j]["TargetSpeed"] = s_TargetSpeed;
	//		roomState["data"][j]["totalFee"] = s_totalFee;
	//		printf("%s", s_roomId);
	//	}
	//}

	for (int i = 1 ;i <= 4; i++) {
		dbFacade.queryroomstates(i, &row, &col, &dbresult);
		cout << "row: " << row << endl;
		if (row > 0) {
			cout << "col为：" << col;
			cout << "isOn: " << dbresult[col + 10] << endl;
			//if (dbresult[col + 10] == "on") {
				j++;
				string s_roomId = dbresult[col + 0];
				cout << "查询的房间号为：" << s_roomId << endl;
				string s_time = dbresult[col + 1];
				string s_isOn = dbresult[col + 10];
				string s_isServing = dbresult[col + 8];
				string s_CurrentTem = dbresult[col + 2];
				string s_TargetTem = dbresult[col + 7];
				string s_TargetSpeed = dbresult[col + 6];
				string s_totalFee = dbresult[col + 4];

				roomState["data"][j]["roomId"] = s_roomId;
				//roomState["data"][j]["state_time"] = s_time;
				roomState["data"][j]["isOn"] = s_isOn;
				roomState["data"][j]["isServing"] = s_isServing;
				roomState["data"][j]["CurrentTem"] = s_CurrentTem;
				roomState["data"][j]["TargetTem"] = s_TargetTem;
				roomState["data"][j]["TargetSpeed"] = s_TargetSpeed;
				roomState["data"][j]["totalFee"] = s_totalFee;

				time_t stateTime = atol(s_time.c_str());
				roomState["data"][j]["state_time"] = ctime(&stateTime);
			//}
		}
	}


	return roomState;
}
