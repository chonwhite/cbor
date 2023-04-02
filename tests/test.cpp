#include <iostream>

#include "cbor.hpp"

using namespace cbor;

// TODO
// friend stream

int main(int argc, char** argv) {
    DataItem cb = std::map<DataItem, DataItem> {
        {"a", 123},
        {1, "aaa"}
    };

    uint16_t int16 = 23;
    DataItem i16(int16);

    int a = cb["a"];
    std::cout << "a:" << a << std::endl;

    cb["a"] = 321;
    a = cb["a"];
    std::cout << "a:" << a << std::endl;

    cb["b"] = "abc";
    std::string b = cb["b"];
    std::cout << "b:" << b << std::endl;

    std::string one = cb[1]; // TODO index ? map;
    std::cout << "one:" << one << std::endl;

    DataItem args = std::vector<DataItem> {
        "name", 1024, true
    };
    DataItem payload = std::map<DataItem, DataItem>();
    payload['c'] = "cmd";
    payload['a'] = args;
    payload['t'] = 2;
    
    DataItem c = payload['c'];
    std::cout << "c:" << c.as<std::string>() << std::endl;

    int t = payload['t'];
    std::cout << "t:" << t << std::endl;
    // payload['t'].write(std::cout);
    // std::cout << t << std::endl;

    // std::map<int, int> aa;
    // DataItem m(aa);

    DataItem m = cbor::map({
        {"a", "a"},
        {"a", "a"}
    });

    DataItem array = cbor::array();
    assert(array.type() == cbor::type_t::Array);
    array.push_back(234);
    array.push_back(DataItem(222));
    array.emplace_back(111);
    array.emplace_back(cbor::map({
        {"a", "b"}
    }));
    
    DataItem map = cbor::map({});
    assert(map.type() == cbor::type_t::Map);

    array.push_back(map);


}
