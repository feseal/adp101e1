#ifndef INSYS_BOOT_LOADER_HPP
#define INSYS_BOOT_LOADER_HPP

#include <string>
#include <stdexcept>
#include "ibus.hpp"

namespace insys { namespace boot {

struct boot_error : std::runtime_error {
    boot_error(const std::string& what_arg) throw() : std::runtime_error("boot: " + what_arg) {}
};

struct memory_error : boot_error {
    memory_error(const bus::address& addr) throw() : boot_error("memory error in address" + addr.str()) {}
};

struct argument_error : boot_error {
    argument_error(const std::string& what_arg) throw() : boot_error(what_arg) {}
};

struct symbol_not_found : boot_error {
    symbol_not_found(const std::string& what_arg) throw() : boot_error("warning " + what_arg + " not found") {}
};

void load(const bus::address& proc, bus::bus_ptr bus, const std::string& path, int argc = 0, char* argv[] = 0);


}} //namespce insys::boot

#endif //INSYS_BOOT_LOADER_HPP
