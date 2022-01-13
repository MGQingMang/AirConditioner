#include <iostream>
#include "httplib.h"
#include "json.hpp"
#include <string>
#include <mutex>

#include "controller.h"
#include "schedule.h"
#include "global.h"

using namespace std;
using namespace httplib;
using json = nlohmann::json;

//string speedStr[4] = { "off", "low", "mid", "high" };
mutex svrLock;

int server(void){
	Server svr;
	Controller controller;
	
	/*controller.PowerOn();
	controller.sendParm();*/

	svr.set_file_extension_and_mimetype_mapping("json", "application/json");

	svr.Post("/customer", [&](const Request& req, Response& res, const ContentReader &content_reader) {
		svrLock.lock();
		std::string body;
		content_reader([&](const char *data, size_t data_length) {
			body.append(data, data_length);
			return true;
		});

		cout << "-POST-CUSTOMER------------------" << endl;
		cout << "Client: " << req.remote_addr << ":" << req.remote_port << endl;
		cout << "Recv HTTP body: " << body << endl;

		controller.forwardTime();

		json recv = json::parse(body);
		string cliHost = req.remote_addr;
		int id = recv["id"];
		string type = recv["type"];
		json data = recv["data"];
		int roomId = data["roomId"];
		json send;
		send["id"] = id;
		if (type == "TurnOnAc") {
			string speed = data["targetSpeed"];
			int temp = data["targetTemp"];
			controller.turnOn(roomId, speed, temp);
			send["type"] = (string)"ACK";
		}
		else if (type == "AdjustSpeed") {
			string speed = data["targetSpeed"];
			controller.setSpeed(roomId, speed);
			send["type"] = (string)"ACK";
		}
		else if (type == "AdjustTemp") {
			int temp = data["targetTemp"];
			controller.setTemp(roomId, temp);
			send["type"] = (string)"ACK";
		}
		else if (type == "StopWind") {
			controller.stopWind(roomId);
			send["type"] = (string)"ACK";
		}
		else if (type == "RestartWind") {
			string speed = data["targetSpeed"];
			int temp = data["targetTemp"];
			controller.restartWind(roomId, speed, temp);
			send["type"] = (string)"ACK";
		}
		else if (type == "TurnOffAc") {
			controller.turnOff(roomId);
			send["type"] = (string)"ACK";
		}
		else if (type == "UpdateRequest") {
			double roomTemp = data["roomTemp"];
			Fee fee = controller.updateRoomTempCalcFee(roomId, roomTemp);
			int speed = controller.getAcSpeed(roomId);
			int temp = controller.getAcTemp(roomId);
			//json rdata;
			if (speed > 0) {
				//rdata["status"] = (string)"on";
				send["status"] = (string)"on";
			}
			else {
				//rdata["status"] = (string)"off";
				send["status"] = (string)"off";
			}
			/*rdata["thisFee"] = (double)fee.fee;
			rdata["totalFee"] = (double)fee.totFee;
			rdata["speed"] = (string)speedStr(speed);
			rdata["targetTemp"] = (int)temp;
			rdata["temperature"] = data["roomTemp"];*/
			send["thisFee"] = (double)fee.fee;
			send["totalFee"] = (double)fee.totFee;
			send["speed"] = (string)speedStr(speed);
			send["targetTemp"] = (int)temp;
			send["temperature"] = data["roomTemp"];
			if (temp <= 25) {
				//rdata["mode"] = "cold";
				send["mode"] = "cold";
			}
			else {
				//rdata["mode"] = "hot";
				send["mode"] = "hot";
			}
			//send["type"] = (string)"UpdateFee";
			//send["data"] = (json)data;
		}
		
		res.set_content(send.dump(), "application/json");
		res.set_header("Access-Control-Allow-Origin", "*");
		res.set_header("Access-Control-Allow-Credentials", "true");
		res.set_header("Access-Control-Allow-Methods", "POST,GET,OPTIONS,DELETE");
		res.set_header("Access-Control-Allow-Headers", "Authorization,Accept,Origin,DNT,X-CustomHeader,Keep-Alive,User-Agent,X-Requested-With,If-Modified-Since,Cache-Control,Content-Type,Content-Range,Range");
		cout << "Send HTTP body: " << send.dump() << endl;
		cout << "-END-POST-----------------------" << endl << endl;
		svrLock.unlock();
	});

	svr.Options("/customer", [&](const Request& req, Response& res) {
		cout << "-OPTIONS-CUSTOMER--------------" << endl << endl;;
		res.set_header("Access-Control-Allow-Origin", "*");
		res.set_header("Access-Control-Allow-Credentials", "true");
		res.set_header("Access-Control-Allow-Methods", "POST,GET,OPTIONS,DELETE");
		res.set_header("Access-Control-Allow-Headers", "Authorization,Accept,Origin,DNT,X-CustomHeader,Keep-Alive,User-Agent,X-Requested-With,If-Modified-Since,Cache-Control,Content-Type,Content-Range,Range");
	});

	svr.Post("/manager", [&](const Request& req, Response& res, const ContentReader &content_reader) {
		svrLock.lock();
		std::string body;
		content_reader([&](const char *data, size_t data_length) {
			body.append(data, data_length);
			return true;
		});
		json j = json::parse(body);

		cout << "-POST-MANAGER-------------------" << endl;
		cout << "Client: " << req.remote_addr << ":" << req.remote_port << endl;
		cout << "Recv HTTP body: " << body << endl;
		json send;

		string type = j["type"];
		if (type == "RequestRoomState") {
			send = controller.getRoomState();
		}
		else if (type == "powerOn") {
			//string mode = j["mode"];
			controller.sendParm();
			controller.PowerOn();
			//send = ?
		}

		//Report
		else if (j["type"] == "Report") {
			json data = j["data"];
			time_t startTime_t = (time_t)data["startTime"];
			string endTime = (string)data["endTime"];
			string kind = (string)data["time"];
			/*if (kind == "month") {
				startTime -= ;
			}
			else if (kind == "week") {
				startTime -= ;
			}*/
			string startTime = to_string(startTime_t);
			Report  report = controller.askForReport(startTime, endTime);//需要Controller的实例
			int reportSize = report.DataForReportInfo.size();//房间总数 
			DataForReport dfr[x];
			int i = 0;
			for (vector<DataForReport>::iterator iter2 = report.DataForReportInfo.begin(); iter2 != report.DataForReportInfo.end(); iter2++, i++) {
				dfr[i] = *iter2;
			}
			send["reportId"] = report.reportId;
			//json roomData[report.DataForReportInfo.size()];
			for (int i = 0; i < report.DataForReportInfo.size(); i++) {//size为房间总数
				send["data"][i]["roomId"] = dfr[i].roomId;//房间号
				send["data"][i]["useCount"] = dfr[i].useAC;//空调使用次数
				send["data"][i]["freTargetTem"] = dfr[i].freTargetTem;//最常用目标温度
				send["data"][i]["freWindSpeed"] = dfr[i].freWindSpeed;//最常用风速
				send["data"][i]["tarTemCount"] = dfr[i].reachTargetTemp;//达到目标温度的次数
				send["data"][i]["schCount"] = dfr[i].schedule;//调度次数
				send["data"][i]["listCount"] = dfr[i].listCount;//详单记录数
				send["data"][i]["totalFee"] = dfr[i].totalFee;//总费用
			}
		}

		//Bill
		else if (j["type"] == "Bill") {
			int roomId = j["roomId"];
			Bill bill = controller.getBill(roomId);
			send["roomId"] = roomId;
			json data;
			/*data["startTime"] = bill.startTime;
			data["endTime"] = bill.endTime;*/
			time_t startTime = atol(bill.startTime.c_str());
			data["startTime"] = ctime(&startTime);
			time_t endTime = atol(bill.endTime.c_str());
			data["endTime"] = ctime(&endTime);
			data["feeTotal"] = bill.feeTotal;
			send["data"] = data;
		}

		//List
		else if (j["type"] == "List") {
			int roomId = j["roomId"];
			List list = controller.getList(roomId);
			//cout << "getlist success" << endl;
			//int listSize = list.fullList.listInfo.size();//房间总数
			ListInfo li[100];
			int i = 0;
			for (vector<ListInfo>::iterator iter3 = list.fullList.listInfo.begin(); iter3 != list.fullList.listInfo.end(); iter3++, i++) {
				//cout << "initialing list:" << i + 1 << endl;
				li[i] = *iter3;
				/*cout << "roomId = " << list.fullList.roomId << endl;
				cout << "startTime = " << iter3->startTime << endl;
				cout << "endTime = " << iter3->endTime << endl;
				cout << "targetSpeed = " << iter3->targetSpeed << endl;
				cout << "targetTemp = " << iter3->targetTemp << endl;
				cout << "segmentFee = " << iter3->segmentFee << endl;*/
			}
			send["roomId"] = list.fullList.roomId;
			for (int i = 0; i < list.fullList.listInfo.size(); i++) {
				time_t startTime = atol(li[i].startTime.c_str());
				send["data"][i]["startTime"] = ctime(&startTime);
				time_t endTime = atol(li[i].endTime.c_str());
				send["data"][i]["endTime"] = ctime(&endTime);
				send["data"][i]["targetSpeed"] = li[i].targetSpeed;
				send["data"][i]["targetTemp"] = li[i].targetTemp;
				send["data"][i]["segmentFee"] = li[i].segmentFee;
			}
		}

		res.set_header("Access-Control-Allow-Origin", "*");
		res.set_header("Access-Control-Allow-Credentials", "true");
		res.set_header("Access-Control-Allow-Methods", "POST,GET,OPTIONS,DELETE");
		res.set_header("Access-Control-Allow-Headers", "Authorization,Accept,Origin,DNT,X-CustomHeader,Keep-Alive,User-Agent,X-Requested-With,If-Modified-Since,Cache-Control,Content-Type,Content-Range,Range");

		res.set_content(send.dump(), "application/json");

		cout << "Send HTTP body: " << send.dump() << endl;
		cout << "-END-POST-----------------------" << endl << endl;
		svrLock.unlock();
	});

	svr.Options("/manager", [&](const Request& req, Response& res) {
		cout << "-OPTIONS-MANAGER---------------" << endl << endl;;
		res.set_header("Access-Control-Allow-Origin", "*");
		res.set_header("Access-Control-Allow-Credentials", "true");
		res.set_header("Access-Control-Allow-Methods", "POST,GET,OPTIONS,DELETE");
		res.set_header("Access-Control-Allow-Headers", "Authorization,Accept,Origin,DNT,X-CustomHeader,Keep-Alive,User-Agent,X-Requested-With,If-Modified-Since,Cache-Control,Content-Type,Content-Range,Range");
	});

	svr.listen("0.0.0.0", port);

	return 0;
}
