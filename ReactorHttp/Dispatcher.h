#pragma once
#include"Channel.h"
#include"EventLoop.h"
typedef struct Dispatcher
{
	//int
	void* (*init)();
	//添加
	int (*add)(struct Channel* channel, struct EventLoop* evLoop);
	//删除
	int (*remove)(struct Channel* channel, struct EventLoop* evLoop);
	//修改
	int (*modify)(struct Channel* channel, struct EventLoop* evLoop);
	//事件检测
	int (*dispatch)(struct EventLoop* evLoop, int timeout);//超时 timeout单位：s
	//清除数据，关闭fd或释放内存
	int (*clear)(struct EventLoop* evLoop);

}Dispatcher;