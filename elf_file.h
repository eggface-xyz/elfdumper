#ifndef _ELF_FILE_H
#define _ELF_FILE_H

#include <util.h>

#include <string_view>
#include <string>
#include <vector>
#include <bit>

#include <fmt/format.h>

#include <elf.h>

using namespace std::literals;

struct Elf_64 
{
    using Ehdr = Elf64_Ehdr;
    using Phdr = Elf64_Phdr;
    using Shdr = Elf64_Shdr;
};

template<typename ElfVar>
class Elf
{
public:
    using Ehdr = typename ElfVar::Ehdr;
    using Phdr = typename ElfVar::Phdr;
    using Shdr = typename ElfVar::Shdr;

    Elf(std::string_view filename);
    void dump();

private:
    void dumpElfHeader(Ehdr* ehdr);
    void dumpProgramHeader(Phdr* phdr, int index);
    void dumpSectionHeader(Shdr* shdr, int index);
    void dumpSection(Shdr *shdr, int index);
private:
    std::string m_filename;
    std::vector<uint8_t> m_data;
    Ehdr *m_ehdr;
    std::vector<Phdr*> m_phdrs;
    std::vector<Shdr*> m_shdrs;

    uint16_t m_sectionNameIndex;
    std::vector<std::string_view> m_sectionNames;
};

using Elf64 = Elf<Elf_64>;

template<typename ElfVar>
Elf<ElfVar>::Elf(std::string_view filename) : m_filename(filename),
                    m_data(readFile(filename)),
                    m_ehdr(std::bit_cast<Ehdr*>(m_data.data())),
                    m_sectionNameIndex(m_ehdr->e_shstrndx)
{
    for (int i = 0; i != m_ehdr->e_phnum; ++i)
        m_phdrs.push_back(std::bit_cast<Phdr*>(m_data.data() + i * m_ehdr->e_phentsize + m_ehdr->e_phoff));
    for (int i = 0; i != m_ehdr->e_shnum; ++i)
        m_shdrs.push_back(std::bit_cast<Shdr*>(m_data.data() + i * m_ehdr->e_shentsize + m_ehdr->e_shoff));
    auto* section = m_shdrs[m_sectionNameIndex];
    auto sectionNames = std::string_view(std::bit_cast<const char*>(m_data.data()) + section->sh_offset, section->sh_size);
    for (int i = 0; i != m_ehdr->e_shnum; ++i)
    {
        auto* shdr = m_shdrs[i];
        auto nameLength = sectionNames.find_first_of('\0', shdr->sh_name) - shdr->sh_name;
        m_sectionNames.push_back(sectionNames.substr(shdr->sh_name, nameLength));
    }

}

template<typename ElfVar>
void Elf<ElfVar>::dump()
{
    dumpElfHeader(m_ehdr);
    for (int i = 0; i != m_ehdr->e_phnum; ++i)
    {
        dumpProgramHeader(m_phdrs[i], i);
    }
    for (int i = 0; i != m_ehdr->e_shnum; ++i)
    {
        dumpSectionHeader(m_shdrs[i], i);
    }
}

template<typename ElfVar>
void Elf<ElfVar>::dumpElfHeader(Ehdr* ehdr)
{
    fmt::print("Elf Header:\n");
    fmt::print("\tElf type: {}\n", ElfType2Str(ehdr->e_type));
    fmt::print("\tElf machine: {}\n", Machine2Str(ehdr->e_machine));
    fmt::print("\tEntry: {:#X}\n", ehdr->e_entry);
    fmt::print("\tProgram header offset : {:#X}\n", ehdr->e_phoff);
    fmt::print("\tSection header offset : {:#X}\n", ehdr->e_shoff);
    fmt::print("\tElf header size : {:#X}\n", ehdr->e_ehsize);
    fmt::print("\tProgram header entry size : {:#X}\n", ehdr->e_phentsize);
    fmt::print("\tProgram header entry number : {:#X}\n", ehdr->e_phnum);
    fmt::print("\tSection header entry size : {:#X}\n", ehdr->e_shentsize);
    fmt::print("\tSection header entry number : {:#X}\n", ehdr->e_shnum);
    fmt::print("\tString table secion index for Section names: {:#X}\n",
            ehdr->e_shstrndx);
    fmt::print("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
}

template<typename ElfVar>
void Elf<ElfVar>::dumpProgramHeader(Phdr* phdr, int index)
{
    fmt::print("Segment header at index {}\n", index);
    fmt::print("\tSegment type: {}\n", SegmentType2Str(phdr->p_type));
    fmt::print("\tSegment offset: {:#X}\n", phdr->p_offset);
    fmt::print("=============================\n", phdr->p_offset);
}

template<typename ElfVar>
void Elf<ElfVar>::dumpSectionHeader(Shdr* shdr, int index)
{
    fmt::print("Section header at index {}\n", index);
    fmt::print("\tSection header naming index {}, name: {}\n", shdr->sh_name, m_sectionNames[index]);
    fmt::print("\tSection at ({:#X}, {:#X}, {:#X}) in file\n", shdr->sh_offset, shdr->sh_size,
            shdr->sh_offset + shdr->sh_size);
    fmt::print("\tSection address in memory: {:#X}, align requirement {:#X}\n", shdr->sh_addr,
            shdr->sh_addralign);
    fmt::print("\tSection type {}\n", SectionType2Str(shdr->sh_type));
    fmt::print("\tSection flag {:#X}, {}\n", shdr->sh_flags, Shf2str(shdr->sh_flags));
    fmt::print("\tSection sh_link {:#X}, sh_info {:#X}\n", shdr->sh_link, shdr->sh_info);
    dumpSection(shdr, index);
    fmt::print("--------------------------------------\n");
}

template<typename ElfVar>
void Elf<ElfVar>::dumpSection(Shdr *shdr, int index)
{
    if (m_sectionNames[index] == ".interp"sv)
    {
        fmt::print("Interpreter: {}\n", std::string_view{std::bit_cast<char*>(m_data.data()) +
                shdr->sh_offset, shdr->sh_size});
    }
    if (m_sectionNames[index] == ".dynsym"sv)
    {

    }
}

#endif // _ELF_FILE_H
