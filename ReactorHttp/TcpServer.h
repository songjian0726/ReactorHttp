#pragma once
#include"EventLoop.h"
#include"ThreadPool.h"
struct Listener {
	int lfd;
	unsigned short port;
};

struct TcpServer {
	int threadNum;
	struct EventLoop* mainLoop;
	struct ThreadPool* threadPool;
	struct Listener* listener;
};

//初始化TCP服务器实例
struct TcpServer* tcpServerInit(unsigned short port, int threadNum);

//初始化监听文件描述符
struct Listener* listenerInit(unsigned short port);

//启动TCP服务器
void tcpServerRun(struct TcpServer* server);