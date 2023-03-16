#include "HttpResponse.h"
#include<strings.h>
#include<string.h>
#include<stdlib.h>
#define ResHeaderSize (16)
struct HttpResponse* httpResponseInit()
{
	struct HttpResponse* response = (struct HttpResponse*)malloc(sizeof(struct HttpResponse));
	response->headerNum = 0;
	int size = sizeof(struct ResponseHeader) * ResHeaderSize;
	response->headers = (struct ResponseHeader*)malloc(size);
	response->statusCode = Unkown;
	//初始化
	bzero(response->headers, size);
	bzero(response->statusMsg, sizeof(response->statusMsg));
	//函数指针
	response->sendDataFunc = NULL;
	return response;
}

void httpResponseDestroy(struct HttpResponse* response)
{
	if (response != NULL) {
		free(response->headers);
		free(response);
	}
}

void httpResponseAddHeader(struct HttpResponse* response, const char* key, const char* value)
{
	if (response == NULL || key == NULL || value == NULL) {
		return;
	}
	strcpy(response->headers[response->headerNum].key, key);
	strcpy(response->headers[response->headerNum].value, value);
	response->headerNum++;

}
