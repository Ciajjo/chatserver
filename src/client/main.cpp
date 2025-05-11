#include <iostream>
#include <thread>
#include <string>
#include <vector>
#include <chrono>
#include <ctime>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "group.h"
#include "user.h"
#include "public.h"
#include "json.hpp"

using json = nlohmann::json;

// 记录当前系统登陆的用户信息
User currentUser;
// 记录当前登录用户的好友列表信息
std::vector<User> friendList;
// 记录当前登录用户的群组列表信息
std::vector<Group> groupList;
// 显示当前登录成功用户的基本信息
void showCurrentUserData();
// 控制主菜单页面程序
bool isMainMenuRunning = false;

// 接受线程
void readTaskHandler(int);
// 获取系统时间（聊天信息需要添加时间信息）
std::string getCurrentTime();
// 主聊天页面程序
void mainMenu(int);

int main(int argc, char **argv)
{
    if (argc < 3)
    {
        std::cerr << "command invaild" << std::endl;
        exit(-1);
    }

    char *ip = argv[1];
    uint16_t port = atoi(argv[2]);

    int cfd = socket(AF_INET, SOCK_STREAM, 0);
    if (cfd == -1)
    {
        std::cout << "fd create error" << std::endl;
        exit(-1);
    }

    sockaddr_in saddr;
    memset(&saddr, 0, sizeof saddr);
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(port);
    saddr.sin_addr.s_addr = inet_addr(ip);

    if (connect(cfd, reinterpret_cast<sockaddr *>(&saddr), sizeof saddr) == -1)
    {
        std::cerr << "connect error" << std::endl;
        close(cfd);
        exit(-1);
    }

    while (true)
    {
        std::cout << "====================" << std::endl;
        std::cout << "1. login" << std::endl;
        std::cout << "2. register" << std::endl;
        std::cout << "3. quit" << std::endl;
        std::cout << "====================" << std::endl;
        std::cout << "choice:";
        int choice = 0;
        std::cin >> choice;
        std::cin.get();

        switch (choice)
        {
        case 1:
        {
            int id = 0;
            char pwd[50] = {0};
            std::cout << "userid: ";
            std::cin >> id;
            std::cin.get();
            std::cout << "userpasswd: ";
            std::cin.getline(pwd, 50);

            json js;
            js["msgid"] = LOGIN_MSG;
            js["id"] = id;
            js["password"] = pwd;
            std::string request = js.dump();

            int len = send(cfd, request.c_str(), strlen(request.c_str()) + 1, 0);
            if (len == -1)
            {
                std::cerr << "login error: " << request << std::endl;
            }
            else
            {
                char buf[BUFSIZ] = {0};
                len = recv(cfd, buf, BUFSIZ, 0);
                if (len == -1)
                {
                    std::cerr << "login response fail" << std::endl;
                }
                else
                {
                    json response = json::parse(buf);
                    if (response["errno"].get<int>() != 0)
                    {
                        std::cerr << response["errmsg"] << std::endl;
                    }
                    else
                    {
                        currentUser.setId(response["id"].get<int>());
                        currentUser.setName(response["name"]);

                        if (response.contains("friends"))
                        {
                            friendList.clear();
                            std::vector<std::string> vec = response["friends"];
                            for (auto &str : vec)
                            {
                                json js = json::parse(str);
                                User user;
                                user.setId(js["id"].get<int>());
                                user.setName(js["name"]);
                                user.setState(js["state"]);
                                friendList.push_back(user);
                            }
                        }

                        if (response.contains("groups"))
                        {
                            groupList.clear();
                            std::vector<std::string> vec = response["groups"];
                            for (auto &str : vec)
                            {
                                json js = json::parse(str);
                                Group group;
                                group.setId(js["id"].get<int>());
                                group.setName(js["groupname"]);
                                group.setDesc(js["groupdesc"]);
                                groupList.push_back(group);

                                std::vector<std::string> userVec = js["users"];
                                for (auto &userStr : userVec)
                                {
                                    GroupUser groupUser;
                                    json userjs = json::parse(userStr);
                                    groupUser.setId(js["id"].get<int>());
                                    groupUser.setName(js["name"]);
                                    groupUser.setState(js["state"]);
                                    groupUser.setRole(js["role"]);
                                    group.getUsers().push_back(groupUser);
                                }
                                groupList.push_back(group);
                            }
                        }
                        // 显示登录用户的基本信息
                        showCurrentUserData();

                        // 显示当前用户的离线消息  个人聊天信息或者群组消息
                        if (response.contains("offlinemsg"))
                        {
                            std::vector<std::string> vec = response["offlinemsg"];
                            for (std::string &str : vec)
                            {
                                json js = json::parse(str);
                                // time + [id] + name + " said: " + xxx
                                if (ONE_CHAT_MSG == js["msgid"].get<int>())
                                {
                                    std::cout << js["time"].get<std::string>() << " [" << js["id"] << "]" << js["name"].get<std::string>()
                                              << " said: " << js["msg"].get<std::string>() << std::endl;
                                }
                                else
                                {
                                    std::cout << "群消息[" << js["groupid"] << "]:" << js["time"].get<std::string>() << " [" << js["id"] << "]" << js["name"].get<std::string>()
                                              << " said: " << js["msg"].get<std::string>() << std::endl;
                                }
                            }
                        }

                        // 登陆成功，启动接受线程负责接收数据
                        static int threadNum = 0;
                        if (threadNum == 0)
                        {
                            threadNum = 1;
                            std::thread readTask(readTaskHandler, cfd);
                            readTask.detach();
                        }

                        // 进入聊天主菜单页面
                        isMainMenuRunning = true;
                        mainMenu(cfd);
                    }
                }
            }
        }
        break;
        case 2:
        {
            char name[50] = {0};
            char pwd[50] = {0};
            std::cout << "username: ";
            std::cin.getline(name, 50);
            std::cout << "userpassword: ";
            std::cin.getline(pwd, 50);

            json js;
            js["msgid"] = REG_MSG;
            js["name"] = name;
            js["password"] = pwd;
            std::string request = js.dump();

            int len = send(cfd, request.c_str(), strlen(request.c_str()) + 1, 0);
            if (len == -1)
            {
                std::cerr << "send error: " << request << std::endl;
            }
            else
            {
                char buf[BUFSIZ] = {0};
                len = recv(cfd, buf, BUFSIZ, 0);
                if (len == -1)
                {
                    std::cerr << name << "response fatal" << std::endl;
                }
                else
                {
                    json response = json::parse(buf);
                    if (response["errno"].get<int>() != 0)
                    {
                        std::cerr << "name is exist" << std::endl;
                    }
                    else
                    {
                        std::cout << name << " register success, userid is " << response["id"] << ", dont forget it" << std::endl;
                    }
                }
            }
        }
        break;
        case 3:
            close(cfd);
            exit(0);

        default:
            std::cerr << "invalid input!" << std::endl;
            break;
        }
    }
}

