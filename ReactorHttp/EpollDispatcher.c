#include"Dispatcher.h"
#include<sys/epoll.h>
#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>

#define Max (520)

typedef struct EpollData {
	int epfd;
	struct epoll_event* events;
}EpollData;

static void* epollInit();
static int epollAdd(Channel* channel, EventLoop* evLoop);//添加
static int epollRemove(Channel* channel, EventLoop* evLoop);//删除
static int epollModify(Channel* channel, EventLoop* evLoop);//修改
static int epollDispatch(EventLoop* evLoop, int timeout);//事件检测超时 timeout单位：s
static int epollClear(EventLoop* evLoop);//清除数据，关闭fd或释放内存
static int epollCtl(Channel* channel, EventLoop* evLoop, int op);



struct Dispatcher EpollDispatcher = {  //全局结构体变量
	epollInit,
	epollAdd,
	epollRemove,
	epollModify,
	epollDispatch,
	epollClear
};

static void* epollInit(){
	struct EpollData* data = (struct EpollData*)malloc(sizeof(struct EpollData));
	data->epfd = epoll_create(1);
	if (data->epfd == -1) {
		perror("epoll_create");
		exit(0);
	}
	data->events = (struct epoll_event*)calloc(Max, sizeof(struct epoll_event));
	return data;
}

static int epollCtl(Channel* channel, EventLoop* evLoop, int op) {
	struct EpollData* data = (struct EpollData*)evLoop->dispatcherData;
	struct epoll_event ev;
	ev.data.fd = channel->fd;//data是联合体
	int events = 0;
	if (channel->events & ReadEvent) {  //将自己定义的读写事件标志转化为该io复用系统的标志
		events |= EPOLLIN;
	}
	if (channel->events & WriteEvent) {
		events |= EPOLLOUT;
	}
	ev.events = events;
	int ret = epoll_ctl(data->epfd, op, channel->fd, &ev);
	return ret;//失败返回-1
}

static int epollAdd(Channel* channel, EventLoop* evLoop){
	int ret = epollCtl(channel, evLoop, EPOLL_CTL_ADD);
	if (ret == -1) {
		perror("epoll_add");
		exit(0);
	}
	return ret;
}

static int epollRemove(Channel* channel, EventLoop* evLoop){
	int ret = epollCtl(channel, evLoop, EPOLL_CTL_DEL);
	if (ret == -1) {
		perror("epoll_delete");
		exit(0);
	}
	//通过channel释放对应的tcpconnection资源
	channel->destroyCallback(channel->arg);
	return ret;
}

static int epollModify(Channel* channel, EventLoop* evLoop){
	int ret = epollCtl(channel, evLoop, EPOLL_CTL_MOD);
	if (ret == -1) {
		perror("epoll_modify");
		exit(0);
	}
	return ret;
}

static int epollDispatch(EventLoop* evLoop, int timeout){
	struct EpollData* data = (struct EpollData*)evLoop->dispatcherData;
	int count = epoll_wait(data->epfd, data->events, Max, timeout * 1000);//timeout转化为毫秒 count为变化的个数
	for (int i = 0; i < count; ++i) {
		int events = data->events[i].events;
		int fd = data->events[i].data.fd;
		if (events & EPOLLERR || events & EPOLLHUP) {//EPOLLERR对端断开连接 EPOLLHUP对方断开连接后我方发送信息
			//对方断开连接，下树 删除fd
			//epollRemove(channel, evLoop);
			continue;
		}
		if (events & EPOLLIN) {
			eventActivate(evLoop, fd, ReadEvent);
		}
		if (events & EPOLLOUT) {
			eventActivate(evLoop, fd, WriteEvent);
		}
	}

}

static int epollClear(EventLoop* evLoop){
	struct EpollData* data = (struct EpollData*)evLoop->dispatcherData;
	free(data->events);
	close(data->epfd);
	free(data);
	return 0;
}