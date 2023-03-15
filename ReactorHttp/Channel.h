#pragma once
#include<stdbool.h>

//回调函数的函数指针
typedef int(*handleFund)(void* arg);

typedef struct Channel {
	//文件描述符
	int fd;
	//对应的事件
	int events;
	//回调函数
	handleFund readCallback;
	handleFund writeCallback;
	//回调函数参数
	void* arg;
}Channel;

//定义文件描述符的读写事件
enum FDEvent {
	TimeOut = 0x01,
	ReadEvent = 0x02,
	WriteEvent = 0x04
};

//初始化一个Channel
struct Channel* channelInit(int fd, int events, handleFund readFunc, handleFund writeFunc, void* arg);

//修改fd的写事件(检测或不检测)
void writeEventEnable(struct Channel* channel, bool flag);

//判断是否需要检测文件描述符的写事件
bool isWriteEventEnable(struct Channel* channel);