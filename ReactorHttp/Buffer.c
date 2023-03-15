#include "Buffer.h"
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/uio.h>
struct Buffer* bufferInit(int size)
{
	struct Buffer* buffer = (struct Buffer*)malloc(sizeof(struct Buffer));
	if (buffer != NULL) {
		buffer->data = (char*)malloc(size);
		buffer->capacity = size;
		buffer->writePos = buffer->writePos = 0;
		memset(buffer->data, 0, size);
	}
	return buffer;
}

void bufferDestroy(struct Buffer* buf)
{
	if (buf != NULL) {
		if (buf->data != NULL) {
			free(buf->data);
		}
		free(buf);
	}
}

void bufferExtendRoom(struct Buffer* buffer, int size)//这里的size是每次写时写入的大小
{
	if (bufferWriteableSize(buffer) >= size) {//内存够写
		return;
	}
	if (buffer->readPos + bufferWriteableSize(buffer) >= size) {
		//得到未读内存大小
		int readable = bufferReadableSize(buffer);
		//移动内存
		memcpy(buffer->data, buffer->data + buffer->readPos, readable);
		//更新位置
		buffer->readPos = 0;
		buffer->writePos = readable;
	}
	else {//内存不够用 扩容
		void* temp = realloc(buffer->data, buffer->capacity + size);
		if (temp != NULL) {
			return;
		}
		memset(temp + buffer->capacity, 0, size);
		//更新数据
		buffer->data = temp;
		buffer->capacity += size;

	}
}

int bufferWriteableSize(struct Buffer* buffer)
{
	return buffer->capacity - buffer->writePos;
}

int bufferReadableSize(struct Buffer* buffer)
{
	return buffer->writePos - buffer->readPos;
}

int bufferAppendData(struct Buffer* buffer, const char* data, int size)
{
	if (buffer == NULL || data == NULL || size <= 0) {//非法情况
		return -1;
	}
	//扩容
	bufferExtendRoom(buffer, size);//保证空间足够
	memcpy(buffer->data + buffer->writePos, data, size);//从buffer->data的写指针位置开始写
	buffer->writePos += size;//更新写指针位置
	return 0;
}

int bufferAppendString(struct Buffer* buffer, const char* data)
{
	int size = strlen(data);
	int ret = bufferAppendData(buffer, data, size);
	return ret;
}

int bufferSocketRead(struct Buffer* buffer, int fd)
{
	struct iovec vec[2]; //readv需要的结构体数组
	//初始化数组元素
	int writeable = bufferReadableSize(buffer);
	vec[0].iov_base = buffer->data + buffer->writePos;
	vec[0].iov_len = writeable;

	char* tmpbuf = (char*)malloc(40960);//第二块空间手动申请40k  需要释放
	vec[1].iov_base = buffer->data + buffer->writePos;
	vec[1].iov_len = 40960;

	int result = readv(fd, vec, 2);//2表示2块内存
	if (result == -1) { //readv失败返回-1
		return -1;
	}
	else if (result <= writeable) {
		//此时数据全在vec[0] 即(buffer->data)里
		buffer->writePos += result;//移动写指针
	}
	else {//此时vec[1]里有部分数据，应当拓展buffer->data并拷贝进去 注意此时data写满，写指针应当指向末尾
		buffer->writePos = buffer->capacity;
		bufferAppendData(buffer, tmpbuf, result - writeable);
	}
	free(tmpbuf);
	return result;
}
