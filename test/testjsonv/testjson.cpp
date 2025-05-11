#include "json.hpp"
using json = nlohmann::json;

#include <iostream>
#include <vector>
#include <map>
#include <string>

std::string func1()
{
    json js;
    js["msg"] = 2;
    js["from"] = "zahngsan";
    js["to"] = "li si";
    // std::cout << js << std::endl;
    std::string buf = js.dump();
    // std::cout << buf.c_str() << std::endl;
    return buf;
}

std::string func2()
{
    json js;
    js["id"] = {1, 2, 3, 4, 5, 6};
    js["name"] = "zhangsan";
    js["msg"]["zhangsan"] = "helloworld";
    js["msg"]["zhang"] = "helloworld";
    js["msg"] = {{"zhangsan", "helloorld"}, {"zhang", "helloworld"}};
    // std::cout << js << std::endl;
    std::string s = js.dump();
    return s;
}

void func3()
{
    std::vector<int> v;
    v.push_back(1);
    v.push_back(5);
    v.push_back(2);
    json js;
    js["list"] = v;

    std::map<int, std::string> m;
    m.insert(std::make_pair<int, std::string>(1, "sss"));
    m.insert(std::make_pair<int, std::string>(5, "s"));
    m.insert(std::make_pair<int, std::string>(2, "ss"));
    js["path"] = m;

    std::string s = js.dump();

    std::cout << s.c_str() << std::endl;
}

int main()
{
    // std::string s = func1();
    std::string s = func2();

    //数据的反序列化

    json buf = json::parse(s);
    std::vector<int> arr = buf["id"];
    auto msg = buf["msg"];
    std::cout << msg << std::endl;
    // func2();
    // func3();
}