#ifndef INSYS_BUS_HPP
#define INSYS_BUS_HPP

#include <stdexcept>
#include <boost/shared_ptr.hpp>

#include "bus_address.hpp"

namespace insys { namespace bus {


struct address;

struct bus_error : std::runtime_error {
    bus_error(const std::string& what_arg) throw() : std::runtime_error("bus: " + what_arg) {}
};

struct timeout_error : bus_error {
    timeout_error(const std::string& what_arg) throw() : bus_error("timeout " + what_arg) {}
};

struct ibus {
    typedef uint32_t value_type;
    virtual value_type read(const address&) = 0;
    virtual void write(const address&, value_type) = 0;
    virtual ~ibus() {};
    template <typename Iterator>
    void write(address addr, Iterator first, Iterator last) {
        while (first != last) {
            write(addr++, *first++);
        }
    }

    template <typename Iterator>
    void read(address addr, Iterator first, Iterator last) {
        while (first != last) {
            *first++ = read(addr++);
        }
    }
};

typedef boost::shared_ptr<ibus> bus_ptr;

}} //namespace insys::bus

#endif //INSYS_BUS_HPP
