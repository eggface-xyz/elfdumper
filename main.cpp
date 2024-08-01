#include <elf_file.h>

#include <iostream>

void printException(std::exception& ex)
{
    std::cout << "Caught exception from main: " << ex.what() << std::endl;
}

int main(int argc, char**argv)
{
    try {
        Elf64 elf{argv[1]};
        elf.dump();
        elf.write("a.out");
    } catch (std::exception& ex)
    {
        printException(ex);
    }
}
