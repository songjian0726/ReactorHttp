#include "EventLoop.h"
#include<assert.h>
#include<sys/socket.h>
struct EventLoop* eventLoopInit(){
    return eventLoopInitEx(NULL);
}

void taskWakeup(struct EventLoop* evloop) {
    const char* msg = "ף������仰�������쿪�ģ�";
    write(evloop->socketPair[0], msg, sizeof msg);
}

int readLocalMassage(void* arg) {
    struct EventLoop* evloop = (struct EventLoop*)arg;
    char buf[256];
    read(evloop->socketPair[1], buf, sizeof(buf));//�������� ���账��
    return 0;
}

struct EventLoop* eventLoopInitEx(const char* threadName){
    struct EventLoop* evLoop = (struct EventLoop*)malloc(sizeof(struct EventLoop));

    evLoop->isQuit = false;
    evLoop->threadID = pthread_self();
    pthread_mutex_init(&evLoop->mutex, NULL);//��ʼ��������
    strcpy(evLoop->threadName, threadName == NULL ? "MainThread" : threadName);
    evLoop->dispatcher = &EpollDispatcher; //ָ����Epoll
    evLoop->dispatcherData = evLoop->dispatcher->init();

    //����
    evLoop->head = evLoop->tail = NULL;

    //map
    evLoop->channelMap = channelMapInit(128);//��ʼ��С

    //��ʼ������socket fd
    int ret = socketpair(AF_UNIX, SOCK_STREAM, 0, evLoop->socketPair);
    if (ret == -1) {
        perror("socketpair");
        exit(0);
    }
    //ָ��evLoop->socketPair[0]���ͣ�[1]���� ��socketPair[1]��װ��һ��channel
    struct Channel* channel = channelInit(evLoop->socketPair[1], ReadEvent, readLocalMassage, NULL, evLoop);
    //����װ��channel��ӵ��������
    eventLoopAddTask(evLoop, channel, ADD);
    return evLoop;
}

int eventLoopRun(EventLoop* evLoop){
    assert(evLoop != NULL);
    //ȡ���¼��ַ��ͼ��ģ��
    struct Dispatcher* dispatcher = evLoop->dispatcher;
    //�Ƚ��߳�ID�Ƿ�����
    if (evLoop->threadID != pthread_self()) {
        return -1;
    }
    //ѭ��
    while (!evLoop->isQuit) {
        dispatcher->dispatch(&evLoop, 2); //��ʱ2s
        eventLoopProcessTask(evLoop);
    }

    return 0;
}

int eventActivate(struct EventLoop* evLoop, int fd, int event){
    if (fd < 0 || evLoop == NULL) {
        return -1;
    }
    //ȡ��channel
    struct Channel* channel = evLoop->channelMap->list[fd];
    assert(channel->fd == fd);
    if (event & ReadEvent && channel->readCallback) { //���¼�
        channel->readCallback(channel->arg);
    }
    if (event & WriteEvent && channel->writeCallback) { //д�¼�
        channel->readCallback(channel->arg);
    }

    return 0;
}

int eventLoopAddTask(EventLoop* evLoop, Channel* channel, int type){
    //����������������Դ
    pthread_mutex_lock(&evLoop->mutex);
    struct ChannelElement* node = (struct ChannelElement*)malloc(sizeof(struct ChannelElement));
    node->channel = channel;
    node->type = type;
    node->next = NULL;
    //����Ϊ��ʱ
    if (evLoop->head = NULL) {
        evLoop->head = evLoop->tail = node;
    }
    else {
        evLoop->tail->next = node;
        evLoop->tail = node;
    }
    pthread_mutex_unlock(&evLoop->mutex);
    //����ڵ�
    if (evLoop->threadID == pthread_self()) {
        //��ǰ�߳�Ϊ���߳�
        eventLoopProcessTask(evLoop);
    }
    else {
        //���߳��������ʱ �����̴߳�������е�����
        taskWakeup(evLoop);
    }

    return 0;
}

int eventLoopProcessTask(EventLoop* evLoop)//����������������ȡ�� ���ص���ǰ�ķַ�ģ����
{
    pthread_mutex_lock(&evLoop->mutex);
    //ȡ������ͷ�ڵ�
    struct ChannelElement* head = evLoop->head;
    while (head != NULL) {
        struct Channel* channel = head->channel;
        if (head->type == ADD) {
            //���
            eventLoopAdd(evLoop, channel);
        }
        else if (head->type == DELETE) {
            //ɾ��
            eventLoopRemove(evLoop, channel);
            
        }
        else if (head->type == MODIFY) {
            //�޸�
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
        //channelMap�е�list�ռ䲻�� ����
        if (!makeMapRoom(channelMap, fd, sizeof(struct Channel*))) {
            return -1;
        }
    }
    //�ҵ�fd��channelMap�е�λ�ò��洢
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
    //ɾ��channel��fd��ӳ��
    evLoop->channelMap->list[channel->fd] = NULL;
    //�ر�fd
    close(channel->fd);
    //�ͷ�channel
    free(channel);

    return 0;
}