void showCurrentUserData()
{
    std::cout << "==========login user==========" << std::endl;
    std::cout << "current login user id: " << currentUser.getId() << "\tname: " << currentUser.getName();
    std::cout << "----------friend list---------" << std::endl;
    if (!friendList.empty())
    {
        for (auto &user : friendList)
        {
            std::cout << user.getId() << "\t" << user.getName() << "\t" << user.getState() << std::endl;
        }
    }
    std::cout << "----------group list----------" << std::endl;
    if (!groupList.empty())
    {
        for (auto &group : groupList)
        {
            std::cout << group.getId() << "\t" << group.getName() << "\t" << group.getDesc() << std::endl;
            for (auto &groupUser : group.getUsers())
            {
                std::cout << groupUser.getId() << "\t" << groupUser.getName() << "\t" << groupUser.getState() << "\t" << groupUser.getRole() << std::endl;
            }
        }
    }
    std::cout << "==============================" << std::endl;
}

void readTaskHandler(int cfd)
{
    while (true)
    {
        char buf[BUFSIZ] = {0};
        int len = recv(cfd, buf, BUFSIZ, 0);
        if (len == -1 || len == 0)
        {
            close(cfd);
            exit(-1);
        }

        json js = json::parse(buf);
        int msgtype = js["msgid"].get<int>();
        if (ONE_CHAT_MSG == msgtype)
        {
            std::cout << js["time"].get<std::string>() << " [" << js["id"] << "]" << js["name"].get<std::string>()
                      << " said: " << js["msg"].get<std::string>() << std::endl;
            continue;
        }
    }
}

