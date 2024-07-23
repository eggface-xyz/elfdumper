#include <util.h>

#include <fstream>
#include <bit>

std::vector<uint8_t> readFile(std::string_view filename)
{
    std::ifstream input{filename.data(), std::ios::ate | std::ios::binary};
    if (!input.is_open())
    {
        throw std::runtime_error{"Can't open file " + std::string{filename}};
    }
    auto filesize = input.tellg();
    input.seekg(0);
    std::vector<uint8_t> content(static_cast<size_t>(filesize), 0);
    input.read(std::bit_cast<char*>(content.data()), filesize);
    return content;
}

