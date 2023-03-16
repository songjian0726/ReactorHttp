#include "TcpConnection.h"
#include<stdio.h>
#include<stdlib.h>
#include"Log.h"

int processRead(void* arg) {
	struct TcpConnection* conn = (struct TcpConnection*)arg;
	//接收
	int count = bufferSocketRead(conn->readBuf, conn->channel->fd);
	Debug("接收到的http请求数据: %s", conn->readBuf->data + conn->readBuf->readPos);
	if (count > 0) {//成功接收
		//解析http
		int socket = conn->channel->fd;
#ifdef MSG_SEND_AUTO
		writeEventEnable(conn->channel, true);
		eventLoopAddTask(conn->evLoop, conn->channel, MODIFY);
#endif
		bool flag = parseHttpRequest(conn->request, conn->readBuf, conn->response, conn->writeBuf, socket);

		if (!flag) {
			//解析失败 回复一个简单的html
			char* errMsg = "HTTP/1.1 400 BAD Request\r\n\r\n";
			bufferAppendString(conn->writeBuf, errMsg);
		}
	}
	else {
		//断开连接 第一种发送方式中，此处不能断开连接
#ifdef MSG_SEND_AUTO
		eventLoopAddTask(conn->evLoop, conn->channel, DELETE);
#endif
	}
	//断开连接 第一种发送方式中，此处不能断开连接
#ifndef MSG_SEND_AUTO
	eventLoopAddTask(conn->evLoop, conn->channel, DELETE);
#endif
	return 0;
}

int processWrite(void* arg) {
	Debug("开始发送数据(基于写事件发送)...");
	struct TcpConnection* conn = (struct TcpConnection*)arg;
	//发送数据
	int count = bufferSendData(conn->writeBuf, conn->channel->fd);
	if (count > 0) {
		//判断数据是否被完全发送
		if (bufferReadableSize(conn->writeBuf) == 0) {
			//1.不再检测写事件 修改channel中保存的事件
			writeEventEnable(conn->channel, false);
			//2.修改dispatcher检测的集合
			eventLoopAddTask(conn->evLoop, conn->channel, MODIFY);
			//3.删除这个节点
			eventLoopAddTask(conn->evLoop, conn->channel, DELETE);
		}
	}
	return 0;
}

struct TcpConnection* tcpConnectionInit(int fd, struct EventLoop* evLoop)
{
	struct TcpConnection* conn = (struct TcpConnection*)malloc(sizeof(struct TcpConnection));
	conn->evLoop = evLoop;
	conn->readBuf = bufferInit(10240);//初始化10k
	conn->writeBuf = bufferInit(10240);
	//http
	conn->request = httpRequestInit();
	conn->response = httpResponseInit();
	sprintf(conn->name, "Connection-%d", fd);
	conn->channel = channelInit(fd, ReadEvent, processRead, processWrite, tcpConnectionDestroy, conn);
	eventLoopAddTask(evLoop, conn->channel, ADD);
	Debug("和客户端建立连接,threadName: %s, threadId: %ld", evLoop->threadName, evLoop->threadID);

	return conn;
}

int tcpConnectionDestroy(void* arg)
{
	struct TcpConnection* conn = (struct TcpConnection*)arg;
	if (conn != NULL) {
		if (conn->readBuf && bufferReadableSize(conn->readBuf) == 0 &&
			conn->writeBuf && bufferReadableSize(conn->writeBuf) == 0) {
			destroyChannel(conn->evLoop, conn->channel);
			bufferDestroy(conn->readBuf);
			bufferDestroy(conn->writeBuf);
			httpRequestDestroy(conn->request);
			httpResponseDestroy(conn->response);
			free(conn);
		}
	}
	Debug("断开连接，释放资源，connName: %s", conn->name);
	return 0;
}
