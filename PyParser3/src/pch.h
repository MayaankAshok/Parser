#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <array>
#include <unordered_set>
#include <algorithm>
#include <unordered_map>
#include <assert.h>
#include <sstream>
#include <fstream>
#include <cctype>
#include <locale>
#include <variant>
#include <filesystem>
#include <any>


#include "spdlog/spdlog.h"
#define INFO spdlog::info
#define DEB spdlog::debug
#define ERR spdlog::error


// trim from start (in place)
inline void ltrim(std::string& s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
        return !std::isspace(ch);
        }));
}

// trim from end (in place)
inline void rtrim(std::string& s) {
    s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
        return !std::isspace(ch);
        }).base(), s.end());
}

inline void trim(std::string& s) {
	rtrim(s);
	ltrim(s);
}

inline std::string trim_copy(std::string s) {
	trim(s);
	return s;
}

namespace fs = std::filesystem;

inline std::string readFile(fs::path path)
{
    // Open the stream to 'lock' the file.
    std::ifstream f(path, std::ios::in | std::ios::binary);

    // Obtain the size of the file.
    const auto sz = fs::file_size(path);

    // Create a buffer.
    std::string result(sz, '\0');

    // Read the whole file into the buffer.
    f.read(result.data(), sz);

    return result;
}