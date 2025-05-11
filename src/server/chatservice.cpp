#include "chatservice.h"
#include "public.h"

#include <muduo/base/Logging.h>
#include <vector>
#include <map>

using namespace muduo;

ChatService *ChatService::instance()
{
    static ChatService service;
    return &service;
}

ChatService::ChatService()
{
    msgHandlerMap_.insert(std::make_pair(LOGIN_MSG, std::bind(&ChatService::login, this, _1, _2, _3)));
    msgHandlerMap_.insert(std::make_pair(REG_MSG, std::bind(&ChatService::reg, this, _1, _2, _3)));
    msgHandlerMap_.insert(std::make_pair(ONE_CHAT_MSG, std::bind(&ChatService::onChat, this, _1, _2, _3)));
    msgHandlerMap_.insert(std::make_pair(ADD_FRIEND_MSG, std::bind(&ChatService::addFriend, this, _1, _2, _3)));
    msgHandlerMap_.insert(std::make_pair(CREATE_GROUP_MSG, std::bind(&ChatService::createGroup, this, _1, _2, _3)));
    msgHandlerMap_.insert(std::make_pair(ADD_GROUP_MSG, std::bind(&ChatService::addGroup, this, _1, _2, _3)));
    msgHandlerMap_.insert(std::make_pair(GROUP_CHAT_MSG, std::bind(&ChatService::groupChat, this, _1, _2, _3)));
    msgHandlerMap_.insert(std::make_pair(LOGINOUT_MSG, std::bind(&ChatService::loginOut, this, _1, _2, _3)));

    if (redis_.connect())
    {
        // 设置上报消息的回调
        redis_.init_notify_handler(std::bind(&ChatService::handleRedisSubscribeMessage, this, _1, _2));
    }
}

void ChatService::login(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    LOG_INFO << "do login service";
    int id = js["id"].get<int>();
    std::string pwd = js["password"];

    User user = usermodel_.query(id);
    if (user.getId() == id && user.getPwd() == pwd)
    {
        if (user.getState() == "online")
        {
            // 不可重复登陆
            json response;
            response["msgid"] = LOGIN_MSG_ACK;
            response["errno"] = 2;
            response["errmsg"] = "不可重复登陆";
            conn->send(response.dump());
        }
        else
        {
            // 登陆成功

            // 记录用户连接信息
            {
                std::lock_guard<std::mutex> lock(connMutex_);
                userConnMap_.insert(std::make_pair(id, conn));
            }

            redis_.subscribe(id);

            // 更新状态
            user.setState("online");
            usermodel_.updateState(user);

            json response;
            response["msgid"] = LOGIN_MSG_ACK;
            response["errno"] = 0;
            response["id"] = user.getId();
            response["name"] = user.getName();
            // 查询用户是否有离线消息
            std::vector<std::string> vec = offlineMsgModel_.query(id);
            if (!vec.empty())
            {
                response["offlinemsg"] = vec;
                offlineMsgModel_.remove(id);
            }
            // 查询公司用户的好友信息
            std::vector<User> userVec = friendModel_.query(id);
            if (!userVec.empty())
            {
                std::vector<std::string> vec2;
                for (auto &user : userVec)
                {
                    json js;
                    js["id"] = user.getId();
                    js["name"] = user.getName();
                    js["state"] = user.getState();
                    vec2.push_back(js.dump());
                }
                response["friends"] = vec2;
            }
            // 查询用户群组和群员
            std::vector<Group> groupVec = groupModel_.queryGroup(id);
            if (!groupVec.empty())
            {
                std::vector<std::string> vec1;
                for (auto &group : groupVec)
                {
                    json groupJs;
                    groupJs["id"] = group.getId();
                    groupJs["groupname"] = group.getName();
                    groupJs["groupdesc"] = group.getDesc();

                    std::vector<std::string> userVec;
                    for (auto &user : group.getUsers())
                    {
                        json userJs;
                        userJs["id"] = user.getId();
                        userJs["name"] = user.getName();
                        userJs["state"] = user.getState();
                        userVec.push_back(userJs.dump());
                    }
                    groupJs["users"] = userVec;
                    vec1.push_back(groupJs.dump());
                }
                response["groups"] = vec1;
            }

            conn->send(response.dump());
        }
    }
    else
    {
        // 登录失败
        json response;
        response["msgid"] = LOGIN_MSG_ACK;
        response["errno"] = 1;
        response["errmsg"] = "用户名不存在或者密码错误";
        conn->send(response.dump());
    }
}