// "help" command handler
void help(int fd = 0, std::string str = "");
// "chat" command handler
void chat(int, std::string);
// "addfriend" command handler
void addfriend(int, std::string);
// "creategroup" command handler
void creategroup(int, std::string);
// "addgroup" command handler
void addgroup(int, std::string);
// "groupchat" command handler
void groupchat(int, std::string);
// "loginout" command handler
void loginout(int, std::string);

// 系统支持的客户端命令列表
std::unordered_map<std::string, std::string> commandMap = {
    {"help", "显示所有支持的命令，格式help"},
    {"chat", "一对一聊天，格式chat:friendid:message"},
    {"addfriend", "添加好友，格式addfriend:friendid"},
    {"creategroup", "创建群组，格式creategroup:groupname:groupdesc"},
    {"addgroup", "加入群组，格式addgroup:groupid"},
    {"groupchat", "群聊，格式groupchat:groupid:message"},
    {"loginout", "注销，格式loginout"}};

// 注册系统支持的客户端命令处理
std::unordered_map<std::string, std::function<void(int, std::string)>> commandHandlerMap = {
    {"help", help},
    {"chat", chat},
    {"addfriend", addfriend},
    {"creategroup", creategroup},
    {"addgroup", addgroup},
    {"groupchat", groupchat},
    {"loginout", loginout}};

void mainMenu(int cfd)
{
    help();

    char buffer[1024] = {0};
    while (isMainMenuRunning)
    {
        std::cin.getline(buffer, 1024);
        std::string commandbuf(buffer);
        std::string command; // 存储命令
        int idx = commandbuf.find(":");
        if (-1 == idx)
        {
            command = commandbuf;
        }
        else
        {
            command = commandbuf.substr(0, idx);
        }
        auto it = commandHandlerMap.find(command);
        if (it == commandHandlerMap.end())
        {
            std::cerr << "invalid input command!" << std::endl;
            continue;
        }

        // 调用相应命令的事件处理回调，mainMenu对修改封闭，添加新功能不需要修改该函数
        it->second(cfd, commandbuf.substr(idx + 1, commandbuf.size() - idx)); // 调用命令处理方法
    }
}

