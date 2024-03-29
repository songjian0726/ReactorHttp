#include "Channel.h"
#include<stdlib.h>
#include"Log.h"

Channel* channelInit(int fd, int events, handleFunc readFunc,
 handleFunc writeFunc, handleFunc destroyFunc, void* arg)
{
	Debug("chennelInit in channel.c 8");
	struct Channel* channel = (struct Channel*)malloc(sizeof(struct Channel));
	channel->arg = arg;
	channel->fd = fd;
	channel->events = events;
	channel->readCallback = readFunc;
	channel->writeCallback = writeFunc;
	channel->destroyCallback = destroyFunc;
	return channel;
} 

void writeEventEnable(Channel* channel, bool flag)
{
	if (flag) {
		channel->events |= WriteEvent;
	}
	else {
		channel->events = channel->events & ~WriteEvent;  //λ�� WriteEvent�ķ�
	}
}

bool isWriteEventEnable(Channel* channel)
{
	return channel->events & WriteEvent;
}
