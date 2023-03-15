#pragma once
#include"Channel.h"
#include"EventLoop.h"
typedef struct Dispatcher
{
	//int
	void* (*init)();
	//���
	int (*add)(struct Channel* channel, struct EventLoop* evLoop);
	//ɾ��
	int (*remove)(struct Channel* channel, struct EventLoop* evLoop);
	//�޸�
	int (*modify)(struct Channel* channel, struct EventLoop* evLoop);
	//�¼����
	int (*dispatch)(struct EventLoop* evLoop, int timeout);//��ʱ timeout��λ��s
	//������ݣ��ر�fd���ͷ��ڴ�
	int (*clear)(struct EventLoop* evLoop);

}Dispatcher;