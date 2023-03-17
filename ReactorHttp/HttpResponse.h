#pragma once
#include"Buffer.h"

//����ö��״̬��
enum HttpStatusCode {
	Unknown,//0
	OK = 200,
	MovedPermanently = 301,
	MovedTemporarily = 302,
	BadRequest = 400,
	NotFound = 404
};

//������Ӧͷ�ṹ�� ��ֵ��
struct ResponseHeader
{
	char key[32];
	char value[128];
};

typedef void(*responseBody) (const char* fileName, struct Buffer* sendBuf, int socket );

//����ṹ��
struct HttpResponse
{
	//״̬��: ״̬�룬״̬����(404_NOT_FOUND)�ȣ�
	enum HttpStatusCode statusCode;
	char statusMsg[128];
	char fileName[128];
	//��Ӧͷ-��ֵ��
	struct ResponseHeader* headers;
	int headerNum;//��ǰ��Ӧͷ�м�ֵ������
	responseBody sendDataFunc;
};

//��ʼ��
struct HttpResponse* httpResponseInit();

//����
void httpResponseDestroy(struct HttpResponse* response);

//�����Ӧͷ ����Ӧ����Ӽ�ֵ��
void httpResponseAddHeader(struct HttpResponse* response, const char* key, const char* value);

//��֯http��Ӧ��Ϣ
void httpResponsePrepareMsg(struct HttpResponse* response, struct Buffer* sendBuf, int socket);