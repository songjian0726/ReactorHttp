#pragma once
#include<stdbool.h>

//�ص������ĺ���ָ��
typedef int(*handleFund)(void* arg);

typedef struct Channel {
	//�ļ�������
	int fd;
	//��Ӧ���¼�
	int events;
	//�ص�����
	handleFund readCallback;
	handleFund writeCallback;
	//�ص���������
	void* arg;
}Channel;

//�����ļ��������Ķ�д�¼�
enum FDEvent {
	TimeOut = 0x01,
	ReadEvent = 0x02,
	WriteEvent = 0x04
};

//��ʼ��һ��Channel
struct Channel* channelInit(int fd, int events, handleFund readFunc, handleFund writeFunc, void* arg);

//�޸�fd��д�¼�(���򲻼��)
void writeEventEnable(struct Channel* channel, bool flag);

//�ж��Ƿ���Ҫ����ļ���������д�¼�
bool isWriteEventEnable(struct Channel* channel);