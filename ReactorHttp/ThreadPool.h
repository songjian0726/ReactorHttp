#pragma once
#include"WorkerThread.h"
#include"EventLoop.h"
#include<stdbool.h>
struct ThreadPool {
	//���̵߳ķ�Ӧ��
	struct EventLoop* mainLoop;
	bool isStart;
	int threadNum;
	struct WorkerThread* workerThreads;
	int index;
};

//��ʼ���̳߳�
struct ThreadPool* threadPoolInit(struct EventLoop* mainLoop, int count);

//�����̳߳�
void threadPoolRun(struct ThreadPool* pool);

//ȡ���̳߳���ĳ�����̵߳ķ�Ӧ��ʵ��
struct EventLoop* takeWorkerEventLoop(struct ThreadPool* pool);