// "help" command handler
void help(int, std::string)
{
    std::cout << "show command list >>> " << std::endl;
    for (auto &p : commandMap)
    {
        std::cout << p.first << " : " << p.second << std::endl;
    }
    std::cout << std::endl;
}
// "addfriend" command handler
void addfriend(int cfd, std::string str)
{
    int friendid = atoi(str.c_str());
    json js;
    js["msgid"] = ADD_FRIEND_MSG;
    js["id"] = currentUser.getId();
    js["friendid"] = friendid;
    std::string buffer = js.dump();

    int len = send(cfd, buffer.c_str(), strlen(buffer.c_str()) + 1, 0);
    if (-1 == len)
    {
        std::cerr << "send addfriend msg error -> " << buffer << std::endl;
    }
}
// "chat" command handler
void chat(int cfd, std::string str)
{
    int idx = str.find(":"); // friendid:message
    if (-1 == idx)
    {
        std::cerr << "chat command invalid!" << std::endl;
        return;
    }

    int friendid = atoi(str.substr(0, idx).c_str());
    std::string message = str.substr(idx + 1, str.size() - idx);

    json js;
    js["msgid"] = ONE_CHAT_MSG;
    js["id"] = currentUser.getId();
    js["name"] = currentUser.getName();
    js["toid"] = friendid;
    js["msg"] = message;
    js["time"] = getCurrentTime();
    std::string buffer = js.dump();

    int len = send(cfd, buffer.c_str(), strlen(buffer.c_str()) + 1, 0);
    if (-1 == len)
    {
        std::cerr << "send chat msg error -> " << buffer << std::endl;
    }
}
// "creategroup" command handler  groupname:groupdesc
void creategroup(int cfd, std::string str)
{
    int idx = str.find(":");
    if (idx == -1)
    {
        std::cerr << "chat command invalid!" << std::endl;
        return;
    }

    std::string groupName = str.substr(0, idx);
    std::string groupDesc = str.substr(idx + 1, str.size() - idx);

    json js;
    js["msgid"] = CREATE_GROUP_MSG;
    js["id"] = currentUser.getId();
    js["groupname"] = groupName;
    js["groupDesc"] = groupDesc;
    std::string buf = js.dump();

    int len = send(cfd, buf.c_str(), strlen(buf.c_str()) + 1, 0);
    if (-1 == len)
    {
        std::cerr << "send creategroup msg error -> " << buf << std::endl;
    }
}
// "addgroup" command handler
void addgroup(int cfd, std::string str)
{
    int groupid = atoi(str.c_str());
    json js;
    js["msgid"] = ADD_GROUP_MSG;
    js["id"] = currentUser.getId();
    js["groupid"] = groupid;
    std::string buffer = js.dump();

    int len = send(cfd, buffer.c_str(), strlen(buffer.c_str()) + 1, 0);
    if (-1 == len)
    {
        std::cerr << "send addgroup msg error -> " << buffer << std::endl;
    }
}
// "groupchat" command handler   groupid:message
void groupchat(int cfd, std::string str)
{
    int idx = str.find(":");
    if (-1 == idx)
    {
        std::cerr << "groupchat command invalid!" << std::endl;
        return;
    }

    int groupid = atoi(str.substr(0, idx).c_str());
    std::string message = str.substr(idx + 1, str.size() - idx);

    json js;
    js["msgid"] = GROUP_CHAT_MSG;
    js["id"] = currentUser.getId();
    js["name"] = currentUser.getName();
    js["groupid"] = groupid;
    js["msg"] = message;
    js["time"] = getCurrentTime();
    std::string buffer = js.dump();

    int len = send(cfd, buffer.c_str(), strlen(buffer.c_str()) + 1, 0);
    if (-1 == len)
    {
        std::cerr << "send groupchat msg error -> " << buffer << std::endl;
    }
}
// "loginout" command handler
void loginout(int cfd, std::string)
{
    json js;
    js["msgid"] = LOGINOUT_MSG;
    js["id"] = currentUser.getId();
    std::string buffer = js.dump();

    int len = send(cfd, buffer.c_str(), strlen(buffer.c_str()) + 1, 0);
    if (-1 == len)
    {
        std::cerr << "send loginout msg error -> " << buffer << std::endl;
    }
    else
    {
        isMainMenuRunning = false;
    }
}

// 获取系统时间（聊天信息需要添加时间信息）
std::string getCurrentTime()
{
    auto tt = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    struct tm *ptm = localtime(&tt);
    char date[60] = {0};
    sprintf(date, "%d-%02d-%02d %02d:%02d:%02d",
            (int)ptm->tm_year + 1900, (int)ptm->tm_mon + 1, (int)ptm->tm_mday,
            (int)ptm->tm_hour, (int)ptm->tm_min, (int)ptm->tm_sec);
    return std::string(date);
}