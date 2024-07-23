#ifndef _ELF_FILE_H
#define _ELF_FILE_H

#include <util.h>

#include <string_view>
#include <string>
#include <vector>
#include <bit>

#include <fmt/format.h>

#include <elf.h>

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
private:
    std::string m_filename;
    std::vector<uint8_t> m_data;
    Ehdr *m_ehdr;
    std::vector<Phdr*> m_phdrs;
    std::vector<Shdr*> m_shdrs;
};

using Elf64 = Elf<Elf_64>;

template<typename ElfVar>
Elf<ElfVar>::Elf(std::string_view filename) : m_filename(filename),
                    m_data(readFile(filename)),
                    m_ehdr(std::bit_cast<Ehdr*>(m_data.data()))
{
    for (int i = 0; i != m_ehdr->e_phnum; ++i)
        m_phdrs.push_back(std::bit_cast<Phdr*>(m_data.data() + i * m_ehdr->e_phentsize + m_ehdr->e_phoff));
    for (int i = 0; i != m_ehdr->e_shnum; ++i)
        m_shdrs.push_back(std::bit_cast<Shdr*>(m_data.data() + i * m_ehdr->e_shentsize + m_ehdr->e_shoff));

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
    fmt::print("\tSegment type: {}\n", ProgramType2Str(phdr->p_type));
    fmt::print("\tSegment offset: {:#X}\n", phdr->p_offset);
    fmt::print("=============================\n", phdr->p_offset);
}

template<typename ElfVar>
void Elf<ElfVar>::dumpSectionHeader(Shdr* shdr, int index)
{
    fmt::print("Section header at index {}\n", index);
    fmt::print("\tSection header naming index {}\n", shdr->sh_name);
    fmt::print("--------------------------------------\n");
}

#endif // _ELF_FILE_H
