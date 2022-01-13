#include "schedule.h"

Queue::Queue() {
	head = new Node;
	head->next = NULL;
}

Node *Queue::create() {
	if (head == NULL) {
		head = new Node;
		head->next = NULL;
	}
	return head;
}

Node *Queue::find(int roomId) {
	for (Node *n = head->next; n != NULL; n = n->next) {
		if (roomId == n->roomId) {
			return n;
		}
	}
	return NULL;
}

Node *Queue::insert(int roomId, int serveObjId, int speed, int temp, double time) {
	Node *n = new Node;
	n->next = NULL;
	n->roomId = roomId;
	n->serveObjId = serveObjId;
	n->speed = speed;
	n->temp = temp;
	n->time = time;
	
	n->next = head->next;
	head->next = n;

	return n;
}

Node *Queue::update(int roomId, int serveObjId, int speed, int temp, double time) {
	Node *n = find(roomId);
	if (n == NULL) {
		return NULL;
	}
	if (serveObjId >= 0) {
		n->serveObjId = serveObjId;
	}
	if (speed >= 0) {
		n->speed = speed;
	}
	if (temp >= 0) {
		n->temp = temp;
	}
	if (time >= 0) {
		n->time = time;
	}
	return n;
}

Node *Queue::removeFrom(int roomId) {
	Node *t = NULL;
	for (Node *n = head; n->next != NULL; n = n->next) {
		if (n->next->roomId == roomId) {
			t = n->next;
			n->next = n->next->next;
			//delete(t);
			return t;
		}
	}
	return t;
}

Node *Queue::removeFrom(Node *targ) {
	for (Node *n = head; n->next != NULL; n = n->next) {
		if (n->next == targ) {
			n->next = n->next->next;
			targ->next = NULL;
			return targ;
		}
	}
	return NULL;
}

int Queue::getlen() {
	int len = 0;
	for (Node *n = head->next; n != NULL; n = n->next) {
		len++;
	}
	return len;
}

int WaitQueue::addToWait(int roomId, int speed, int temp, double waitTime) {
	Node *n = insert(roomId, -1, speed, temp, waitTime);
	if (n == NULL) {
		return -1;
	}
	else {
		return 0;
	}
}

int WaitQueue::updateWait(int roomId, int temp) {
	Node *n = update(roomId, -1, -1, temp, -1);
	if (n == NULL) {
		return -1;
	}
	else {
		return 0;
	}
}

Node *WaitQueue::findTime0SpeedMaxTimeMin() {
	int maxSpeed = 0;
	double minWaitTime = s + 1;
	Node *targ = NULL;
	for (Node *t = head->next; t != NULL; t = t->next) {
		if (t->time <= 0) {
			if (t->speed > maxSpeed) {
				targ = t;
				maxSpeed = t->speed;
				minWaitTime = t->time;
			}
			else if (t->speed == maxSpeed && t->time < minWaitTime) {
				targ = t;
				maxSpeed = t->speed;
				minWaitTime = t->time;
			}
		}
	}
	return targ;
}

Node *WaitQueue::findTimeMin() {
	double minTime = s + 1;
	Node *targ = NULL;
	for (Node *t = head->next; t != NULL; t = t->next) {
		if (t->time < minTime) {
			targ = t;
			minTime = t->time;
			
		}
	}
	return targ;
}

int WaitQueue::timeDown(double time) {
	for (Node *n = head->next; n != NULL; n = n->next) {
		n->time -= time;
	}
	return 0;
}

int ServiceQueue::addToServe(int roomId, int serveObjId, int speed, int temp, double serveTime) {
	Node *res = insert(roomId, serveObjId, speed, temp, serveTime);
	//数据库：被调度次数+1，记录调度信息
	if (res == NULL || getlen() > y) {
		return -1;
	}
	else {
		return 0;
	}
}

int ServiceQueue::updateServe(int roomId, int serveObjId, int temp) {
	Node *n = update(roomId, serveObjId, -1, temp, -1);
	if (n == NULL) {
		return -1;
	}
	else {
		return 0;
	}
}

Node *ServiceQueue::findSpeedMinTimeMax() {
	int minSpeed = 4;
	double maxServeTime = 0;
	Node *targ = NULL;
	for (Node *t = head->next; t != NULL; t = t->next) {
		if (t->speed < minSpeed && t->speed > 0) {
			targ = t;
			minSpeed = t->speed;
			maxServeTime = t->time;
		}
		else if (t->speed == minSpeed && t->time > maxServeTime) {
			targ = t;
			minSpeed = t->speed;
			maxServeTime = t->time;
		}
	}
	return targ;
}

Node *ServiceQueue::findTimeMax() {
	double maxTime = -1;
	Node *targ = NULL;
	for (Node *t = head->next; t != NULL; t = t->next) {
		if (t->time > maxTime) {
			targ = t;
			maxTime = t->time;
		}
	}
	return targ;
}

int ServiceQueue::timeUp(double time) {
	for (Node *n = head->next; n != NULL; n = n->next) {
		n->time += time;
	}
	return 0;
}

int ServiceQueue::getRoomAcStatus(int roomId, string type) {
	Node *n = find(roomId);
	if (n != NULL) {
		if (type == "speed") {
			return n->speed;
		}
		else if (type == "temp") {
			return n->temp;
		}
		return -1;
	}
	else {
		return -1;
	}
}

void Queue::read() {
	for (Node *n = head->next; n != NULL; n = n->next) {
		printf("%d->", n->roomId);
	}
}
