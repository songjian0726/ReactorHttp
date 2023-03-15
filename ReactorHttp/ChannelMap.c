#include "ChannelMap.h"
#include<stdio.h>
#include<stdlib.h>

ChannelMap* channelMapInit(int size)
{
	ChannelMap* map = (ChannelMap*)malloc(sizeof(ChannelMap));
	map->size = size;
	map->list = (Channel**)malloc(size * sizeof(Channel*));
	return map;
}

void ChannelMapClear(ChannelMap* map)
{
	if (map != NULL) {
		for (int i = 0; i < map->size; ++i) {
			if (map->list[i] != NULL) {
				free(map->list[i]);
			}
		}
		free(map->list);
		map->list = NULL;
	}
	map->size = 0;
}

bool makeMapRoom(ChannelMap* map, int newSize, int unitSize)
{
	if (map->size < newSize) {
		int curSize = map->size;
		//每次扩大为原来的2倍 直到大于等于newSize
		while (curSize < newSize) {
			curSize *= 2;
		}
		//扩容
		Channel ** temp = realloc(map->list, curSize * unitSize);
		if (temp == NULL) {
			return false;
		}
		map->list = temp;
		memset(&map->list[map->size], 0, (curSize - map->size) * unitSize);//将新加的空间清空
		map->size = curSize;
	}
	return false;
}
