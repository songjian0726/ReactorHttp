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
static int SelectAdd(Channel* channel, EventLoop* evLoop);//���
static int SelectRemove(Channel* channel, EventLoop* evLoop);//ɾ��
static int SelectModify(Channel* channel, EventLoop* evLoop);//�޸�
static int SelectDispatch(EventLoop* evLoop, int timeout);//�¼���ⳬʱ timeout��λ��s
static int SelectClear(EventLoop* evLoop);//������ݣ��ر�fd���ͷ��ڴ�
static void setFdSet(Channel* channel, struct SelectData* data);
static void clearFdSet(Channel* channel, struct SelectData* data);


struct Dispatcher SelectDispatcher = {  //ȫ�ֽṹ�����
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
	if (channel->events & ReadEvent) {  //���Լ�����Ķ�д�¼���־ת��Ϊ��io����ϵͳ�ı�־
		FD_SET(channel->fd, &data->readSet);
	}
	if (channel->events & WriteEvent) {
		FD_SET(channel->fd, &data->writeSet);
	}
}
static void clearFdSet(Channel* channel, struct SelectData* data) {
	if (channel->events & ReadEvent) {  //���Լ�����Ķ�д�¼���־ת��Ϊ��io����ϵͳ�ı�־
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
	return 0;//�ɹ�����0 ʧ�ܷ���-1
}

static int SelectRemove(Channel* channel, EventLoop* evLoop) {
	struct SelectData* data = (struct SelectData*)evLoop->dispatcherData;
	if (channel->fd >= Max) {
		return -1;
	}
	clearFdSet(channel, data);
	//ͨ��channel�ͷŶ�Ӧ��tcpconnection��Դ
	channel->destroyCallback(channel->arg);
	return 0;//�ɹ�����0 ʧ�ܷ���-1
}

static int SelectModify(Channel* channel, EventLoop* evLoop) {
	struct SelectData* data = (struct SelectData*)evLoop->dispatcherData;
	clearFdSet(channel, data);
	setFdSet(channel, data);
	return 0;//�ɹ�����0 ʧ�ܷ���-1
}

static int SelectDispatch(EventLoop* evLoop, int timeout) {
	struct SelectData* data = (struct SelectData*)evLoop->dispatcherData;
	struct timeval val;
	val.tv_sec = timeout;
	val.tv_usec = 0;
	fd_set rdtmp = data->readSet;
	fd_set wrtmp = data->writeSet;
	int count = select(Max, &rdtmp, &wrtmp, NULL, &val);//timeoutת��Ϊ���� countΪ�仯�ĸ���
	if (count == -1) {
		perror("select");
		exit(0);
	}
	for (int i = 0; i < Max; ++i) {  //i��select�б���ʱ���ļ�������
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