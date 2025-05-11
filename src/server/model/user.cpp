#include "user.h"

User::User(int id, std::string name, std::string pwd, std::string state)
    : id_(id), name_(name), passwd_(pwd), state_(state) {}