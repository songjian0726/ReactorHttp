#include "HttpRequest.h"
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<strings.h>
#include<sys/stat.h>
#include<assert.h>

#define HeaderSize (12)
struct HttpRequest* httpRequestInit()
{
    struct HttpRequest* request = (struct HttpRequest*)malloc(sizeof(struct HttpRequest));
    httpRequestReset(request);
    request->reqHeaders = (struct HttpRequest*)malloc(sizeof(struct HttpRequest) * HeaderSize);
    return request;
}

void httpRequestReset(struct HttpRequest* req)
{
    req->curState = ParseReqLine;
    req->method = NULL;
    req->url = NULL;
    req->version = NULL;
    req->reqHeadersNum = 0;
}

void httpRequestResetEx(struct HttpRequest* req)
{
    free(req->url);
    free(req->method);
    free(req->version);
    if (req->reqHeaders != NULL) {
        for (int i = 0; i < req->reqHeadersNum; ++i) {
            free(req->reqHeaders[i].key);
            free(req->reqHeaders[i].value);
        }
        free(req->reqHeaders);
    }
    httpRequestReset(req);
}

void httpRequestDestroy(struct HttpRequest* req)
{
    if (req != NULL) {
        httpRequestResetEx(req);
        free(req);
    }
}

enum HttpRequestState httpRequestState(struct HttpRequest* request)
{
    return request->curState;
}

void httpRequestAddHeader(struct HttpRequest* request, const char* key, const char* value)
{
    request->reqHeaders[request->reqHeadersNum].key = key;
    request->reqHeaders[request->reqHeadersNum].value = value;
    request->reqHeadersNum++;
}

char* httpRequestGetHeader(struct HttpRequest* request, const char* key)
{
    if (request != NULL) {
        for (int i = 0; i < request->reqHeadersNum; ++i) {
            if (strncasecmp(request->reqHeaders[i].key, key, strlen(key)) == 0) {
                return request->reqHeaders[i].value;
            }
        }
    }
    return NULL;
}

char* splitRequestLine(const char* start, const char* end, const char* sub, char** ptr) {//sub��Ҫ�������ַ���
    char* space = end;
    if (sub != NULL) { //�������һ��ʱ��sub����NULL
        char* space = memmem(start, end - start, sub, sizeof(sub));//�ҵ��ո��λ�ã���һ���ո�ǰ��GET/POSE
        assert(space != NULL);
    }
    int length = space - start;
    char* tmp = (char*)malloc(length + 1);
    strncpy(tmp, start, length);
    tmp[length] = '\0';
    *ptr = tmp;
    return space + 1;
}

bool parseHttpRequestLine(struct HttpRequest* request, struct Buffer* readBuf)
{
    //���������У������ַ���������ַ
    char* end = bufferFindCRLF(readBuf);
    //�����ַ�����ʼ��ַ
    char* start = readBuf->data + readBuf->readPos;
    //�������ܳ���
    int lineSize = end - start;

    if (lineSize) {
        start = splitRequestLine(start, end, " ", &request->method);//����ʽ  ���һ��������ָ��ĵ�ַ ��������
        start = splitRequestLine(start, end, " ", &request->url);//·��
        splitRequestLine(start, end, NULL, &request->version);//�汾
#if 0
        //����ʽ GET��
        char* space = memmem(start, lineSize, " ", 1);//�ҵ��ո��λ�ã���һ���ո�ǰ��GET/POSE
        assert(space != NULL);
        int methodSize = space - start;
        request->method = (char*)malloc(methodSize + 1);
        strncpy(request->method, start, methodSize);
        request->method[methodSize] = '\0';

        //����ľ�̬��Դ ·���� ��һ���ո�͵ڶ����ո�֮��
        start = space + 1;//�ӵ�һ���ո��ʼ
        char* space = memmem(start, end - start, " ", 1);//ʣ����Ҫ���Ŀռ�Ϊend - start
        assert(space != NULL);
        int urlSize = space - start;
        request->url = (char*)malloc(urlSize + 1);
        strncpy(request->url, start, urlSize);
        request->url[urlSize] = '\0';

        //�����HTTP�汾
        start = space + 1;//�ӵڶ����ո��ʼ ��end����
        //char* space = memmem(start, end - start, " ", 1);//ʣ����Ҫ���Ŀռ�Ϊend - start
        //assert(space != NULL);
        //int urlSize = space - start;
        request->version = (char*)malloc(end - start + 1);
        strncpy(request->version, start, end - start);
        request->version[end - start] = '\0';
#endif

        //׼����������ͷ
        readBuf->readPos = readBuf->readPos + lineSize + 2;//�Ѿ�����lineSize���ֽڣ���readPos���� ���滹��\r\n �ٺ���2�ֽ�
        //�޸ĵ�ǰ�Ľ���״̬
        request->curState = ParseReqHeaders;
        return true;
    }

    return false;
}

