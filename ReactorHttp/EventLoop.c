#include "EventLoop.h"
#include<assert.h>
#include<sys/socket.h>
struct EventLoop* eventLoopInit(){
    return eventLoopInitEx(NULL);
}

void taskWakeup(struct EventLoop* evloop) {
    const char* msg = "祝看到这句话的人天天开心！";
    write(evloop->socketPair[0], msg, sizeof msg);
}

int readLocalMassage(void* arg) {
    struct EventLoop* evloop = (struct EventLoop*)arg;
    char buf[256];
    read(evloop->socketPair[1], buf, sizeof(buf));//读出即可 无需处理
    return 0;
}

struct EventLoop* eventLoopInitEx(const char* threadName){
    struct EventLoop* evLoop = (struct EventLoop*)malloc(sizeof(struct EventLoop));

    evLoop->isQuit = false;
    evLoop->threadID = pthread_self();
    pthread_mutex_init(&evLoop->mutex, NULL);//初始化互斥锁
    strcpy(evLoop->threadName, threadName == NULL ? "MainThread" : threadName);
    evLoop->dispatcher = &EpollDispatcher; //指定了Epoll
    evLoop->dispatcherData = evLoop->dispatcher->init();

    //链表
    evLoop->head = evLoop->tail = NULL;

    //map
    evLoop->channelMap = channelMapInit(128);//初始大小

    //初始化本地socket fd
    int ret = socketpair(AF_UNIX, SOCK_STREAM, 0, evLoop->socketPair);
    if (ret == -1) {
        perror("socketpair");
        exit(0);
    }
    //指定evLoop->socketPair[0]发送，[1]接收 将socketPair[1]封装成一个channel
    struct Channel* channel = channelInit(evLoop->socketPair[1], ReadEvent, readLocalMassage, NULL, evLoop);
    //将封装的channel添加到任务队列
    eventLoopAddTask(evLoop, channel, ADD);
    return evLoop;
}

int eventLoopRun(EventLoop* evLoop){
    assert(evLoop != NULL);
    //取出事件分发和检测模型
    struct Dispatcher* dispatcher = evLoop->dispatcher;
    //比较线程ID是否正常
    if (evLoop->threadID != pthread_self()) {
        return -1;
    }
    //循环
    while (!evLoop->isQuit) {
        dispatcher->dispatch(&evLoop, 2); //超时2s
        eventLoopProcessTask(evLoop);
    }

    return 0;
}

int eventActivate(struct EventLoop* evLoop, int fd, int event){
    if (fd < 0 || evLoop == NULL) {
        return -1;
    }
    //取出channel
    struct Channel* channel = evLoop->channelMap->list[fd];
    assert(channel->fd == fd);
    if (event & ReadEvent && channel->readCallback) { //读事件
        channel->readCallback(channel->arg);
    }
    if (event & WriteEvent && channel->writeCallback) { //写事件
        channel->readCallback(channel->arg);
    }

    return 0;
}

int eventLoopAddTask(EventLoop* evLoop, Channel* channel, int type){
    //加锁，保护共享资源
    pthread_mutex_lock(&evLoop->mutex);
    struct ChannelElement* node = (struct ChannelElement*)malloc(sizeof(struct ChannelElement));
    node->channel = channel;
    node->type = type;
    node->next = NULL;
    //链表为空时
    if (evLoop->head = NULL) {
        evLoop->head = evLoop->tail = node;
    }
    else {
        evLoop->tail->next = node;
        evLoop->tail = node;
    }
    pthread_mutex_unlock(&evLoop->mutex);
    //处理节点
    if (evLoop->threadID == pthread_self()) {
        //当前线程为子线程
        eventLoopProcessTask(evLoop);
    }
    else {
        //主线程添加任务时 让子线程处理队列中的任务
        taskWakeup(evLoop);
    }

    return 0;
}

int eventLoopProcessTask(EventLoop* evLoop)//将任务队列里的任务取出 挂载到当前的分发模型上
{
    pthread_mutex_lock(&evLoop->mutex);
    //取出链表头节点
    struct ChannelElement* head = evLoop->head;
    while (head != NULL) {
        struct Channel* channel = head->channel;
        if (head->type == ADD) {
            //添加
            eventLoopAdd(evLoop, channel);
        }
        else if (head->type == DELETE) {
            //删除
            eventLoopRemove(evLoop, channel);
            
        }
        else if (head->type == MODIFY) {
            //修改
            eventLoopModify(evLoop, channel);
        }
        struct ChannelElement* tmp = head;
        head = head->next;
        free(tmp);
    }
    evLoop->head = evLoop->tail = NULL;

    pthread_mutex_unlock(&evLoop->mutex);

    return 0;
}

int eventLoopAdd(struct EventLoop* evLoop, struct Channel* channel)
{
    int fd = channel->fd;
    struct ChannelMap* channelMap = evLoop->channelMap;
    if (fd >= channelMap->size) {
        //channelMap中的list空间不足 扩容
        if (!makeMapRoom(channelMap, fd, sizeof(struct Channel*))) {
            return -1;
        }
    }
    //找到fd在channelMap中的位置并存储
    if (channelMap->list[fd] == NULL) {
        channelMap->list[fd] = channel;
        evLoop->dispatcher->add(channel, evLoop);
    }
    return 0;
}

int eventLoopRemove(struct EventLoop* evLoop, struct Channel* channel)
{
    int fd = channel->fd;
    struct ChannelMap* channelMap = evLoop->channelMap;
    if (fd >= channelMap->size) {
        return -1;
    }
    int ret = evLoop->dispatcher->remove(channel, evLoop);
    return ret;
}

int eventLoopModify(struct EventLoop* evLoop, struct Channel* channel)
{
    int fd = channel->fd;
    struct ChannelMap* channelMap = evLoop->channelMap;
    if (fd >= channelMap->size || channelMap->list[fd] == NULL) {
        return -1;
    }
    int ret = evLoop->dispatcher->modify(channel, evLoop);
    return ret;
}

int destroyChannel(struct EventLoop* evLoop, struct Channel* channel)
{
    //删除channel和fd的映射
    evLoop->channelMap->list[channel->fd] = NULL;
    //关闭fd
    close(channel->fd);
    //释放channel
    free(channel);

    return 0;
}
