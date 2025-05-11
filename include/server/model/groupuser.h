#ifndef GROUPUSER_H
#define GROUPUSER_H

#include "user.h"

class GroupUser : public User
{
public:
    // GroupUser();
    // ~GroupUser();
    void setRole(std::string role) { role_ = role; }
    std::string getRole() { return role_; }

private:
    std::string role_;
};

#endif // GROUPUSER_H