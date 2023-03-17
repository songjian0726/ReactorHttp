#include "TcpServer.h"
#include"TcpConnection.h"
#include<arpa/inet.h>
#include<stdio.h>
#include<stdlib.h>
#include"Log.h"

struct TcpServer* tcpServerInit(unsigned short port, int threadNum)
{
	Debug("进入到tcpserver初始化函数");
	struct TcpServer* tcp = (struct TcpServer*)malloc(sizeof(struct TcpServer));
	tcp->listener = listenerInit(port);//待填充
	Debug("listener inited");
	tcp->mainLoop = eventLoopInit();
	Debug("evloop inited");
	tcp->threadNum = threadNum;
	tcp->threadPool = threadPoolInit(tcp->mainLoop, threadNum);
	Debug("threadPool inited");

	
	return tcp;
}

struct Listener* listenerInit(unsigned short port)
{
	struct Listener* listener = (struct Listener*)malloc(sizeof(struct Listener));
	//1.创建监听的fd
	int lfd = socket(AF_INET, SOCK_STREAM, 0);
	if (lfd == -1) {
		perror("socket");
		return NULL;
	}
	//2.设置端口复用
	int opt = 1;//代表可以端口复用
	int ret = setsockopt(lfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof opt);
	if (ret == -1) {  //设置失败返回-1
		perror("setsockopt");
		return NULL;
	}
	//3.绑定
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = INADDR_ANY; //0 表示任意ip  如果指定ip需要转换成大端格式
	ret = bind(lfd, (struct sockaddr*)&addr, sizeof addr);
	if (ret == -1) {
		perror("bind");
		return NULL;
	}
	//4.设置监听
	ret = listen(lfd, 128); //128即可
	if (ret == -1) {
		perror("listen");
		return NULL;
	}
	//返回listener
	listener->lfd = lfd;
	listener->port = port;
	return listener;
}

int acceptConnection(void* arg) {
	struct TcpServer* server = (struct TcpServer*)arg;
	//accept提取
	int cfd = accept(server->listener->lfd, NULL, NULL);
	//从线程池中选出一个子线程接收处理cfd
	struct EventLoop* evLoop = takeWorkerEventLoop(server->threadPool);
	//cfd放到TcpConnection中
	tcpConnectionInit(cfd, evLoop);
	return 0;
}

void tcpServerRun(struct TcpServer* server)
{
	Debug("服务器程序已经启动......");
	//启动线程池
	threadPoolRun(server->threadPool);
	Debug("threadPoolRun");

	//初始化一个channel实例
	struct Channel* channel = channelInit(server->listener->lfd, 
		ReadEvent, acceptConnection, NULL, NULL, server);//待补充
	Debug("channelInit server.c83");

	//添加检测任务
	eventLoopAddTask(server->mainLoop, channel, ADD); //将channel ADD到mainLoop中
	Debug("addTask server.c87");

	//启动反应堆模型
	eventLoopRun(server->mainLoop);
	Debug("LoopRun server.c91");

}
