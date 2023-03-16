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
static int epollAdd(Channel* channel, EventLoop* evLoop);//���
static int epollRemove(Channel* channel, EventLoop* evLoop);//ɾ��
static int epollModify(Channel* channel, EventLoop* evLoop);//�޸�
static int epollDispatch(EventLoop* evLoop, int timeout);//�¼���ⳬʱ timeout��λ��s
static int epollClear(EventLoop* evLoop);//������ݣ��ر�fd���ͷ��ڴ�
static int epollCtl(Channel* channel, EventLoop* evLoop, int op);



struct Dispatcher EpollDispatcher = {  //ȫ�ֽṹ�����
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
	ev.data.fd = channel->fd;//data��������
	int events = 0;
	if (channel->events & ReadEvent) {  //���Լ�����Ķ�д�¼���־ת��Ϊ��io����ϵͳ�ı�־
		events |= EPOLLIN;
	}
	if (channel->events & WriteEvent) {
		events |= EPOLLOUT;
	}
	ev.events = events;
	int ret = epoll_ctl(data->epfd, op, channel->fd, &ev);
	return ret;//ʧ�ܷ���-1
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
	//ͨ��channel�ͷŶ�Ӧ��tcpconnection��Դ
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
	int count = epoll_wait(data->epfd, data->events, Max, timeout * 1000);//timeoutת��Ϊ���� countΪ�仯�ĸ���
	for (int i = 0; i < count; ++i) {
		int events = data->events[i].events;
		int fd = data->events[i].data.fd;
		if (events & EPOLLERR || events & EPOLLHUP) {//EPOLLERR�Զ˶Ͽ����� EPOLLHUP�Է��Ͽ����Ӻ��ҷ�������Ϣ
			//�Է��Ͽ����ӣ����� ɾ��fd
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