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
    struct tm* local, * ptr; //存储时间
    time_t t; //时间结构或者对象
    t = time(NULL); //获取当前系统的日历时间
    local = localtime(&t);//将日历时间转化为本地时间
    ptr = gmtime(&t);//将日历时间转化为世界标准时间
    return ctime(&t);
}*/