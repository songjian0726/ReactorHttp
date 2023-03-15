#pragma once

//请求头键值对
struct RequestHeader {
	char* key;
	char* value;
};

struct HttpRequest {
	char* method;
	char* url;
	char* version;
	struct RequestHeader* reqHeaders;//请求头中的键值对们
	int reqHeadersNum;//键值对数组中的数量
};