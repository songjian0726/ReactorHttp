#pragma once

struct Buffer {
	char* data;
	int capacity;
	int readPos;
	int writePos;
};

//��ʼ��
struct Buffer* bufferInit(int size);

//�����ڴ�
void bufferDestroy(struct Buffer* buf);

//�ṹ������
void bufferExtendRoom(struct Buffer* buffer, int size);

//�鿴buf�п�д���ڴ�����
int bufferWriteableSize(struct Buffer* buffer);

//�鿴buf�пɶ����ڴ�����
int bufferReadableSize(struct Buffer* buffer);

//д�ڴ�ĺ���  ֱ��д��������׽�������
//ֱ��д�ڴ�
int bufferAppendData(struct Buffer* buffer, const char* data, int size);
int bufferAppendString(struct Buffer* buffer, const char* data);
//�����׽�������
int bufferSocketRead(struct Buffer* buffer, int fd);


//����\r\nȡ��һ��(�ҵ�\r\n�����ݿ��е�λ��)
char* bufferFindCRLF(struct Buffer* buffer);