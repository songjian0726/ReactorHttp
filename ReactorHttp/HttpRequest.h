#pragma once
#include"Buffer.h"
#include"HttpResponse.h"
#include<stdbool.h>

//请求头键值对
struct RequestHeader {
	char* key;
	char* value;
};

//当前的解析状态
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
	struct RequestHeader* reqHeaders;//请求头中的键值对存储的数组
	int reqHeadersNum;//键值对数组中的数量
	enum HttpRequestState curState;//当前状态
};

//初始化httpRequest结构体
struct HttpRequest* httpRequestInit();

//重置httpRequest结构体
void httpRequestReset(struct HttpRequest* req);
void httpRequestResetEx(struct HttpRequest* req);

//销毁
void httpRequestDestroy(struct HttpRequest* req);

//获取处理状态State
enum HttpRequestState httpRequestState(struct HttpRequest* request);

//添加请求头
void httpRequestAddHeader(struct HttpRequest* request, const char* key, const char* value);

//根据key获得对应的value
char* httpRequestGetHeader(struct HttpRequest* request, const char* key);

//解析请求行
bool parseHttpRequestLine(struct HttpRequest* request, struct Buffer* readBuf);

//解析请求头
bool parseHttpRequestHeader(struct HttpRequest* request, struct Buffer* readBuf);

//解析http请求  解析是把http请求拆分
bool parseHttpRequest(struct HttpRequest* request, struct Buffer* readBuf, struct HttpResponse* response, struct Buffer* sendBuf, int socket);

//处理http请求 处理是根据request结构体里的数据做出相应反馈
bool processHttpRequest(struct HttpRequest* request, struct HttpResponse* response);

//解码字符串
void decodeMsg(char* to, char* from);


const char* getFileType(const char* name);

int sendFile(const char* fileName, struct Buffer* sendBuf, int cfd);

int sendDir(const char* dirName, struct Buffer* sendBuf, int cfd);


