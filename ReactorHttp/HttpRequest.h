#pragma once

//����ͷ��ֵ��
struct RequestHeader {
	char* key;
	char* value;
};

struct HttpRequest {
	char* method;
	char* url;
	char* version;
	struct RequestHeader* reqHeaders;//����ͷ�еļ�ֵ����
	int reqHeadersNum;//��ֵ�������е�����
};