void ChatService::reg(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    LOG_INFO << "do reg service";
    std::string name = js["name"];
    std::string pwd = js["password"];

    User user;
    user.setName(name);
    user.setPwd(pwd);
    bool state = usermodel_.insert(user);
    if (state)
    {
        // 注册成功
        json response;
        response["msgid"] = REG_MSG_ACK;
        response["errno"] = 0;
        response["id"] = user.getId();
        conn->send(response.dump());
    }
    else
    {
        // 注册失败
        json response;
        response["msgid"] = REG_MSG_ACK;
        response["errno"] = 1;
        conn->send(response.dump());
    }
}

void ChatService::onChat(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int toid = js["to"].get<int>();
    // 在同一台服务器
    {
        std::lock_guard<std::mutex> lock(connMutex_);
        auto it = userConnMap_.find(toid);
        if (it != userConnMap_.end())
        {
            // toid在线，转发消息
            it->second->send(js.dump());
            return;
        }
    }

    // toid在线，但不在同一台服务器
    User user = usermodel_.query(toid);
    if (user.getState() == "online")
    {
        redis_.publish(toid, js.dump());
        return;
    }

    // toid不在线，存储离线消息
    offlineMsgModel_.insert(toid, js.dump());
}

void ChatService::addFriend(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int userId = js["id"].get<int>();
    int friendId = js["friendid"].get<int>();

    // 存储好友信息
    friendModel_.insert(userId, friendId);
}

MsgHandler ChatService::getHandler(int msgid)
{
    // 记录错误日志，msgid没有对应的事件处理回调
    auto it = msgHandlerMap_.find(msgid);
    if (it == msgHandlerMap_.end())
    {

        return [=](const TcpConnectionPtr &conn, json &js, Timestamp)
        {
            LOG_ERROR << "msgid:" << msgid << "can not find handler";
        };
    }
    else
    {
        return msgHandlerMap_[msgid];
    }
}

void ChatService::clientCloseException(const TcpConnectionPtr &conn)
{
    User user;
    {
        std::lock_guard<std::mutex> lock(connMutex_);
        for (auto it = userConnMap_.begin(); it != userConnMap_.end(); ++it)
        {
            if (it->second == conn)
            {
                // 从map表删除用户连接信息
                user.setId(it->first);
                userConnMap_.erase(it);
                break;
            }
        }
    }

    redis_.unsubscribe(user.getId());

    // 更新用户的状态信息
    if (user.getId() != -1)
    {
        user.setState("offline");
        usermodel_.updateState(user);
    }
}

void ChatService::loginOut(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int userid = js["id"].get<int>();
    {
        std::lock_guard<std::mutex> lock(connMutex_);
        for (auto it = userConnMap_.begin(); it != userConnMap_.end(); ++it)
        {
            if (it->second == conn)
            {
                // 从map表删除用户连接信息
                userConnMap_.erase(it);
            }
        }
    }

    redis_.unsubscribe(userid);

    // 更新用户的状态信息
    User user(userid, "", "", "offline");
    usermodel_.updateState(user);
}

void ChatService::reset()
{
    // 吧online用户设置成offline
    usermodel_.resetState();
}

void ChatService::createGroup(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int userid = js["id"].get<int>();
    std::string name = js["groupname"];
    std::string desc = js["groupdesc"];

    Group group(-1, name, desc);
    if (groupModel_.createGroup(group))
    {
        groupModel_.addGroup(userid, group.getId(), "creator");
    }
}

void ChatService::addGroup(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int userId = js["userid"].get<int>();
    int groupId = js["groupid"].get<int>();
    groupModel_.addGroup(userId, groupId, "normal");
}

void ChatService::groupChat(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int userId = js["userid"].get<int>();
    int groupId = js["groupid"].get<int>();
    std::vector<int> userIdVec = groupModel_.queryGroupUsers(userId, groupId);
    std::lock_guard<std::mutex> lock(connMutex_);
    for (auto &id : userIdVec)
    {
        auto it = userConnMap_.find(id);
        if (it != userConnMap_.end())
        {
            it->second->send(js.dump());
        }
        else
        {
            // toid在线，但不在同一台服务器
            User user = usermodel_.query(id);
            if (user.getState() == "online")
            {
                redis_.publish(id, js.dump());
                return;
            }
            else
            {
                // 存储离线消息
                offlineMsgModel_.insert(id, js.dump());
            }
        }
    }
}

void ChatService::handleRedisSubscribeMessage(int userid, std::string msg)
{
    std::lock_guard<std::mutex> lock(connMutex_);
    auto it = userConnMap_.find(userid);
    if (it != userConnMap_.end())
    {
        it->second->send(msg);
        return;
    }

    // 存储该用户的离线消息
    offlineMsgModel_.insert(userid, msg);
}
