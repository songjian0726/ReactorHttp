#include "TcpConnection.h"
#include<stdio.h>
#include<stdlib.h>
#include"Log.h"

int processRead(void* arg) {
	struct TcpConnection* conn = (struct TcpConnection*)arg;
	//����
	int count = bufferSocketRead(conn->readBuf, conn->channel->fd);
	Debug("���յ���http��������: %s", conn->readBuf->data + conn->readBuf->readPos);
	if (count > 0) {//�ɹ�����
		//����http
		int socket = conn->channel->fd;
#ifdef MSG_SEND_AUTO
		writeEventEnable(conn->channel, true);
		eventLoopAddTask(conn->evLoop, conn->channel, MODIFY);
#endif
		bool flag = parseHttpRequest(conn->request, conn->readBuf, conn->response, conn->writeBuf, socket);

		if (!flag) {
			//����ʧ�� �ظ�һ���򵥵�html
			char* errMsg = "HTTP/1.1 400 BAD Request\r\n\r\n";
			bufferAppendString(conn->writeBuf, errMsg);
		}
	}
	else {
		//�Ͽ����� ��һ�ַ��ͷ�ʽ�У��˴����ܶϿ�����
#ifdef MSG_SEND_AUTO
		eventLoopAddTask(conn->evLoop, conn->channel, DELETE);
#endif
	}
	//�Ͽ����� ��һ�ַ��ͷ�ʽ�У��˴����ܶϿ�����
#ifndef MSG_SEND_AUTO
	eventLoopAddTask(conn->evLoop, conn->channel, DELETE);
#endif
	return 0;
}

int processWrite(void* arg) {
	Debug("��ʼ��������(����д�¼�����)...");
	struct TcpConnection* conn = (struct TcpConnection*)arg;
	//��������
	int count = bufferSendData(conn->writeBuf, conn->channel->fd);
	if (count > 0) {
		//�ж������Ƿ���ȫ����
		if (bufferReadableSize(conn->writeBuf) == 0) {
			//1.���ټ��д�¼� �޸�channel�б�����¼�
			writeEventEnable(conn->channel, false);
			//2.�޸�dispatcher���ļ���
			eventLoopAddTask(conn->evLoop, conn->channel, MODIFY);
			//3.ɾ������ڵ�
			eventLoopAddTask(conn->evLoop, conn->channel, DELETE);
		}
	}
	return 0;
}

struct TcpConnection* tcpConnectionInit(int fd, struct EventLoop* evLoop)
{
	struct TcpConnection* conn = (struct TcpConnection*)malloc(sizeof(struct TcpConnection));
	conn->evLoop = evLoop;
	conn->readBuf = bufferInit(10240);//��ʼ��10k
	conn->writeBuf = bufferInit(10240);
	//http
	conn->request = httpRequestInit();
	conn->response = httpResponseInit();
	sprintf(conn->name, "Connection-%d", fd);
	conn->channel = channelInit(fd, ReadEvent, processRead, processWrite, tcpConnectionDestroy, conn);
	eventLoopAddTask(evLoop, conn->channel, ADD);
	Debug("�Ϳͻ��˽�������,threadName: %s, threadId: %ld", evLoop->threadName, evLoop->threadID);

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
	Debug("�Ͽ����ӣ��ͷ���Դ��connName: %s", conn->name);
	return 0;
}
