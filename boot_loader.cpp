#include <algorithm>
#include <iterator>
#include <iostream>

#include <boost/tuple/tuple.hpp>

#include <elfio/elfio.hpp>

#include "boot_loader.hpp"


namespace brd { namespace boot {

namespace elf = ELFIO;

void load_code(const bus::address& proc, bus::bus_ptr bus, 
               const elf::elfio& executable);
void load_args(const bus::address& proc, bus::bus_ptr bus, 
               const elf::elfio& executable, int argc, char* argv[]);

void load(const bus::address& proc, bus::bus_ptr bus, 
          const std::string& path, int argc, char* argv[]) {
    elf::elfio executable;
    if (!executable.load(path)) {
        throw boot_error("can't find or process ELF file " + path);
    }

    load_code(proc, bus, executable);
    load_args(proc, bus, executable, argc, argv);
}

template <typename Iterator>
typename std::iterator_traits<Iterator>::difference_type
mismatch_index(Iterator first1, Iterator last1, Iterator first2) {
    std::pair<Iterator, Iterator> p = std::mismatch(first1, last1, first2);
    return std::distance(first1, p.first);
}

void load_code(const bus::address& proc, bus::bus_ptr bus, 
               const elf::elfio& executable) {
    typedef bus::ibus::value_type value_type;
    for (int i = 0; i < executable.sections.size(); ++i) {
        const elf::section* psec = executable.sections[i];
        
        if (psec->get_type() == SHT_PROGBITS) {

            assert(psec->get_size()%sizeof(value_type) == 0);
            std::vector<value_type> 
                data(reinterpret_cast<const value_type*>(psec->get_data()),
                     reinterpret_cast<const value_type*>(psec->get_data() + 
                                                         psec->get_size()));
        
            bus->write(proc + psec->get_address(), data.begin(), data.end());
            std::vector<value_type> mem(data.size());
            bus->read(proc + psec->get_address(), mem.begin(), mem.end());
            if (!std::equal(data.begin(), data.end(), mem.begin())) {
                throw memory_error(proc + mismatch_index(data.begin(), 
                                                         data.end(), 
                                                         mem.begin()));
            }
        }
    }
    
}

boost::tuple<unsigned int, unsigned int> 
find_symbol(const elf::elfio& executable, const std::string& name);

void load_args(const bus::address& proc, bus::bus_ptr bus, 
               const elf::elfio& executable, int argc, char* argv[]) {
    try {
        typedef bus::ibus::value_type value_type;
        unsigned int argv_addr, argv_size;
        boost::tie(argv_addr, argv_size) = 
            find_symbol(executable, "___argv_string");

        std::string argv_string(argc > 0 ? argv[0] : "");
        for (int i = 1; i < argc; ++i) {
            argv_string.push_back(' ');
            argv_string.append(argv[i]);
        }
        if (argv_string.length() < argv_size) {
            std::vector<value_type> data(argv_size, 0);
            std::copy(argv_string.begin(), argv_string.end(),
                      data.begin());
            bus->write(proc + argv_addr, 
                       data.begin(), data.end());

        } else {
            throw argument_error("arguments memory overflow");
        }
    } catch(symbol_not_found& w) {
        std::cerr << w.what() << std::endl;
    }
}

boost::tuple<unsigned int, unsigned int> 
find_symbol(const elf::elfio& executable, const std::string& name) {
    for (int i = 0; i < executable.sections.size(); ++i) {
        elf::section* psec = executable.sections[i];
        if (psec->get_type() == SHT_SYMTAB) {
            const elf::symbol_section_accessor symbols(executable, psec);
            for (unsigned int j = 0; j < symbols.get_symbols_num(); ++j) {
                std::string symbol_name;
                elf::Elf64_Addr value = 0;
                elf::Elf_Xword size = 0;
                unsigned char bind;
                unsigned char type;
                elf::Elf_Half section_index;
                unsigned char other;
                symbols.get_symbol(j, symbol_name, value, size, bind, type,
                                   section_index, other);
                if (symbol_name == name) {
                    return boost::make_tuple(value, size);
                }
            }
        }
    }
    throw symbol_not_found(name);
}


}} //namespace brd::boot
