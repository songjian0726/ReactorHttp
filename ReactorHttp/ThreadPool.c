#include "ThreadPool.h"
#include<assert.h>

struct ThreadPool* threadPoolInit(struct EventLoop* mainLoop, int count)
{
	struct ThreadPool* pool = (struct ThreadPool*)malloc(sizeof(struct ThreadPool));
	pool->index = 0;
	pool->isStart = false;
	pool->mainLoop = mainLoop;
	pool->workerThreads = (struct WorkerThread*)malloc(sizeof(struct WorkerThread) * count);
	return pool;
}

void threadPoolRun(struct ThreadPool* pool)
{
	assert(pool && !pool->isStart);//如果此时线程池已经在运行
	if (pool->mainLoop->threadID != pthread_self()) { //如果执行这个函数的线程不是主线程
		exit(0);
	}
	pool->isStart = true;
	if (pool->threadNum > 0) {
		for (int i = 0; i < pool->threadNum; ++i) {
			workerThreadInit(&pool->workerThreads[i], i);
			workerThreadRun(&pool->workerThreads[i]);
		}
	}
}

struct EventLoop* takeWorkerEventLoop(struct ThreadPool* pool)
{
	assert(pool->isStart);
	if (pool->mainLoop->threadID != pthread_self()) { //如果执行这个函数的线程不是主线程
		exit(0);
	}
	//从线程池中找一个子进程，然后取出里面的反应堆实例
	struct EventLoop* evLoop = pool->mainLoop;
	if (pool->threadNum > 0) {
		evLoop = pool->workerThreads[pool->index].evLoop;
		pool->index = (++pool->index) % pool->threadNum;//循环index，将任务均匀分配给每一个子线程
	}
	return evLoop;
}
