#include "json.hpp"
#include <iostream>
#include <vector>
#include <map>
#include <string>

using json = nlohmann::json;


// void fun1()
// {
//     json js;
//     js["msg_type"] = 2;
//     js["from"] = "zhangsan";
//     js["to"] = "li si";
//     js["msg"] = "hello, what are you doing now";
//     std::cout << js << std::endl;
//     std::string str = js.dump();
//     std::cout << str.c_str() << std::endl;
// }

// void fun2()
// {
//     json js;
//     // 添加数组
//     js["id"] = {1, 2, 3, 4, 5};
//     // 添加key-value
//     js["name"] = "zhang san";
//     // 添加对象
//     js["msg"]["zhang san"] = "hello world";
//     js["msg"]["liu shuo"] = "hello china";
//     // 上面等同于下面这句一次性添加数组对象
//     js["msg"] = {{"zhang san", "hello world"}, {"liu shuo", "hello china"}};
//     std::cout << js << std::endl;
// }

// void fun3()
// {
//     json js;
//     // 直接序列化一个vector容器
//     std::vector<int> vec;
//     vec.push_back(1);
//     vec.push_back(2);
//     vec.push_back(5);
//     js["list"] = vec;
//     // 直接序列化一个map容器
//     std::map<int, std::string> m;
//     m.insert({1, "黄山"});
//     m.insert({2, "华山"});
//     m.insert({3, "泰山"});
//     js["path"] = m;
//     std::cout << js << std::endl;
//     std::string str = js.dump();
//     std::cout << str << std::endl;
// }

std::string fun1()
{
    json js;
    js["msg_type"] = 2;
    js["from"] = "zhangsan";
    js["to"] = "li si";
    js["msg"] = "hello, what are you doing now";
    std::string str = js.dump();
    return str;
}

std::string fun2()
{
    json js;
    // 添加数组
    js["id"] = {1, 2, 3, 4, 5};
    // 添加key-value
    js["name"] = "zhang san";
    // 添加对象
    js["msg"]["zhang san"] = "hello world";
    js["msg"]["liu shuo"] = "hello china";
    // 上面等同于下面这句一次性添加数组对象
    js["msg"] = {{"zhang san", "hello world"}, {"liu shuo", "hello china"}};

    return js.dump();
}

void fun3()
{
    json js;
    // 直接序列化一个vector容器
    std::vector<int> vec;
    vec.push_back(1);
    vec.push_back(2);
    vec.push_back(5);
    js["list"] = vec;
    // 直接序列化一个map容器
    std::map<int, std::string> m;
    m.insert({1, "黄山"});
    m.insert({2, "华山"});
    m.insert({3, "泰山"});
    js["path"] = m;
    std::cout << js << std::endl;
    std::string str = js.dump();
    std::cout << str << std::endl;
}

int main()
{
    // fun1();
    // fun2();
    // fun3();

    // std::string strBuf = fun1();
    std::string strBuf = fun2();
    json buf = json::parse(strBuf);

    // std::cout << buf["msg_type"] << std::endl;
    // std::cout << buf["from"] << std::endl;
    // std::cout << buf["to"] << std::endl;
    // std::cout << buf["msg"] << std::endl;

    std::cout << buf["id"] << std::endl;
    std::vector<int> v = buf["id"];
    for(auto &a : v)
    {
        std::cout << a << std::endl;
    }
}