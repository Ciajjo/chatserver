#include "m.h"

ChatServer::ChatServer(EventLoop *loop, const InetAddress &listenAddr, const std::string &nameArg)
    : server_(loop, listenAddr, nameArg), loop_(loop)
{
    // 给服务器注册用户连接的创建和断开回调
    server_.setConnectionCallback(std::bind(&ChatServer::onConnection, this, std::placeholders::_1));
    // 给服务器注册用户读写事件回调
    server_.setMessageCallback(std::bind(&ChatServer::onMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    // 设置服务器端的线程数量    1个IO线程 3个worker线程
    server_.setThreadNum(4);
}

void ChatServer::start()
{
    server_.start();
}

void ChatServer::onConnection(const TcpConnectionPtr &conn)
{
    if (conn->connected())
    {
        std::cout << conn->peerAddress().toIpPort() << "    =>  " << conn->localAddress().toIpPort() << " state:on" << std::endl;
    }
    else
    {
        std::cout << conn->peerAddress().toIpPort() << "    =>  " << conn->localAddress().toIpPort() << " state:off" << std::endl;
        conn->shutdown();
    }
}

void ChatServer::onMessage(const TcpConnectionPtr &conn, Buffer *buffer, Timestamp time)
{
    std::string buf = buffer->retrieveAllAsString();
    std::cout << "rece data:" << buf << "\ttime:" << time.toString() << std::endl;
    conn->send(buf);
}