//��������ͷ�е�һ��
bool parseHttpRequestHeader(struct HttpRequest* request, struct Buffer* readBuf)
{
    char* end = bufferFindCRLF(readBuf);
    if (end != NULL) {
        char* start = readBuf->data + readBuf->readPos;
        int lineSize = end - start;
        //����:  �ָ��ֵ��
        char* middle = memmem(start, lineSize, ": ", 2);//������еı�׼http������ :������һ���ո�
        if (middle != NULL) {
            char* key = malloc(middle - start + 1);//+1Ϊ�˴洢�ַ�������\0
            strncpy(key, start, middle - start);
            key[middle - start] = '\0';

            char* value = malloc(end - middle - 2 + 1);//-2 ��": "
            strncpy(value, middle + 2, end - middle - 2);
            value[end - middle - 2] = '\0';
            httpRequestAddHeader(request, key, value);

            //����readPos����һ�е�λ��
            readBuf->readPos += lineSize;
            readBuf->readPos += 2;
        }
        else {//���middle == NULL ����ͷ�Ѿ��������� ������һ������
            //���ƶ�ָ���������� �����������ַ� "\r\n"
            readBuf->readPos += 2;
            //�޸Ľ���״̬ ֻ����GET����ʱ������Ҫ���յ��Ĳ���
            request->curState = ParseReqDone;
        }

        return true;
    }
    return false;
}

bool parseHttpRequest(struct HttpRequest* request, struct Buffer* readBuf)
{
    bool flag = true;
    while (request->curState != ParseReqDone) {
        switch (request->curState)
        {
        case ParseReqLine:
            flag = parseHttpRequestLine(request, readBuf);
            break;
        case ParseReqHeaders:
            flag = parseHttpRequestHeader(request, readBuf);
            break;
        case ParseReqBody://����POST����ʱ��Ҫ�����ⲿ�� �˴��ݲ�����
            break;
        default:
            break;
        }
        if (!flag) {
            return flag;
        }
        if (request->curState == ParseReqDone) { //������Ϻ� ׼���ظ�����
            //1.���ݽ����������ݶԿͻ��˵�������������

            //2.��֯��Ӧ���ݲ�����
        }
    }

    request->curState = ParseReqLine;//��ԭ����״̬�����ڴ����������
    return flag;
}

//����request
bool processHttpRequest(struct HttpRequest* request)
{
    // ���������� get /xxx/1.jpg http/1.1
    if (strcasecmp(request->method, "get") != 0)
    {
        return -1;
    }
    decodeMsg(request->url, request->url);
    // ����ͻ�������ľ�̬��Դ(Ŀ¼�����ļ�)
    char* file = NULL;
    if (strcmp(request->url, "/") == 0)
    {
        file = "./";
    }
    else
    {
        file = request->url + 1;
    }
    // ��ȡ�ļ�����
    struct stat st;
    int ret = stat(file, &st);
    if (ret == -1)
    {
        // �ļ������� -- �ظ�404

        //sendHeadMsg(cfd, 404, "Not Found", getFileType(".html"), -1);
        //sendFile("404.html", cfd);
        return 0;
    }
    // �ж��ļ�����
    if (S_ISDIR(st.st_mode))
    {
        // �����Ŀ¼�е����ݷ��͸��ͻ���
        //sendHeadMsg(cfd, 200, "OK", getFileType(".html"), -1);
        //sendDir(file, cfd);
    }
    else
    {
        // ���ļ������ݷ��͸��ͻ���
        //sendHeadMsg(cfd, 200, "OK", getFileType(file), st.st_size);
        //sendFile(file, cfd);
    }
    return false;
}

// ���ַ�ת��Ϊ������
int hexToDec(char c)
{
    if (c >= '0' && c <= '9')
        return c - '0';
    if (c >= 'a' && c <= 'f')
        return c - 'a' + 10;
    if (c >= 'A' && c <= 'F')
        return c - 'A' + 10;

    return 0;
}

// ����
// to �洢����֮�������, ��������, from�����������, �������
void decodeMsg(char* to, char* from)
{
    for (; *from != '\0'; ++to, ++from)
    {
        // isxdigit -> �ж��ַ��ǲ���16���Ƹ�ʽ, ȡֵ�� 0-f
        // Linux%E5%86%85%E6%A0%B8.jpg
        if (from[0] == '%' && isxdigit(from[1]) && isxdigit(from[2]))
        {
            // ��16���Ƶ��� -> ʮ���� �������ֵ��ֵ�����ַ� int -> char
            // B2 == 178
            // ��3���ַ�, �����һ���ַ�, ����ַ�����ԭʼ����
            *to = hexToDec(from[1]) * 16 + hexToDec(from[2]);

            // ���� from[1] �� from[2] ����ڵ�ǰѭ�����Ѿ��������
            from += 2;
        }
        else
        {
            // �ַ�����, ��ֵ
            *to = *from;
        }
    }
    *to = '\0';
}
