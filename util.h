#ifndef _UTIL_H
#define _UTIL_H

#include <string_view>
#include <vector>
#include <stdexcept>
#include <fmt/format.h>
#include <elf.h>

std::vector<uint8_t> readFile(std::string_view filename);
std::string join(std::vector<std::string_view> seq, std::string_view del);

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

inline std::string_view SegmentType2Str(uint32_t p_type)
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
    case 0x6474E553:
        return "PT_GNU_PROPERTY";
    case 0x6474E550:
        return "PT_GNU_EH_FRAME";
    case 0x6474E551:
        return "PT_GNU_STACK";
    case 0x6474E552:
        return "PT_GNU_RELRO";

    default:
        throw std::runtime_error{"Unknown p_type " + fmt::format("{:#X}", p_type)};

    }
}

inline std::string_view SectionType2Str(uint32_t sh_type)
{
    switch(sh_type)
    {
    case 0:
        return "SHT_NULL";
    case 1:
        return "SHT_PROGBITS";
    case 2:
        return "SHT_SYMTAB";
    case 3:
        return "SHT_STRTAB";
    case 4:
        return "SHT_RELA";
    case 5:
        return "SHT_HASH";
    case 6:
        return "SHT_DYNAMIC";
    case 7:
        return "SHT_NOTE";
    case 8:
        return "SHT_NOBITS";
    case 9:
        return "SHT_REL";
    case 0xa:
        return "SHT_SHLIB";
    case 0xb:
        return "SHT_DYNSYM";
    case 0xe:
        return "SHT_INIT_ARRAY";
    case 0xf:
        return "SHT_FINI_ARRAY";
    case 0x10:
        return "SHT_PREINIT_ARRAY";
    case 0x60000000:
        return "SHT_LOOS";
    case 0x6ffffef:
        return "SHT_LOSUNW";
    case 0x6fffffef:
        return "SHT_SUNW_capchain";
    case 0x6ffffff0:
        return "SHT_SUNW_capinfo";
    case 0x6ffffff1:
        return "SHT_SUNW_symsort";
    case 0x6ffffff2:
        return "SHT_SUNW_tlssort";
    case 0x6ffffff3:
        return "SHT_SUNW_LDYNSYM";
    case 0x6ffffff4:
        return "SHT_SUNW_dof";
    case 0x6ffffff5:
        return "SHT_GNU_ATTRIBUTES";
    case 0x6ffffff6:
        return "SHT_GNU_HASH";
    case 0x6ffffff7:
        return "SHT_GNU_LIBLIST";
    case 0x6ffffff8:
        return "SHT_CHECKSUM";
    case 0x6ffffff9:
        return "SHT_SUNW_DEBUG";
    case 0x6ffffffa:
        return "SHT_SUNW_move";
    case 0x6ffffffb:
        return "SHT_SUNW_COMDAT";
    case 0x6ffffffc:
        return "SHT_SUNW_syminfo";
    case 0x6ffffffd:
        return "SHT_GNU_verdef";
    case 0x6ffffffe:
        return "SHT_GNU_verneed";
    case 0x6fffffff:
        return "SHT_GNU_versym";
    default:
        throw std::runtime_error{"Unknow sh_type " + fmt::format("{:#X}", sh_type)};
    }
}

inline std::string Shf2str(uint32_t flag)
{
    std::vector<std::string_view> ret;
    if (flag & SHF_WRITE)
        ret.push_back("SHF_WRITE");
    if (flag & SHF_ALLOC)
        ret.push_back("SHF_ALLOC");
    if (flag & SHF_EXECINSTR)
        ret.push_back("SHF_EXECINSTR");
    if (flag & SHF_MERGE)
        ret.push_back("SHF_MERGE");
    if (flag & SHF_STRINGS)
        ret.push_back("SHF_STRINGS");
    if (flag & SHF_INFO_LINK)
        ret.push_back("SHF_INFO_LINK");
    if (flag & SHF_OS_NONCONFORMING)
        ret.push_back("SHF_OS_NONCONFORMING");
    if (flag & SHF_GROUP)
        ret.push_back("SHF_GROUP");
    if (flag & SHF_TLS)
        ret.push_back("SHF_TLS");
    if (flag & SHF_COMPRESSED)
        ret.push_back("SHF_COMPRESSED");
    if (flag & SHF_GNU_RETAIN)
        ret.push_back("SHF_GNU_RETAIN");
    if (flag & SHF_ORDERED)
        ret.push_back("SHF_ORDERED");
    if (flag & SHF_EXCLUDE)
        ret.push_back("SHF_EXCLUDE");
    if (ret.empty())
        return "Empty flags";
    return join(ret, " | ");
}

#endif // _UTIL_H
