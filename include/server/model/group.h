#ifndef GROUP_H
#define GROUP_H

#include <string>
#include <vector>

#include "groupuser.h"

class Group
{
public:
    Group(int id = -1, std::string name = "", std::string desc = "");
    // ~Group();

    void setId(int id) { id_ = id; }
    void setName(std::string name) { name_ = name; }
    void setDesc(std::string desc) { desc_ = desc; }

    int getId() { return id_; }
    std::string getName() { return name_; }
    std::string getDesc() { return desc_; }
    std::vector<GroupUser> &getUsers() { return users_; }

private:
    int id_;
    std::string name_;
    std::string desc_;
    std::vector<GroupUser> users_;
};

#endif // GROUP_H