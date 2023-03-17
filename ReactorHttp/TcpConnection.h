#pragma once

#include"EventLoop.h"
#include"Buffer.h"
#include"Channel.h"
#include"HttpRequest.h"
#include"HttpResponse.h"

//#define MSG_SEND_AUTO  //控制两种发送方式 一种是全读到buf中一起发送 另一种是读一点发一点

struct TcpConnection {
	struct EventLoop* evLoop;
	struct Channel* channel;
	struct Buffer* readBuf;
	struct Buffer* writeBuf;
	char name[32];
	//http协议
	struct HttpRequest* request;
	struct HttpResponse* response;
};

//初始化
struct TcpConnection* tcpConnectionInit(int fd, struct EventLoop* evLoop);

//资源释放
int tcpConnectionDestroy(void* conn);