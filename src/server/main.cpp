#include "chatserver.h"
#include "chatservice.h"

#include <iostream>
#include <signal.h>

// 处理服务器ctrl+c结束后， 重置user的状态信息
void resetHandler(int)
{
    ChatService::instance()->reset();
    exit(0);
}

int main()
{
    signal(SIGINT, resetHandler);

    EventLoop loop;
    InetAddress addr("127.0.0.1", 6000);
    ChatServer server(&loop, addr, "server");

    server.start();
    loop.loop();
}