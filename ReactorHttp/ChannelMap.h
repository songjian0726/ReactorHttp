#pragma once
#include"Channel.h"
typedef struct ChannelMap {
	int size;//list指针指向的数组的元素个数
	struct Channel** list;// Channel* list[]
}ChannelMap;

//初始化
ChannelMap* channelMapInit(int size);

//清空数据
void ChannelMapClear(ChannelMap* map);

//重新分配list大小
bool makeMapRoom(ChannelMap* map, int newSize, int unitSize);//元素个数，每个元素大小