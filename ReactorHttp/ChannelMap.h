#pragma once
#include"Channel.h"
typedef struct ChannelMap {
	int size;//listָ��ָ��������Ԫ�ظ���
	struct Channel** list;// Channel* list[]
}ChannelMap;

//��ʼ��
ChannelMap* channelMapInit(int size);

//�������
void ChannelMapClear(ChannelMap* map);

//���·���list��С
bool makeMapRoom(ChannelMap* map, int newSize, int unitSize);//Ԫ�ظ�����ÿ��Ԫ�ش�С