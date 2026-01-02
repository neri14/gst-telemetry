#ifndef UTILS_STRING_H
#define UTILS_STRING_H

#include <string>
#include <algorithm>
#include <cctype>

namespace telemetry {

// Trim from the start (in place)
inline void ltrim(std::string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
        return !std::isspace(ch);
    }));
}

// Trim from the end (in place)
inline void rtrim(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
        return !std::isspace(ch);
    }).base(), s.end());
}

// Trim from both ends (in place)
inline void trim(std::string &s) {
    ltrim(s);
    rtrim(s);
}

inline std::string get_function_name(const std::string &s) {
    std::string str{s};
    trim(str);
    if (str.empty()) {
        return "";
    }
    size_t open_paren = str.find('(');
    if (open_paren != std::string::npos && str.back() == ')') {
        return str.substr(0, open_paren);
    }
    return "";
}

inline std::string get_function_argstr(const std::string &s) {
    std::string str{s};
    trim(str);
    if (str.empty()) {
        return "";
    }
    size_t open_paren = str.find('(');
    size_t close_paren = str.rfind(')');
    if (open_paren != std::string::npos && close_paren != std::string::npos && close_paren > open_paren) {
        str = str.substr(open_paren + 1, close_paren - open_paren - 1);
        trim(str);
        return str;
    }
    return "";
}

} // namespace telemetry

#endif // UTILS_STRING_H