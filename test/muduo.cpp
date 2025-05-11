#include "m.h"

int main()
{
    EventLoop loop;
    InetAddress addr("192.168.17.130", 6000);
    ChatServer server(&loop, addr, "chat");

    server.start();
    loop.loop();
}