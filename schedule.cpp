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
	//��һ������ִ�е���	
	cout << "���ڶԷ���" << roomId << "��������е���" << endl;
	Node *exists = serviceQueue.find(roomId);
	if (exists != NULL) {
		printf("����%d�Ѿ��ڱ������У�\n", roomId);
		return -2;
	}
	exists = waitQueue.find(roomId);
	if (exists != NULL) {
		printf("����%d�Ѿ����ڵȴ���\n", roomId);
		return -2;
	}
	//1 Ѱ�����޿��з���������У�������������
	if (serviceQueue.getlen() < y) {
		int serveObjId = findServeObj(-1);
		serviceQueue.addToServe(roomId, serveObjId, speed, temp, 0);
		serveObj[serveObjId].serveRequest(roomId, speed, temp);
		dbFacade.schedule(roomId, to_string(time(NULL)));
		printf("�ҵ����еķ������%d\n", serveObjId);
		return serveObjId;
	}
	//2 ���޿��з������ʹ�����ȼ����ȣ�Ѱ�����ȼ����͵ķ������
	//����������У����շ������->����ʱ������ҵ�һ�����滻������
	Node *replaced = serviceQueue.findSpeedMinTimeMax();
	//2.1 ����������ٸ��ߣ��������������󣬱��滻��������ȴ�
	if (speed > replaced->speed) {
		int serveObjId = replaced->serveObjId;
		//�����Ƴ�������в�ֹͣ���񣬼���ȴ����У����õȴ�ʱ��s
		serviceQueue.removeFrom(replaced);
		serveObj[serveObjId].stopRequest();
		waitQueue.addToWait(replaced->roomId, replaced->speed, replaced->temp, s);
		this->stopFee(replaced->roomId);
		//����������������У����÷���ʱ��0������������������
		serviceQueue.addToServe(roomId, serveObjId, speed, temp, 0);
		serveObj[serveObjId].serveRequest(roomId, speed, temp);
		dbFacade.schedule(roomId, to_string(time(NULL)));
		printf("���󽫱�������� %d ���񣻷���%d�����󽫵ȴ�\n", serveObjId, replaced->roomId);
		delete(replaced);
		return serveObjId;
	}
	//2.2 �������������ȣ�����ȴ�s�����������
	else if (speed == replaced->speed) {
		waitQueue.addToWait(roomId, speed, temp, s);
		printf("�����Ѽ���ȴ����У�%.2lf������\n",s);
		return -1;
	}
	//2.3 ����������ٸ��ͣ��������޵ȴ�ֱ���з���������
	else {
		waitQueue.addToWait(roomId, speed, temp, INT_MAX);
		printf("�����Ѽ���ȴ����У��ȴ����еķ������\n");
		return -1;
	}
}

