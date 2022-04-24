#pragma once
#include <string>

// https://stackoverflow.com/questions/42946335/deprecated-header-codecvt-replacement

std::wstring string_to_wide_string(const std::string& string);
std::string wide_string_to_string(const std::wstring& wide_string);

