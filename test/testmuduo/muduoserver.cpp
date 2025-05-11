#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>
#include <functional>
#include <iostream>
#include <string>

using namespace muduo;
using namespace muduo::net;
using namespace std::placeholders;

class ChatServer
{
public:
    ChatServer(EventLoop* loop,
            const InetAddress& listenAddr,
            const string& nameArg)
            : server_(loop, listenAddr, nameArg)
            , loop_(loop)
    {
        server_.setConnectionCallback(std::bind(&ChatServer::onConnection, this, _1));

        server_.setMessageCallback(std::bind(&ChatServer::onMessage, this, _1, _2, _3));

        server_.setThreadNum(4);
    }

    void start()
    {
        server_.start();
    }

private:
    void onConnection(const TcpConnectionPtr &conn)
    {
        if (conn->connected())
        {
            std::cout << conn->peerAddress().toIpPort() << " -> " << conn->localAddress().toIpPort() << std::endl;
            std::cout << "state:on" << std::endl;
        }
        else
        {
            std::cout << conn->peerAddress().toIpPort() << " -> " << conn->localAddress().toIpPort() << std::endl;
            std::cout << "state:off" << std::endl;
            conn->shutdown();
            // loop_->quit();
        }
    }

    void onMessage(const TcpConnectionPtr &conn, Buffer *buffer, Timestamp receiveTime)
    {
        std::string buf = buffer->retrieveAllAsString();
        std::cout << "recv data:\t" << buf << "\ttime\t" << receiveTime.toString() << std::endl;
        conn->send(buf);
    }

    TcpServer server_;
    EventLoop *loop_;
};

int main()
{
    EventLoop loop;
    InetAddress addr("192.168.17.130", 6000);

    ChatServer server(&loop, addr, "server");
    server.start();
    loop.loop();
}