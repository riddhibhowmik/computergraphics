#pragma once
#include <string>
#include <vector>

class FileDialog {
public:
    static std::string OpenFile(const char* filter, const char* initialDir);
    static std::string SaveFile(const char* filter, const char* initialDir, const char* defaultName);
    static std::vector<std::string> OpenFiles(const char* filter, const char* initialDir);
};
