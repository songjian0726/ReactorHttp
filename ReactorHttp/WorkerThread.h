#pragma once
#include<pthread.h>
#include"EventLoop.h"
typedef struct WorkerThread {
	pthread_t threadID;//线程ID
	char name[24];
	pthread_mutex_t mutex;//互斥锁
	pthread_cond_t cond;//条件变量
	struct EventLoop* evLoop;
}WorkerThread;


//初始化
int workerThreadInit(struct WorkerThread* thread, int index);

void workerThreadRun(struct WorkerThread* thread);