#pragma once
#include"Buffer.h"

//定义枚举状态码
enum HttpStatusCode {
	Unkown,//0
	OK = 200,
	MovePermanently = 301,
	MoveTemporarily = 302,
	BadRequest = 400,
	NotFound = 404
};

//定义相应头结构体 键值对
struct ResponseHeader
{
	char key[32];
	char value[128];
};

typedef void(*responseBody) (const char* fileName, struct Buffer* sendBuf, int socket );

//定义结构体
struct HttpResponse
{
	//状态行: 状态码，状态描述(404_NOT_FOUND)等，
	enum HttpStatusCode statusCode;
	char statusMsg[128];
	//响应头-键值对
	struct ResponseHeader* headers;
	int headerNum;//当前相应头中键值对数量
	responseBody sendDataFunc;
};

//初始化
struct HttpResponse* httpResponseInit();

//销毁
void httpResponseDestroy(struct HttpResponse* response);

//添加响应头 向响应中添加键值对
void httpResponseAddHeader(struct HttpResponse* response, const char* key, const char* value);