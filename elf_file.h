#ifndef _ELF_FILE_H
#define _ELF_FILE_H

#include <util.h>

#include <string_view>
#include <string>
#include <vector>
#include <bit>
#include <cassert>
#include <fstream>

#include <fmt/format.h>

#include <elf.h>

using namespace std::literals;

struct Elf_64
{
    using Ehdr = Elf64_Ehdr;
    using Phdr = Elf64_Phdr;
    using Shdr = Elf64_Shdr;
    using Sym = Elf64_Sym;
    using Rela = Elf64_Rela;
};

template<typename ElfVar>
class Elf
{
public:
    using Ehdr = typename ElfVar::Ehdr;
    using Phdr = typename ElfVar::Phdr;
    using Shdr = typename ElfVar::Shdr;
    using Sym = typename ElfVar::Sym;
    using Rela = typename ElfVar::Rela;

    Elf(std::string_view filename);
    void dump();
    void write(std::string_view filename);

private:
    void dumpElfHeader(Ehdr* ehdr);
    void dumpProgramHeader(Phdr* phdr, int index);
    void dumpSectionHeader(Shdr* shdr, int index);
    void dumpMapping()
    {
        fmt::print("Based on file \n");
        for (auto i = 0U; i < m_phdrs.size(); ++i)
        {
            range ph{m_phdrs[i]->p_offset, m_phdrs[i]->p_filesz};
            for (auto j = 0U; j < m_shdrs.size(); ++j)
            {
                range sh{m_shdrs[j]->sh_offset, m_shdrs[j]->sh_size};
                if (ph.contains(sh))
                {
                    fmt::print("segment {} includes section {}\n", i, m_sectionNames[j]);
                }
            }
        }
    }
	std::string_view getStringsFromStrtab(Shdr* shdr)
	{
		assert(shdr->sh_type == SHT_STRTAB);
		return {std::bit_cast<char*>(m_data.data()) + shdr->sh_offset, shdr->sh_size};
	}
	void dumpSymbolTable()
	{
		fmt::print("symbol table: \n");
        for (auto i = 0U; i < m_symbolNames.size(); ++i)
        {
			fmt::print("\t{}, {:#X}, {:#X} {}|{}, Vis:{}, section index: {}\n", 
                    /*name=*/m_symbolNames[i].substr(0, 16),
                    /*value=*/m_syms[i]->st_value,
                    /*size=*/m_syms[i]->st_size,
                    /*info.type=*/SymbolType2Str(ELF64_ST_TYPE(m_syms[i]->st_info)),
                    /*info.bind=*/SymbolBind2Str(ELF64_ST_BIND(m_syms[i]->st_info)),
                    /*vis=*/SymbolVis2Str(m_syms[i]->st_other),
                    /*index=*/m_syms[i]->st_shndx
                    );
        }
	}
    void dumpRelaTable()
    {
        fmt::print("rela table: \n");
        for (auto i = 0U; i < m_relas.size(); ++i)
        {
            fmt::print("\toffset {:#X}, info: {:#X}, addend: {:#X}\n",
                    m_relas[i]->r_offset, m_relas[i]->r_info, m_relas[i]->r_addend);
            fmt::print("\t r_type: {}, r_sym: {}\n",
                    RelocationType2Str(ELF64_R_TYPE(m_relas[i]->r_info)),
                    ELF64_R_SYM(m_relas[i]->r_info));
        }
    }

private:
    const std::string m_filename;
    const std::vector<uint8_t> m_data;
    Ehdr *m_ehdr;
    std::vector<Phdr*> m_phdrs;
    std::vector<Shdr*> m_shdrs;

    uint16_t m_sectionNameIndex;
    std::vector<std::string_view> m_sectionNames;

	uint64_t m_symtabStrtabIndex;
    std::vector<Sym*> m_syms;
    std::vector<std::string_view> m_symbolNames;

    std::vector<Rela*> m_relas;
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
    auto sectionNames = getStringsFromStrtab(m_shdrs[m_sectionNameIndex]);
    for (int i = 0; i != m_ehdr->e_shnum; ++i)
    {
        auto* shdr = m_shdrs[i];
        auto nameLength = sectionNames.find_first_of('\0', shdr->sh_name) - shdr->sh_name;
		auto sectionName = sectionNames.substr(shdr->sh_name, nameLength);
        m_sectionNames.push_back(sectionName);
		if (sectionName == ".symtab"sv)
		{
			assert(shdr->sh_entsize != 0 && "entry size is 0 for symbol table section");
			m_symtabStrtabIndex = shdr->sh_link;
			auto* strShdr = m_shdrs[m_symtabStrtabIndex];
			auto symbolNames = getStringsFromStrtab(strShdr);
			for (int i = 0; i < shdr->sh_size / shdr->sh_entsize; ++i)
			{
				auto* sym = std::bit_cast<Sym*>(m_data.data() + shdr->sh_offset) + i;
				m_syms.push_back(sym);
				auto nameLength = symbolNames.find_first_of('\0', sym->st_name) - sym->st_name;
				auto symbolName = symbolNames.substr(sym->st_name, nameLength);
        		m_symbolNames.push_back(symbolName);
			}
		}

        if (shdr->sh_type == SHT_RELA)
        {
			assert(shdr->sh_entsize != 0 && "entry size is 0 for rela table section");
            for (int i = 0; i < shdr->sh_size / shdr->sh_entsize; ++i)
            {
				auto* rela = std::bit_cast<Rela*>(m_data.data() + shdr->sh_offset) + i;
                m_relas.push_back(rela);
            }
        }
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
    dumpMapping();
	dumpSymbolTable();
    dumpRelaTable();
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
    fmt::print("\tSegment vaddr: {:#X}\n", phdr->p_vaddr);
    fmt::print("\tSegment paddr: {:#X}\n", phdr->p_paddr);
    fmt::print("\tSegment filesz: {:#X}\n", phdr->p_filesz);
    fmt::print("\tSegment memsz: {:#X}\n", phdr->p_memsz);
    fmt::print("=============================\n");
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
    fmt::print("--------------------------------------\n");
}

template<typename ElfVar>
void Elf<ElfVar>::write(std::string_view filename)
{
    auto filesize = m_ehdr->e_shoff + m_ehdr->e_shentsize * m_ehdr->e_shnum;
    fmt::print("output filesize {}\n", filesize);
    std::vector<uint8_t> data(filesize, 0);

    std::ofstream file{filename.data()};
    file.write(std::bit_cast<char*>(data.data()), data.size());
}

#endif // _ELF_FILE_H
