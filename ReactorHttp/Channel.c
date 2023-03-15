#include "Channel.h"

Channel* channelInit(int fd, int events, handleFund readFunc, handleFund writeFunc, void* arg)
{
	struct Channel* channel = (struct Channel*)malloc(sizeof(struct Channel));
	channel->arg = arg;
	channel->fd = fd;
	channel->events = events;
	channel->readCallback = readFunc;
	channel->writeCallback = writeFunc;
	return channel;
} 

void writeEventEnable(Channel* channel, bool flag)
{
	if (flag) {
		channel->events |= WriteEvent;
	}
	else {
		channel->events = channel->events & ~WriteEvent;  //位与 WriteEvent的反
	}
}

bool isWriteEventEnable(Channel* channel)
{
	return channel->events & WriteEvent;
}
