#include <iostream>
#include <fstream>

#include "cbor.hpp"

using namespace cbor;

void test_array() {
    DataItem items = std::vector<DataItem> {
        "name", 1024, true
    };
    cbor::iterator iter = items.begin();
    for (; iter != items.end(); iter++) {
        const DataItem& item = *iter;
        std::cout << item.dump() << std::endl;
    }

    // range-based for
    for (auto& item : items) {
        std::cout << item.dump() << '\n';
    }

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

void test_map() {
    DataItem cb = std::map<DataItem, DataItem> {
        {"a", 123},
        {1, "aaa"}
    };
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


    cbor::iterator iter = cb.begin();
    for (; iter != cb.end(); iter++) {
        std::cout << iter.key().dump() << " : " << iter.value().dump() << std::endl;
    }
}

int main(int argc, char** argv) {
    test_array();
    test_map();
    
    uint16_t int16 = 23;
    DataItem i16(int16);

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

    DataItem m = cbor::map({
        {"aa", "a"},
        {"bb", 12},
    });
    assert(m.type() == cbor::type_t::Map);

    m.items().begin();
    for (auto iter : m.items()) {
        std::cout << "iter " << iter.key() << " = " << iter.value() << std::endl;
    }

    // m.set_os_mode(cbor::stream_mode::Binary);
    std::cout << m << std::endl;

    std::ofstream oss("out.cbor", std::ios::out | std::ios::binary);
    oss << m;
}
