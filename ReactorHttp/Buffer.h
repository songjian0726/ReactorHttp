#pragma once

struct Buffer {
	char* data;
	int capacity;
	int readPos;
	int writePos;
};

//初始化
struct Buffer* bufferInit(int size);

//销毁内存
void bufferDestroy(struct Buffer* buf);

//结构体扩容
void bufferExtendRoom(struct Buffer* buffer, int size);

//查看buf中可写的内存容量
int bufferWriteableSize(struct Buffer* buffer);

//查看buf中可读的内存容量
int bufferReadableSize(struct Buffer* buffer);

//写内存的函数  直接写，或接收套接字数据
//直接写内存
int bufferAppendData(struct Buffer* buffer, const char* data, int size);
int bufferAppendString(struct Buffer* buffer, const char* data);
//接收套接字数据
int bufferSocketRead(struct Buffer* buffer, int fd);


//根据\r\n取出一行(找到\r\n在数据块中的位置)
char* bufferFindCRLF(struct Buffer* buffer);