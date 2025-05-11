#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>
#include <iostream>
#include <functional>

using namespace muduo;
using namespace muduo::net;

/**
 * 组合TcpServer对象
 * 创建EventLoop事件循环对象的指针
 * 明确TcpServer构造函数需要什么参数，输出ChatSever构造函数
 * 在当前服务器类的构造函数当中，注册处理连接的回调函数的处理读写事件的回调函数
 * 设置合适的服务器端线程数量， muduo库会自己划分IO线程和woker线程
 */

class ChatServer
{
public:
    //          事件循环        IP+POrt                         服务器名字
    ChatServer(EventLoop *loop, const InetAddress &listenAddr, const std::string &nameArg);

    //开启事件循环
    void start();

private:
    void onConnection(const TcpConnectionPtr &conn);

    //专门处理用户读写事件
    void onMessage(const TcpConnectionPtr &conn, Buffer *buffer, Timestamp time);

    TcpServer server_;
    EventLoop *loop_;
};