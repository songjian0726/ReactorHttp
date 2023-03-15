#include "TcpServer.h"
#include"TcpConnection.h"
#include<arpa/inet.h>
struct TcpServer* tcpServerInit(unsigned short port, int threadNum)
{
	struct TcpServer* tcp = (struct TcpServer*)malloc(sizeof(struct TcpServer));
	tcp->listener = listenerInit(port);//�����
	tcp->mainLoop = eventLoopInit();
	tcp->threadNum = threadNum;
	tcp->threadPool = threadPoolInit(tcp->mainLoop, threadNum);
	
	
	return tcp;
}

struct Listener* listenerInit(unsigned short port)
{
	struct Listener* listener = (struct Listener*)malloc(sizeof(struct Listener));
	//1.����������fd
	int lfd = socket(AF_INET, SOCK_STREAM, 0);
	if (lfd == -1) {
		perror("socket");
		return -1;
	}
	//2.���ö˿ڸ���
	int opt = 1;//������Զ˿ڸ���
	int ret = setsockopt(lfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof opt);
	if (ret == -1) {  //����ʧ�ܷ���-1
		perror("socketopt");
		return -1;
	}
	//3.��
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = INADDR_ANY; //0 ��ʾ����ip  ���ָ��ip��Ҫת���ɴ�˸�ʽ
	ret = bind(lfd, (struct sockaddr*)&addr, sizeof addr);
	if (ret == -1) {
		perror("bind");
		return -1;
	}
	//4.���ü���
	ret = listen(lfd, 128); //128����
	if (ret == -1) {
		perror("listen");
		return -1;
	}
	//����listener
	listener->lfd = lfd;
	listener->port = port;
	return listener;
}

int acceptConnection(void* arg) {
	struct TcpServer* server = (struct TcpServer*)arg;
	//accept��ȡ
	int cfd = accept(server->listener->lfd, NULL, NULL);
	//���̳߳���ѡ��һ�����߳̽��մ���cfd
	struct EventLoop* evLoop = takeWorkerEventLoop(server->threadPool);
	//cfd�ŵ�TcpConnection��
	tcpConnectionInit(cfd, evLoop);
}

void tcpServerRun(struct TcpServer* server)
{
	//�����̳߳�
	threadPoolRun(server->threadPool);
	//��ʼ��һ��channelʵ��
	struct Channel* channel = channelInit(server->listener->lfd, ReadEvent, acceptConnection, NULL, server);//������
	//��Ӽ������
	eventLoopAddTask(server->mainLoop, channel, ADD); //��channel ADD��mainLoop��
	//������Ӧ��ģ��
	eventLoopRun(server->mainLoop);
}
