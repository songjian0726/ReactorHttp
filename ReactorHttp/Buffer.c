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

void bufferExtendRoom(struct Buffer* buffer, int size)//�����size��ÿ��дʱд��Ĵ�С
{
	if (bufferWriteableSize(buffer) >= size) {//�ڴ湻д
		return;
	}
	if (buffer->readPos + bufferWriteableSize(buffer) >= size) {
		//�õ�δ���ڴ��С
		int readable = bufferReadableSize(buffer);
		//�ƶ��ڴ�
		memcpy(buffer->data, buffer->data + buffer->readPos, readable);
		//����λ��
		buffer->readPos = 0;
		buffer->writePos = readable;
	}
	else {//�ڴ治���� ����
		void* temp = realloc(buffer->data, buffer->capacity + size);
		if (temp != NULL) {
			return;
		}
		memset(temp + buffer->capacity, 0, size);
		//��������
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
	if (buffer == NULL || data == NULL || size <= 0) {//�Ƿ����
		return -1;
	}
	//����
	bufferExtendRoom(buffer, size);//��֤�ռ��㹻
	memcpy(buffer->data + buffer->writePos, data, size);//��buffer->data��дָ��λ�ÿ�ʼд
	buffer->writePos += size;//����дָ��λ��
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
	struct iovec vec[2]; //readv��Ҫ�Ľṹ������
	//��ʼ������Ԫ��
	int writeable = bufferReadableSize(buffer);
	vec[0].iov_base = buffer->data + buffer->writePos;
	vec[0].iov_len = writeable;

	char* tmpbuf = (char*)malloc(40960);//�ڶ���ռ��ֶ�����40k  ��Ҫ�ͷ�
	vec[1].iov_base = buffer->data + buffer->writePos;
	vec[1].iov_len = 40960;

	int result = readv(fd, vec, 2);//2��ʾ2���ڴ�
	if (result == -1) { //readvʧ�ܷ���-1
		return -1;
	}
	else if (result <= writeable) {
		//��ʱ����ȫ��vec[0] ��(buffer->data)��
		buffer->writePos += result;//�ƶ�дָ��
	}
	else {//��ʱvec[1]���в������ݣ�Ӧ����չbuffer->data��������ȥ ע���ʱdataд����дָ��Ӧ��ָ��ĩβ
		buffer->writePos = buffer->capacity;
		bufferAppendData(buffer, tmpbuf, result - writeable);
	}
	free(tmpbuf);
	return result;
}
