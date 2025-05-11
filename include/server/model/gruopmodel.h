#ifndef GRUOPMODEL_H
#define GRUOPMODEL_H

#include "group.h"

class GroupModel
{
public:
    // GroupModel();
    // ~GroupModel();
    // 创建群组
    bool createGroup(Group &group);
    // 加入群组
    void addGroup(int userid, int groupid, std::string role);
    // 查询用户所在群组信息
    std::vector<Group> queryGroup(int userid);
    // 根据指定的groupid查询群组用户id列表，除userid自己，主要用户群聊业务给群其他成员群发消息
    std::vector<int> queryGroupUsers(int userid, int groupid);
};

#endif // GRUOPMODEL_H