int Scheduler::forwardTime() {
	if (lastTime > 0) {
		//�Եȴ����С���������е��������ʱ������
		double newTime = clock() / CLOCKS_PER_SEC;
		double passTime = newTime - lastTime;
		lastTime = newTime;
		serviceQueue.timeUp(passTime);
		waitQueue.timeDown(passTime);
		printf("ʱ���ƽ���%.2lf��\n", passTime);
		//2.2.2 ��������ĵȴ�ʱ���С��0ʱ�����ܶ����
		Node *chosed = waitQueue.findTime0SpeedMaxTimeMin();
		if (chosed != NULL) {
			//Ѱ�ҵȴ������з������->�ȴ�ʱ����С�������Ƴ��ȴ�����
			waitQueue.removeFrom(chosed);
			//Ѱ�ҷ�������з���ʱ���������,�����Ƴ�������У�ֹͣ����,����ȴ����У����õȴ�ʱ��s
			Node *replaced = serviceQueue.findTimeMax();
			serviceQueue.removeFrom(replaced);
			int serveObjId = replaced->serveObjId;
			serveObj[serveObjId].stopRequest();
			waitQueue.addToWait(replaced->roomId, replaced->speed, replaced->temp, s);
			this->stopFee(replaced->roomId);
			//����ѡ���������������У����÷���ʱ��0,����������������	
			serviceQueue.addToServe(chosed->roomId, serveObjId, chosed->speed, chosed->temp, 0);
			serveObj[serveObjId].serveRequest(chosed->roomId, chosed->speed, chosed->temp);
			dbFacade.schedule(chosed->roomId, to_string(time(NULL)));
			printf("����%d�ĵȴ�ʱ��Ϊ%.2lf������ʼ����\n", chosed->roomId, chosed->time);
			printf("����%d�����󽫵ȴ�\n", replaced->roomId);
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
	//���з����������ֹͣ�ͷ�ʱ����������п���
	printf("�з��������У�ִ�е��ȣ�\n");
	Node *chosed = waitQueue.findTimeMin();
	if (chosed != NULL) {
		//�����������ڵȴ���Ѱ�ҵȴ������еȴ�ʱ�����,�Ƴ��ȴ�����
		waitQueue.removeFrom(chosed);
		//���������У����÷���ʱ��0��Ѱ�ҿ��еķ�����󣬷��������
		int serveObjId = findServeObj(-1);
		serviceQueue.addToServe(chosed->roomId, serveObjId, chosed->speed, chosed->temp, 0);
		serveObj[serveObjId].serveRequest(chosed->roomId, chosed->speed, chosed->temp);
		dbFacade.schedule(chosed->roomId, to_string(time(NULL)));
		printf("����%d�����󱻷���\n", chosed->roomId);
		delete(chosed);
		return 1;
	}
	return 0;
}
	

int Scheduler::setSpeed(int roomId, int speed) {
	printf("����%d����ı����%d\n", roomId, speed);
	Node *n = serviceQueue.removeFrom(roomId);
	if (n == NULL) {
		n = waitQueue.removeFrom(roomId);
		if (n == NULL) {
			printf("����%d���ڶ����У�\n", roomId);
			return -1;
		}
		//���˷������ڵȴ�������ӵȴ��������Ƴ�
	}
	else {
		//���˷������ڱ����񣬽���ӷ���������Ƴ���ֹͣ����
		int serveObjId = n->serveObjId;
		serveObj[serveObjId].stopRequest();
	}
	//������е���
	int ret = scheduleAlgorithm(roomId, speed, n->temp);
	delete(n);
	return ret;
}

int Scheduler::setTemp(int roomId, int temp) {
	printf("����%d����ı��¶�%d\n", roomId, temp);
	if (waitQueue.updateWait(roomId, temp) == 0) {
		return 0;
	}
	if (serviceQueue.updateServe(roomId, -1, temp) == 0) {
		return 0;
	}
	printf("����%d���ڶ����У�\n", roomId);
	return -1;
}

int Scheduler::startWind(int roomId, int speed, int temp, int flag) {
	printf("����%d�����ͷ磬���٣�%d���¶ȣ�%d\n", roomId, speed, temp);
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
	printf("����%d����ֹͣ�ͷ�\n", roomId);
	Node *stopped = serviceQueue.removeFrom(roomId);
	if (stopped == NULL) {
		printf("���������δ�ҵ�\n");
		stopped = waitQueue.removeFrom(roomId);
		if (stopped != NULL) {
			printf("�ڵȴ��������ҵ�\n");
			return 0;
		}
		else {
			printf("����%d���ڷ��񡢵ȴ������У�\n", roomId);
			return -1;
		}
	}
	else {
		printf("�ڷ���������ҵ�\n");
		serveObj[stopped->serveObjId].stopRequest();
		printf("�������%d���ͷ�\n", stopped->serveObjId);
		delete(stopped);
		return 0;
	}
}

int Scheduler::getAcStatus(int roomId, string type) {
	printf("����%d�����ȡ״̬\n", roomId);
	return serviceQueue.getRoomAcStatus(roomId, type);
}

Scheduler::Scheduler() {
	lastTime = 0;
	for (int i = 0; i < y; i++) {
		serveObj[i].setId();
	}
}

Fee Scheduler::updateRoomTempGetFee(int roomId, double temp) {
	//�жϷ���յ��Ƿ������Ƿ��ͷ�
	string isOn = "", isServing = "";
	Node *room = serviceQueue.find(roomId);
	if (room == NULL) {
		room = waitQueue.find(roomId);
		if (room == NULL) {
			printf("����%d���ڶ����У�\n", roomId);
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

	//�����ݿ��л�ȡ����״̬��Ϣ��ʱ�䡢���١��ϴη��ã�
	roomStates info = dbFacade.queryRoomStatesRet(roomId);
	double fee = info.fee;
	double totFee = info.totalFee;
	time_t lastTime = atoi(info.time.c_str());
	string lastSpeed = info.targetSpeed;
	string lastIsServing = info.isServing;

	//�����·���
	time_t passTime = newTime - lastTime;
	double thisFee = 0;
	if (lastIsServing == "on") {
		thisFee = feeRate[toSpeed(lastSpeed)] * passTime;
	}
	fee += thisFee;
	totFee += thisFee;
	printf("���ڼ��㷿��%d�ķ���\n", roomId);
	cout << lastSpeed << endl;
	printf("passtime: %lld\n", passTime);
	printf("fee: %.2lf\n", thisFee);

	//д���µķ���״̬��Ϣ
	
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

	//���ط���
	Fee ret;
	ret.fee = fee;
	ret.totFee = totFee;
	return ret;
}

int Scheduler::stopFee(int roomId) {
	time_t newTime = time(NULL);

	//�����ݿ��л�ȡ����״̬��Ϣ��ʱ�䡢���١��ϴη��ã�
	roomStates info = dbFacade.queryRoomStatesRet(roomId);
	double fee = info.fee;
	double totFee = info.totalFee;
	time_t lastTime = atoi(info.time.c_str());
	string lastSpeed = info.targetSpeed;

	//�����·���
	time_t passTime = newTime - lastTime;
	double thisFee = 0;
	thisFee = feeRate[toSpeed(lastSpeed)] * passTime;
	fee += thisFee;
	totFee += thisFee;
	printf("����%dֹͣ�ͷ磬���ڼ������\n", roomId);
	cout << lastSpeed << endl;
	printf("passtime: %lld\n", passTime);
	printf("fee: %.2lf\n", thisFee);

	//д���µķ���״̬��Ϣ
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

	//�����ݿ��л�ȡ����״̬��Ϣ��ʱ�䡢���١��ϴη��ã�
	roomStates info = dbFacade.queryRoomStatesRet(roomId);
	double fee = info.fee;
	double totFee = info.totalFee;
	time_t lastTime = atoi(info.time.c_str());
	string lastSpeed = info.targetSpeed;

	printf("����%d���������ñ���ʹ�ÿյ��ķ���\n", roomId);

	//д���µķ���״̬��Ϣ
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
			cout << "colΪ��" << col;
			cout << "isOn: " << dbresult[col + 10] << endl;
			//if (dbresult[col + 10] == "on") {
				j++;
				string s_roomId = dbresult[col + 0];
				cout << "��ѯ�ķ����Ϊ��" << s_roomId << endl;
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
