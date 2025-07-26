#include <iostream>
#include <string>
#include <vector>
#include <elf.h>
#include <fcntl.h>
#include <unistd.h>
#include <libelf.h>

int main() {
    std::string filePath = "lib/heimdall-lld.so";
    
    std::cout << "Testing ELF parsing for: " << filePath << std::endl;
    
    // Initialize libelf
    if (elf_version(EV_CURRENT) == EV_NONE) {
        std::cerr << "ERROR: Failed to initialize libelf" << std::endl;
        return 1;
    }
    
    // Open the file
    int fd = open(filePath.c_str(), O_RDONLY);
    if (fd < 0) {
        std::cerr << "ERROR: Failed to open file: " << filePath << std::endl;
        return 1;
    }
    
    // Open ELF file
    Elf* elf = elf_begin(fd, ELF_C_READ, nullptr);
    if (!elf) {
        std::cerr << "ERROR: Failed to open ELF file with libelf" << std::endl;
        close(fd);
        return 1;
    }
    
    // Get ELF header
    Elf64_Ehdr* ehdr = elf64_getehdr(elf);
    if (!ehdr) {
        std::cerr << "ERROR: Failed to get ELF header" << std::endl;
        elf_end(elf);
        close(fd);
        return 1;
    }
    
    std::cout << "ELF file opened successfully" << std::endl;
    std::cout << "ELF type: " << ehdr->e_type << std::endl;
    std::cout << "ELF machine: " << ehdr->e_machine << std::endl;
    std::cout << "ELF sections: " << ehdr->e_shnum << std::endl;
    
    // Find dynamic section
    Elf_Scn* scn = nullptr;
    Elf64_Shdr* shdr = nullptr;
    bool found_dynamic = false;
    
    while ((scn = elf_nextscn(elf, scn)) != nullptr) {
        shdr = elf64_getshdr(scn);
        if (!shdr) {
            continue;
        }
        
        if (shdr->sh_type == SHT_DYNAMIC) {
            found_dynamic = true;
            std::cout << "Found dynamic section at index " << elf_ndxscn(scn) << std::endl;
            std::cout << "Dynamic section size: " << shdr->sh_size << std::endl;
            std::cout << "Dynamic section link: " << shdr->sh_link << std::endl;
            
            // Get dynamic section data
            Elf_Data* data = elf_getdata(scn, nullptr);
            if (!data) {
                std::cerr << "ERROR: Failed to get dynamic section data" << std::endl;
                continue;
            }
            
            // Get string table
            Elf_Scn* strscn = elf_getscn(elf, shdr->sh_link);
            if (!strscn) {
                std::cerr << "ERROR: Failed to get string table section" << std::endl;
                continue;
            }
            
            Elf64_Shdr* strshdr = elf64_getshdr(strscn);
            if (!strshdr) {
                std::cerr << "ERROR: Failed to get string table header" << std::endl;
                continue;
            }
            
            Elf_Data* strdata = elf_getdata(strscn, nullptr);
            if (!strdata) {
                std::cerr << "ERROR: Failed to get string table data" << std::endl;
                continue;
            }
            
            char* strtab = static_cast<char*>(strdata->d_buf);
            Elf64_Dyn* dyn = static_cast<Elf64_Dyn*>(data->d_buf);
            size_t ndyn = data->d_size / sizeof(Elf64_Dyn);
            
            std::cout << "Dynamic entries: " << ndyn << std::endl;
            
            for (size_t i = 0; i < ndyn; ++i) {
                if (dyn[i].d_tag == DT_NEEDED) {
                    std::string libName = strtab + dyn[i].d_un.d_val;
                    std::cout << "Found dependency: " << libName << std::endl;
                }
            }
            break;
        }
    }
    
    if (!found_dynamic) {
        std::cerr << "ERROR: No dynamic section found" << std::endl;
    }
    
    elf_end(elf);
    close(fd);
    
    return 0;
} 