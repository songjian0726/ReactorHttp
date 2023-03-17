#pragma once
#include"Dispatcher.h"
#include"ChannelMap.h"
#include<pthread.h>
#include"TcpConnection.h"
#include<stdbool.h>

extern struct Dispatcher EpollDispatcher;//�����ⲿ��ȫ�ֱ���
extern struct Dispatcher PollDispatcher;
extern struct Dispatcher SelectDispatcher;

enum ElemType { ADD, DELETE, MODIFY };
//����������еĽڵ�
typedef struct ChannelElement{
	int type;  //��Channel�Ĵ�����
	struct Channel* channel;
	struct ChannelElement* next;
}ChannelElement;

struct Dispatcher;//ǰ������

typedef struct EventLoop{
	bool isQuit;
	struct Dispatcher* dispatcher;
	void* dispatcherData;//����˼�� �������ֲ�ͬ�Ľṹ

	//������� ����ʵ��
	struct ChannelElement* head;
	struct ChannelElement* tail;

	//map
	struct ChannelMap* channelMap;

	//�߳�id��name, mutex
	pthread_t threadID;
	char threadName[32];
	pthread_mutex_t mutex;//������
	int socketPair[2]; //���ڱ���ͨ�ŵ�socket fd

}EventLoop;


//��ʼ��
struct EventLoop* eventLoopInit();
struct EventLoop* eventLoopInitEx(const char* threadName);//���߳���Ҫһ������

//������Ӧ��ģ��
int eventLoopRun(struct EventLoop* evLoop);

//���������fd
int eventActivate(struct EventLoop* evLoop, int fd, int event);

//������������������
int eventLoopAddTask(struct EventLoop* evLoop, struct Channel* channel, int type);

//������������е�����
int eventLoopProcessTask(struct EventLoop* evLoop);

//����dispatcher�еĽڵ�
int eventLoopAdd(struct EventLoop* evLoop, struct Channel* channel);
int eventLoopRemove(struct EventLoop* evLoop, struct Channel* channel);
int eventLoopModify(struct EventLoop* evLoop, struct Channel* channel);

//�ͷ�channel
int destroyChannel(struct EventLoop* evLoop, struct Channel* channel);