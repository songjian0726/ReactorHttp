#include"Dispatcher.h"
#include<sys/select.h>
#include<stdio.h>
#include<stdlib.h>
#define Max (1024)

typedef struct SelectData {
	fd_set readSet;
	fd_set writeSet;
}SelectData;

static void* SelectInit();
static int SelectAdd(Channel* channel, EventLoop* evLoop);//添加
static int SelectRemove(Channel* channel, EventLoop* evLoop);//删除
static int SelectModify(Channel* channel, EventLoop* evLoop);//修改
static int SelectDispatch(EventLoop* evLoop, int timeout);//事件检测超时 timeout单位：s
static int SelectClear(EventLoop* evLoop);//清除数据，关闭fd或释放内存
static void setFdSet(Channel* channel, struct SelectData* data);
static void clearFdSet(Channel* channel, struct SelectData* data);


struct Dispatcher SelectDispatcher = {  //全局结构体变量
	SelectInit,
	SelectAdd,
	SelectRemove,
	SelectModify,
	SelectDispatch,
	SelectClear
};

static void* SelectInit() {
	struct SelectData* data = (struct SelectData*)malloc(sizeof(struct SelectData));
	FD_ZERO(&data->readSet);
	FD_ZERO(&data->writeSet);

	return data;
}

static void setFdSet(Channel* channel, struct SelectData* data) {
	if (channel->events & ReadEvent) {  //将自己定义的读写事件标志转化为该io复用系统的标志
		FD_SET(channel->fd, &data->readSet);
	}
	if (channel->events & WriteEvent) {
		FD_SET(channel->fd, &data->writeSet);
	}
}
static void clearFdSet(Channel* channel, struct SelectData* data) {
	if (channel->events & ReadEvent) {  //将自己定义的读写事件标志转化为该io复用系统的标志
		FD_CLR(channel->fd, &data->readSet);
	}
	if (channel->events & WriteEvent) {
		FD_CLR(channel->fd, &data->writeSet);
	}
}

static int SelectAdd(Channel* channel, EventLoop* evLoop) {
	struct SelectData* data = (struct SelectData*)evLoop->dispatcherData;
	if (channel->fd >= Max) {
		return -1;
	}
	setFdSet(channel, data);
	return 0;//成功返回0 失败返回-1
}

static int SelectRemove(Channel* channel, EventLoop* evLoop) {
	struct SelectData* data = (struct SelectData*)evLoop->dispatcherData;
	if (channel->fd >= Max) {
		return -1;
	}
	clearFdSet(channel, data);
	//通过channel释放对应的tcpconnection资源
	channel->destroyCallback(channel->arg);
	return 0;//成功返回0 失败返回-1
}

static int SelectModify(Channel* channel, EventLoop* evLoop) {
	struct SelectData* data = (struct SelectData*)evLoop->dispatcherData;
	clearFdSet(channel, data);
	setFdSet(channel, data);
	return 0;//成功返回0 失败返回-1
}

static int SelectDispatch(EventLoop* evLoop, int timeout) {
	struct SelectData* data = (struct SelectData*)evLoop->dispatcherData;
	struct timeval val;
	val.tv_sec = timeout;
	val.tv_usec = 0;
	fd_set rdtmp = data->readSet;
	fd_set wrtmp = data->writeSet;
	int count = select(Max, &rdtmp, &wrtmp, NULL, &val);//timeout转化为毫秒 count为变化的个数
	if (count == -1) {
		perror("select");
		exit(0);
	}
	for (int i = 0; i < Max; ++i) {  //i是select中遍历时的文件描述符
		if (FD_ISSET(i, &rdtmp)) {
			eventActivate(evLoop, i, ReadEvent);
		}
		if (FD_ISSET(i, &wrtmp)) {
			eventActivate(evLoop, i, WriteEvent);
		}
	}
	return 0;
}

static int SelectClear(EventLoop* evLoop) {
	struct SelectData* data = (struct SelectData*)evLoop->dispatcherData;
	free(data);
	return 0;
}