#ifndef CHATSERVICE_H
#define CHATSERVICE_H

#include <muduo/net/TcpConnection.h>
#include <unordered_map>
#include <functional>
#include <mutex>

#include "json.hpp"
#include "usermodel.h"
#include "offlinemsgmodel.h"
#include "friwndmodel.h"
#include "gruopmodel.h"
#include "redis.h"

using namespace muduo;
using namespace muduo::net;
using json = nlohmann::json;

using MsgHandler = std::function<void(const TcpConnectionPtr &conn, json &js, Timestamp)>;

class ChatService
{
public:
    // 获取单例对象的接口函数
    static ChatService *instance();
    // ~ChatService();

    // 处理登录业务和注册业务
    void login(const TcpConnectionPtr &conn, json &js, Timestamp time);
    void reg(const TcpConnectionPtr &conn, json &js, Timestamp time);
    // 一一对一聊天业务
    void onChat(const TcpConnectionPtr &conn, json &js, Timestamp time);
    // 添加好友业务
    void addFriend(const TcpConnectionPtr &conn, json &js, Timestamp time);
    // 获取消息对应的处理器
    MsgHandler getHandler(int msgid);
    // 处理客户端异常退出
    void clientCloseException(const TcpConnectionPtr &conn);
    // 处理注销业务
    void loginOut(const TcpConnectionPtr &conn, json &js, Timestamp time);
    // 处理服务器异常退出
    void reset();
    // 创建群组业务
    void createGroup(const TcpConnectionPtr &conn, json &js, Timestamp time);
    // 加入群组业务
    void addGroup(const TcpConnectionPtr &conn, json &js, Timestamp time);
    // 群聊天业务
    void groupChat(const TcpConnectionPtr &conn, json &js, Timestamp time);
    // 从redis消息队列中获取订阅的消息
    void handleRedisSubscribeMessage(int, string);

private:
    ChatService();

    // 存储消息id和其相对应的业务处理方法
    std::unordered_map<int, MsgHandler> msgHandlerMap_;
    // 数据操作类对象
    UserModel usermodel_;
    OfflineMsgModel offlineMsgModel_;
    FriwndModel friendModel_;
    GroupModel groupModel_;
    // 存储在线用户的通信连接
    std::unordered_map<int, TcpConnectionPtr> userConnMap_;
    // 定义互斥锁 保证线程安全
    std::mutex connMutex_;
    // redis操作对象
    Redis redis_;
};

#endif // CHATSERVICE_H