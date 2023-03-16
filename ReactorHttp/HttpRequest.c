#define _GNU_SOURCE


#include "HttpRequest.h"
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<strings.h>
#include<sys/stat.h>
#include<assert.h>
#include<dirent.h>
#include<fcntl.h>
#include<unistd.h>
#include<ctype.h>
#include"Log.h"

#define HeaderSize (12)
struct HttpRequest* httpRequestInit()
{
    struct HttpRequest* request = (struct HttpRequest*)malloc(sizeof(struct HttpRequest));
    httpRequestReset(request);
    Debug("httpRequest 19");
    request->reqHeaders = (struct HttpRequest*)malloc(sizeof(struct HttpRequest) * HeaderSize);
    Debug("httpRequest 21");

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
    request->reqHeaders[request->reqHeadersNum].key = (char*)key;
    request->reqHeaders[request->reqHeadersNum].value = (char*)value;
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

char* splitRequestLine(const char* start, const char* end, const char* sub, char** ptr) {
    char* space = end;
    Debug("%s", start);
    if (sub != NULL)
    {
        Debug("request.c 87");
        space = memmem(start, end - start, sub, strlen(sub));
        Debug("%s", space);
        assert(space != NULL);
        Debug("request.c 90");

    }
    int length = space - start;
    Debug("request.c 94");

    char* tmp = (char*)malloc(length + 1);
    Debug("request.c 97");

    strncpy(tmp, start, length);
    tmp[length] = '\0';
    Debug("request.c 101");

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
    Debug("%s  %d", start, lineSize);

    if (lineSize) {
        Debug("request.c 116");
        start = splitRequestLine(start, end, " ", &request->method);//����ʽ  ���һ��������ָ��ĵ�ַ ��������
        Debug("request.c 117");
        start = splitRequestLine(start, end, " ", &request->url);//·��
        Debug("request.c 119");
        splitRequestLine(start, end, NULL, &request->version);//�汾
        Debug("requset.c 121");
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

bool parseHttpRequest(struct HttpRequest* request, struct Buffer* readBuf, struct HttpResponse* response, struct Buffer* sendBuf, int socket)
{
    Debug("%s", readBuf->data);
    bool flag = true;
    while (request->curState != ParseReqDone) {
        switch (request->curState)
        {
        case ParseReqLine:
            flag = parseHttpRequestLine(request, readBuf);
            Debug("line down");
            break;
        case ParseReqHeaders:
            flag = parseHttpRequestHeader(request, readBuf);
            Debug("header down");
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
            processHttpRequest(request, response);
            //2.��֯��Ӧ���ݲ�����
            httpResponsePrepareMsg(response, sendBuf, socket);
        }
    }

    request->curState = ParseReqLine;//��ԭ����״̬�����ڴ����������
    return flag;
}

//����request
bool processHttpRequest(struct HttpRequest* request, struct HttpResponse* response)
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
        strcpy(response->fileName, "404.html");
        response->statusCode = NotFound;
        strcpy(response->statusMsg, "Not Found");
        //��Ӧͷ
        httpResponseAddHeader(response, "Content-type", getFileType(".html"));
        response->sendDataFunc = sendFile;
        return 0;
    }
    strcpy(response->fileName, file);
    response->statusCode = OK;
    strcpy(response->statusMsg, "OK");
    // �ж��ļ�����
    if (S_ISDIR(st.st_mode)) //Ŀ¼
    {
        // �����Ŀ¼�е����ݷ��͸��ͻ���
        //sendHeadMsg(cfd, 200, "OK", getFileType(".html"), -1);
        //sendDir(file, cfd);
        //��Ӧͷ
        httpResponseAddHeader(response, "Content-type", getFileType(".html"));
        response->sendDataFunc = sendDir;
        return 0;
    }
    else
    {
        // ���ļ������ݷ��͸��ͻ���
        //sendHeadMsg(cfd, 200, "OK", getFileType(file), st.st_size);
        //sendFile(file, cfd);
        //��Ӧͷ
        char tmp[12] = { 0 };
        sprintf(tmp, "%ld", st.st_size);
        httpResponseAddHeader(response, "Content-type", getFileType(file));
        httpResponseAddHeader(response, "Content-length", tmp);
        response->sendDataFunc = sendFile;
        return 0;
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


const char* getFileType(const char* name)
{
    // a.jpg a.mp4 a.html
    // ����������ҡ�.���ַ�, �粻���ڷ���NULL
    const char* dot = strrchr(name, '.');
    if (dot == NULL)
        return "text/plain; charset=utf-8";	// ���ı�
    if (strcmp(dot, ".html") == 0 || strcmp(dot, ".htm") == 0)
        return "text/html; charset=utf-8";
    if (strcmp(dot, ".jpg") == 0 || strcmp(dot, ".jpeg") == 0)
        return "image/jpeg";
    if (strcmp(dot, ".gif") == 0)
        return "image/gif";
    if (strcmp(dot, ".png") == 0)
        return "image/png";
    if (strcmp(dot, ".css") == 0)
        return "text/css";
    if (strcmp(dot, ".au") == 0)
        return "audio/basic";
    if (strcmp(dot, ".wav") == 0)
        return "audio/wav";
    if (strcmp(dot, ".avi") == 0)
        return "video/x-msvideo";
    if (strcmp(dot, ".mov") == 0 || strcmp(dot, ".qt") == 0)
        return "video/quicktime";
    if (strcmp(dot, ".mpeg") == 0 || strcmp(dot, ".mpe") == 0)
        return "video/mpeg";
    if (strcmp(dot, ".vrml") == 0 || strcmp(dot, ".wrl") == 0)
        return "model/vrml";
    if (strcmp(dot, ".midi") == 0 || strcmp(dot, ".mid") == 0)
        return "audio/midi";
    if (strcmp(dot, ".mp3") == 0)
        return "audio/mpeg";
    if (strcmp(dot, ".ogg") == 0)
        return "application/ogg";
    if (strcmp(dot, ".pac") == 0)
        return "application/x-ns-proxy-autoconfig";

    return "text/plain; charset=utf-8";
}


int sendDir(const char* dirName, struct Buffer* sendBuf, int cfd)
{
    char buf[4096] = { 0 };
    sprintf(buf, "<html><head><title>%s</title></head><body><table>", dirName);
    struct dirent** namelist;//#include<dirent.h>
    int num = scandir(dirName, &namelist, NULL, alphasort);
    for (int i = 0; i < num; ++i)
    {
        // ȡ���ļ��� namelist ָ�����һ��ָ������ struct dirent* tmp[]
        char* name = namelist[i]->d_name;
        struct stat st;
        char subPath[1024] = { 0 };
        sprintf(subPath, "%s/%s", dirName, name);
        stat(subPath, &st);
        if (S_ISDIR(st.st_mode))
        {
            // a��ǩ <a href="">name</a>
            sprintf(buf + strlen(buf),
                "<tr><td><a href=\"%s/\">%s</a></td><td>%ld</td></tr>",
                name, name, st.st_size);
        }
        else
        {
            sprintf(buf + strlen(buf),
                "<tr><td><a href=\"%s\">%s</a></td><td>%ld</td></tr>",
                name, name, st.st_size);
        }
        //send(cfd, buf, strlen(buf), 0);
        bufferAppendString(sendBuf, buf);
#ifndef MSG_SEND_AUTO
        bufferSendData(sendBuf, cfd);
#endif
        memset(buf, 0, sizeof(buf));
        free(namelist[i]);
    }
    sprintf(buf, "</table></body></html>");
    //send(cfd, buf, strlen(buf), 0);
    bufferAppendString(sendBuf, buf);
#ifndef MSG_SEND_AUTO
    bufferSendData(sendBuf, cfd);
#endif
    free(namelist);
    return 0;
}


int sendFile(const char* fileName, struct Buffer* sendBuf, int cfd)
{
    // 1. ���ļ�
    int fd = open(fileName, O_RDONLY);
    assert(fd > 0);
#if 1
    while (1)
    {
        char buf[1024];
        int len = read(fd, buf, sizeof buf);
        if (len > 0)
        {
            //send(cfd, buf, len, 0);
            bufferAppendData(sendBuf, buf, len);
#ifndef MSG_SEND_AUTO
            bufferSendData(sendBuf, cfd);
#endif
            usleep(10); // ��ǳ���Ҫ
        }
        else if (len == 0)
        {
            break;
        }
        else
        {
            close(fd);
            perror("read");
        }
    }
#else
    off_t offset = 0;
    int size = lseek(fd, 0, SEEK_END);
    lseek(fd, 0, SEEK_SET);
    while (offset < size)
    {
        int ret = sendfile(cfd, fd, &offset, size - offset);
        printf("sending.. size = %d, offset = %d, ret = %d\n", size, offset, ret);
        printf("ret value: %d\n", ret);
        if (ret == -1 && errno == EAGAIN)
        {
            printf("û����...\n");
        }
    }
#endif
    close(fd);
    return 0;
}
