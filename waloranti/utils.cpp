#include "utils.hpp"

std::string wstringToString(const std::wstring& wstr) {
    return std::string(wstr.begin(), wstr.end());
}