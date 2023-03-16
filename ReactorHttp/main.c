#include <stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include"TcpServer.h"
#include"Log.h"

int main(int argc, char* argv[])
{
    if (argc < 3)
    {
        printf("./a.out port path\n");
        return -1;
    }
    unsigned short port = atoi(argv[1]);
    // 切换服务器的工作路径
    chdir(argv[2]);
    Debug("chdir...");
    //启动服务器
    struct TcpServer* server = tcpServerInit(port, 4);
    Debug("main中创建了服务器...");

    tcpServerRun(server);
    Debug("main中启动了服务器...");

    return 0;
}