#ifndef _UTIL_H
#define _UTIL_H

#include <string_view>
#include <vector>
#include <stdexcept>
#include <fmt/format.h>

std::vector<uint8_t> readFile(std::string_view filename);

inline std::string_view ElfType2Str(uint16_t e_type)
{
    switch(e_type)
    {
    case 0:
        return "ET_NONE";
    case 1:
        return "ET_REL";
    case 2:
        return "ET_EXEC";
    case 3:
        return "ET_DYN";
    case 4:
        return "ET_CORE";
    default:
        throw std::runtime_error{"Unknown e_type"};
    }
}

inline std::string_view Machine2Str(uint16_t machine)
{
    switch(machine)
    {
    case 62:
        return "EM_X86_64";
    default:
        throw std::runtime_error{"Unknown machine"};
    }
}

inline std::string_view ProgramType2Str(uint32_t p_type)
{
    switch(p_type)
    {
    case 0:
        return "PT_NULL";
    case 1:
        return "PT_LOAD";
    case 2:
        return "PT_DYNAMIC";
    case 3:
        return "PT_INTERP";
    case 4:
        return "PT_NOTE";
    case 5:
        return "PT_SHLIB";
    case 6:
        return "PT_PHDR";
    case 0X6474E553:
        return "PT_GNU_PROPERTY";
    case 0X6474E550:
        return "PT_GNU_EH_FRAME";
    case 0X6474E551:
        return "PT_GNU_STACK";
    case 0X6474E552:
        return "PT_GNU_RELRO";

    default:
        throw std::runtime_error{"Unknown p_type " + fmt::format("{:#X}", p_type)};

    }
}

#endif // _UTIL_H
