/**
* Copyright (c) 2017-present, Facebook, Inc.
* All rights reserved.

* This source code is licensed under the BSD-style license found in the
* LICENSE file in the root directory of this source tree.
*/

#ifndef _CUSTOM_ENUM_H_
#define _CUSTOM_ENUM_H_

#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <utility>
#include <sstream>
#include <mutex>

static std::string trim(std::string& str) {
    str.erase(0, str.find_first_not_of(' '));       //prefixing spaces
    str.erase(str.find_last_not_of(' ')+1);         //surfixing spaces
    return str;
}

static std::vector<std::string> split(const std::string &s, char delim) {
    std::stringstream ss(s);
    std::string item;
    std::vector<std::string> elems;
    while (getline(ss, item, delim)) {
        elems.push_back(std::move(item));
    }
    return elems;
}

static std::vector<std::pair<int, std::string> > get_enum_vals(const char *s) {
    std::vector<std::pair<int, std::string> > items;
    int enum_val = 0;
    for (auto& ele : split(std::string(s), ',')) {
        auto idx = ele.find_first_of('=');
        if (idx == std::string::npos) {
            items.push_back(make_pair(enum_val, trim(ele)));
        } else {
            enum_val = stoi(ele.substr(idx + 1, std::string::npos));
            std::string token = ele.substr(0, idx);
            items.push_back(std::make_pair(enum_val, trim(token)));
        }
        enum_val ++;
    }
    return items;
}

// Note that the following codes are not thread-safe. So we need to run them at the beginning
// of the code to make sure all tables are correctly constructed.
// [TODO] This is definitely not a good design.
#define custom_enum(TypeName, ...) \
    enum TypeName { __VA_ARGS__ }; \
    inline const std::string &_## TypeName ## 2 ## string(TypeName t) { \
        static const char *TypeName ## _definition = # __VA_ARGS__; \
        static std::map<TypeName, std::string> table;  \
        static std::mutex mutex; \
        if (table.empty()) { \
            std::lock_guard<std::mutex> lock(mutex); \
            /* cout << "enum " << # TypeName << ": type2str is not initialized yet! " << endl; */ \
            /* cout << "table.size() = " << table.size() << endl; */ \
            for (const auto& item : get_enum_vals(TypeName ## _definition)) { \
                table.insert(std::make_pair((TypeName)item.first, item.second)); \
            } \
            /* cout << "table.size() = " << table.size() << endl; */ \
        } \
        return table[t]; \
    }\
    inline TypeName _string ## 2 ## TypeName(const std::string &s) { \
        static const char *TypeName ## _definition = # __VA_ARGS__; \
        static std::map<std::string, TypeName> table;  \
        static std::mutex mutex; \
        if (table.empty()) {\
            std::lock_guard<std::mutex> lock(mutex); \
            /* cout << "enum " << # TypeName << ": str2type not initialized yet! " << endl; */ \
            for (const auto& item : get_enum_vals(TypeName ## _definition)) { \
                table.insert(std::make_pair(item.second, (TypeName)item.first)); \
            } \
        } \
        return table[s]; \
    }\
    template <typename Ar> \
    inline Ar &operator<<(Ar &oo, TypeName t) { \
        oo << _## TypeName ## 2 ## string(t); \
        return oo; \
    } \
    template <typename Ar> \
    inline Ar &operator>>(Ar &ii, TypeName& t) { \
        std::string value; \
        ii >> value; \
        t = _string ## 2 ## TypeName(value); \
        return ii; \
    } \
    namespace std  \
    { \
        template<> struct hash<TypeName> { uint64_t operator()(const TypeName& s) const { return std::hash<int>{}((int)s); } }; \
    } \


#endif
