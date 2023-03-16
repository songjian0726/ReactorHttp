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

char* splitRequestLine(const char* start, const char* end, const char* sub, char** ptr) {//sub是要检测的子字符串
    char* space = end;
    if (sub != NULL) { //分离最后一段时，sub传入NULL
        char* space = memmem(start, end - start, sub, sizeof(sub));//找到空格的位置，第一个空格前是GET/POSE
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
    //读出请求行，保存字符串结束地址
    char* end = bufferFindCRLF(readBuf);
    //保存字符串起始地址
    char* start = readBuf->data + readBuf->readPos;
    //请求行总长度
    int lineSize = end - start;

    if (lineSize) {
        start = splitRequestLine(start, end, " ", &request->method);//请求方式  最后一个参数是指针的地址 传出参数
        start = splitRequestLine(start, end, " ", &request->url);//路径
        splitRequestLine(start, end, NULL, &request->version);//版本
#if 0
        //请求方式 GET等
        char* space = memmem(start, lineSize, " ", 1);//找到空格的位置，第一个空格前是GET/POSE
        assert(space != NULL);
        int methodSize = space - start;
        request->method = (char*)malloc(methodSize + 1);
        strncpy(request->method, start, methodSize);
        request->method[methodSize] = '\0';

        //请求的静态资源 路径等 第一个空格和第二个空格之间
        start = space + 1;//从第一个空格后开始
        char* space = memmem(start, end - start, " ", 1);//剩余需要检测的空间为end - start
        assert(space != NULL);
        int urlSize = space - start;
        request->url = (char*)malloc(urlSize + 1);
        strncpy(request->url, start, urlSize);
        request->url[urlSize] = '\0';

        //请求的HTTP版本
        start = space + 1;//从第二个空格后开始 到end结束
        //char* space = memmem(start, end - start, " ", 1);//剩余需要检测的空间为end - start
        //assert(space != NULL);
        //int urlSize = space - start;
        request->version = (char*)malloc(end - start + 1);
        strncpy(request->version, start, end - start);
        request->version[end - start] = '\0';
#endif

        //准备解析请求头
        readBuf->readPos = readBuf->readPos + lineSize + 2;//已经读了lineSize个字节，将readPos后移 后面还有\r\n 再后移2字节
        //修改当前的解析状态
        request->curState = ParseReqHeaders;
        return true;
    }

    return false;
}

//处理请求头中的一行
bool parseHttpRequestHeader(struct HttpRequest* request, struct Buffer* readBuf)
{
    char* end = bufferFindCRLF(readBuf);
    if (end != NULL) {
        char* start = readBuf->data + readBuf->readPos;
        int lineSize = end - start;
        //搜索:  分割键值对
        char* middle = memmem(start, lineSize, ": ", 2);//浏览器中的标准http请求中 :后面有一个空格
        if (middle != NULL) {
            char* key = malloc(middle - start + 1);//+1为了存储字符串最后的\0
            strncpy(key, start, middle - start);
            key[middle - start] = '\0';

            char* value = malloc(end - middle - 2 + 1);//-2 是": "
            strncpy(value, middle + 2, end - middle - 2);
            value[end - middle - 2] = '\0';
            httpRequestAddHeader(request, key, value);

            //后移readPos到下一行的位置
            readBuf->readPos += lineSize;
            readBuf->readPos += 2;
        }
        else {//如果middle == NULL 请求头已经解析完了 下面是一个空行
            //控制读指针跳过空行 即跳过两个字符 "\r\n"
            readBuf->readPos += 2;
            //修改解析状态 只考虑GET请求时，不需要接收第四部分
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
        case ParseReqBody://处理POST请求时需要考虑这部分 此处暂不处理
            break;
        default:
            break;
        }
        if (!flag) {
            return flag;
        }
        if (request->curState == ParseReqDone) { //解析完毕后 准备回复数据
            //1.根据解析出的数据对客户端的请求作出处理

            //2.组织响应数据并返回
        }
    }

    request->curState = ParseReqLine;//还原解析状态，便于处理后续请求
    return flag;
}

//处理request
bool processHttpRequest(struct HttpRequest* request)
{
    // 解析请求行 get /xxx/1.jpg http/1.1
    if (strcasecmp(request->method, "get") != 0)
    {
        return -1;
    }
    decodeMsg(request->url, request->url);
    // 处理客户端请求的静态资源(目录或者文件)
    char* file = NULL;
    if (strcmp(request->url, "/") == 0)
    {
        file = "./";
    }
    else
    {
        file = request->url + 1;
    }
    // 获取文件属性
    struct stat st;
    int ret = stat(file, &st);
    if (ret == -1)
    {
        // 文件不存在 -- 回复404

        //sendHeadMsg(cfd, 404, "Not Found", getFileType(".html"), -1);
        //sendFile("404.html", cfd);
        return 0;
    }
    // 判断文件类型
    if (S_ISDIR(st.st_mode))
    {
        // 把这个目录中的内容发送给客户端
        //sendHeadMsg(cfd, 200, "OK", getFileType(".html"), -1);
        //sendDir(file, cfd);
    }
    else
    {
        // 把文件的内容发送给客户端
        //sendHeadMsg(cfd, 200, "OK", getFileType(file), st.st_size);
        //sendFile(file, cfd);
    }
    return false;
}

// 将字符转换为整形数
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

// 解码
// to 存储解码之后的数据, 传出参数, from被解码的数据, 传入参数
void decodeMsg(char* to, char* from)
{
    for (; *from != '\0'; ++to, ++from)
    {
        // isxdigit -> 判断字符是不是16进制格式, 取值在 0-f
        // Linux%E5%86%85%E6%A0%B8.jpg
        if (from[0] == '%' && isxdigit(from[1]) && isxdigit(from[2]))
        {
            // 将16进制的数 -> 十进制 将这个数值赋值给了字符 int -> char
            // B2 == 178
            // 将3个字符, 变成了一个字符, 这个字符就是原始数据
            *to = hexToDec(from[1]) * 16 + hexToDec(from[2]);

            // 跳过 from[1] 和 from[2] 因此在当前循环中已经处理过了
            from += 2;
        }
        else
        {
            // 字符拷贝, 赋值
            *to = *from;
        }
    }
    *to = '\0';
}
