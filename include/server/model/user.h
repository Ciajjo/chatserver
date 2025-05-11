#ifndef USER_H
#define USER_H

#include <string>

class User
{
public:
    User(int id = -1, std::string name = "", std::string pwd = "", std::string state = "offline");
    // ~User();

    void setId(int id) { id_ = id; }
    void setName(std::string name) { name_ = name; }
    void setPwd(std::string pwd) { passwd_ = pwd; }
    void setState(std::string state) { state_ = state; }

    int getId() { return id_; }
    std::string getName() { return name_; }
    std::string getPwd() { return passwd_; }
    std::string getState() { return state_; }

protected:
    int id_;
    std::string name_;
    std::string passwd_;
    std::string state_;
};

#endif // USER_H