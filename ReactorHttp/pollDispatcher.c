#include"Dispatcher.h"
#include<poll.h>
#include<stdio.h>
#include<stdlib.h>
#define Max (1024)

typedef struct PollData {
	int maxfd;
	struct pollfd fds[Max];
}PollData;

static void* pollInit();
static int pollAdd(Channel* channel, EventLoop* evLoop);//���
static int pollRemove(Channel* channel, EventLoop* evLoop);//ɾ��
static int pollModify(Channel* channel, EventLoop* evLoop);//�޸�
static int pollDispatch(EventLoop* evLoop, int timeout);//�¼���ⳬʱ timeout��λ��s
static int pollClear(EventLoop* evLoop);//������ݣ��ر�fd���ͷ��ڴ�



struct Dispatcher PollDispatcher = {  //ȫ�ֽṹ�����
	pollInit,
	pollAdd,
	pollRemove,
	pollModify,
	pollDispatch,
	pollClear
};

static void* pollInit() {
	struct PollData* data = (struct PollData*)malloc(sizeof(struct PollData));
	data->maxfd = 0;
	for (int i = 0; i < Max; ++i) {
		data->fds[i].fd = -1;
		data->fds[i].events = 0;
		data->fds[i].revents = 0;
	}
	return data;
}


static int pollAdd(Channel* channel, EventLoop* evLoop) {
	struct PollData* data = (struct PollData*)evLoop->dispatcherData;
	int events = 0;
	if (channel->events & ReadEvent) {  //���Լ�����Ķ�д�¼���־ת��Ϊ��io����ϵͳ�ı�־
		events |= POLLIN;
	}
	if (channel->events & WriteEvent) {
		events |= POLLOUT;
	}
	int i = 0;
	for (; i < Max; ++i) {
		if (data->fds[i].fd == -1) {
			data->fds[i].events = events;
			data->fds[i].fd = channel->fd;
			data->maxfd = data->maxfd < i ? i : data->maxfd;
			break;
		}
	}
	if (i >= Max) {
		return -1;
	}
	return 0;//�ɹ�����0 ʧ�ܷ���-1
}

static int pollRemove(Channel* channel, EventLoop* evLoop) {
	struct PollData* data = (struct PollData*)evLoop->dispatcherData;
	int i = 0;
	for (; i < Max; ++i) {
		if (data->fds[i].fd == channel->fd) {
			data->fds[i].fd = -1;
			data->fds[i].events = 0;
			data->fds[i].revents = 0;
			break;
		}
	}
	//ͨ��channel�ͷŶ�Ӧ��tcpconnection��Դ
	channel->destroyCallback(channel->arg);
	if (i >= Max) {
		return -1;
	}
	return 0;//�ɹ�����0 ʧ�ܷ���-1
}

static int pollModify(Channel* channel, EventLoop* evLoop) {
	struct PollData* data = (struct PollData*)evLoop->dispatcherData;
	int events = 0;
	if (channel->events & ReadEvent) {  //���Լ�����Ķ�д�¼���־ת��Ϊ��io����ϵͳ�ı�־
		events |= POLLIN;
	}
	if (channel->events & WriteEvent) {
		events |= POLLOUT;
	}
	int i = 0;
	for (; i < Max; ++i) {
		if (data->fds[i].fd == channel->fd) {
			data->fds[i].events = events;
			break;
		}
	}
	if (i >= Max) {
		return -1;
	}
	return 0;//�ɹ�����0 ʧ�ܷ���-1
}

static int pollDispatch(EventLoop* evLoop, int timeout) {
	struct PollData* data = (struct PollData*)evLoop->dispatcherData;
	int count = poll(data->fds, data->maxfd + 1, timeout * 1000);//timeoutת��Ϊ���� countΪ�仯�ĸ���
	if (count == -1) {
		perror("poll");
		exit(0);
	}
	for (int i = 0; i <= data->maxfd; ++i) {
		if (data->fds[i].fd == -1) {
			continue;
		}
		if (data->fds[i].events & POLLIN) {
			eventActivate(evLoop, data->fds[i].fd, ReadEvent);
		}
		if (data->fds[i].events & POLLOUT) {
			eventActivate(evLoop, data->fds[i].fd, WriteEvent);
		}
	}
	return 0;
}

static int pollClear(EventLoop* evLoop) {
	struct PollData* data = (struct PollData*)evLoop->dispatcherData;
	free(data);
	return 0;
}