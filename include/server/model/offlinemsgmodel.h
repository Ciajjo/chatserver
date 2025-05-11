#ifndef OFFLINEMSGMODEL_H
#define OFFLINEMSGMODEL_H

#include <string>
#include <vector>

// 提供离线消息的操作接口方法
class OfflineMsgModel
{
public:
    // 存储用户离线消息
    void insert(int userid, std::string msg);
    // 删除离线消息
    void remove(int userid);
    //  查询用户离线消息
    std::vector<std::string> query(int userid);
    // OfflineMsgModel();
    // ~OfflineMsgModel();

private:

};

#endif // OFFLINEMSGMODEL_H