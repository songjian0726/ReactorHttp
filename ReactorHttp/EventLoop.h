#pragma once
#include"Dispatcher.h"
#include"ChannelMap.h"
#include<pthread.h>
#include"TcpConnection.h"
#include<stdbool.h>

extern struct Dispatcher EpollDispatcher;//声明外部的全局变量
extern struct Dispatcher PollDispatcher;
extern struct Dispatcher SelectDispatcher;

enum ElemType { ADD, DELETE, MODIFY };
//定义任务队列的节点
typedef struct ChannelElement{
	int type;  //对Channel的处理方法
	struct Channel* channel;
	struct ChannelElement* next;
}ChannelElement;

struct Dispatcher;//前向引用

typedef struct EventLoop{
	bool isQuit;
	struct Dispatcher* dispatcher;
	void* dispatcherData;//泛型思想 兼容三种不同的结构

	//任务队列 链表实现
	struct ChannelElement* head;
	struct ChannelElement* tail;

	//map
	struct ChannelMap* channelMap;

	//线程id，name, mutex
	pthread_t threadID;
	char threadName[32];
	pthread_mutex_t mutex;//互斥锁
	int socketPair[2]; //用于本地通信的socket fd

}EventLoop;


//初始化
struct EventLoop* eventLoopInit();
struct EventLoop* eventLoopInitEx(const char* threadName);//子线程需要一个参数

//启动反应堆模型
int eventLoopRun(struct EventLoop* evLoop);

//处理被激活的fd
int eventActivate(struct EventLoop* evLoop, int fd, int event);

//向任务队列中添加任务
int eventLoopAddTask(struct EventLoop* evLoop, struct Channel* channel, int type);

//处理任务队列中的任务
int eventLoopProcessTask(struct EventLoop* evLoop);

//处理dispatcher中的节点
int eventLoopAdd(struct EventLoop* evLoop, struct Channel* channel);
int eventLoopRemove(struct EventLoop* evLoop, struct Channel* channel);
int eventLoopModify(struct EventLoop* evLoop, struct Channel* channel);

//释放channel
int destroyChannel(struct EventLoop* evLoop, struct Channel* channel);