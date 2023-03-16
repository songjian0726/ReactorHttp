#pragma once
#include"Buffer.h"
#include<stdbool.h>

//����ͷ��ֵ��
struct RequestHeader {
	char* key;
	char* value;
};

//��ǰ�Ľ���״̬
enum HttpRequestState {
	ParseReqLine,
	ParseReqHeaders,
	ParseReqBody,
	ParseReqDone
};

struct HttpRequest {
	char* method;
	char* url;
	char* version;
	struct RequestHeader* reqHeaders;//����ͷ�еļ�ֵ�Դ洢������
	int reqHeadersNum;//��ֵ�������е�����
	enum HttpRequestState curState;//��ǰ״̬
};

//��ʼ��httpRequest�ṹ��
struct HttpRequest* httpRequestInit();

//����httpRequest�ṹ��
void httpRequestReset(struct HttpRequest* req);
void httpRequestResetEx(struct HttpRequest* req);

//����
void httpRequestDestroy(struct HttpRequest* req);

//��ȡ����״̬State
enum HttpRequestState httpRequestState(struct HttpRequest* request);

//�������ͷ
void httpRequestAddHeader(struct HttpRequest* request, const char* key, const char* value);

//����key��ö�Ӧ��value
char* httpRequestGetHeader(struct HttpRequest* request, const char* key);

//����������
bool parseHttpRequestLine(struct HttpRequest* request, struct Buffer* readBuf);

//��������ͷ
bool parseHttpRequestHeader(struct HttpRequest* request, struct Buffer* readBuf);

//����http����  �����ǰ�http������
bool parseHttpRequest(struct HttpRequest* request, struct Buffer* readBuf);

//����http���� �����Ǹ���request�ṹ���������������Ӧ����
bool processHttpRequest(struct HttpRequest* request);

//�����ַ���
void decodeMsg(char* to, char* from);