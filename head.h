#pragma once
#include <stdio.h>
#include <iostream>
#include <string>
#include <cstring>
#include <fstream>
#include <io.h>
#include <direct.h>
#include <vector>
#include <sstream>
#include <time.h>
#include <algorithm>
using namespace std;
/*string getTime(void) {
    struct tm* local, * ptr; //�洢ʱ��
    time_t t; //ʱ��ṹ���߶���
    t = time(NULL); //��ȡ��ǰϵͳ������ʱ��
    local = localtime(&t);//������ʱ��ת��Ϊ����ʱ��
    ptr = gmtime(&t);//������ʱ��ת��Ϊ�����׼ʱ��
    return ctime(&t);
}*/