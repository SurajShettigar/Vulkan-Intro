#include <fstream>
#include <vector>
#include <string>
#include <iostream>

static std::vector<char> readBinFile(const std::string &filepath) throw()
{
    std::ifstream file(filepath, std::ios::ate | std::ios::binary);

    if(!file.is_open())
        throw std::runtime_error(std::string("Failed to open file from location: ") + filepath);
    
    size_t fileSize = static_cast<size_t>(file.tellg());
    std::vector<char> fileData(fileSize);

    file.seekg(0);
    file.read(fileData.data(), fileSize);

    file.close();

    return fileData;
}
