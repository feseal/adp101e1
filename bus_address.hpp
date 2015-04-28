#ifndef BRD_BUS_ADDRESS_HPP
#define BRD_BUS_ADDRESS_HPP

#include <ostream>
#include <string>
#include <sstream>
#include <boost/operators.hpp>


namespace brd { namespace bus {

struct address : boost::incrementable<address>
               , boost::addable<address, uint32_t> {
    address(uint32_t type, uint32_t value, uint32_t step = 1)
        : type_(type)
        , value_(value)
        , step_(step) {}

    uint32_t type() const { return type_; }
    uint32_t value() const { return value_; }
    uint32_t step() const { return step_; }
    std::string str() const {
        std::ostringstream os;
        os << "0x" << std::hex << type_ << " 0x" << value_;
        return os.str();
    }
    address& operator++() {
        value_ += step_;
        return *this;
    }
    
    address& operator+=(uint32_t value) {
        value_ += value;
        return *this;
    }
private:
    uint32_t type_;
    uint32_t value_;
    uint32_t step_;
};

inline std::ostream& operator<< (std::ostream& os, const address& addr) {
    os << std::hex << addr.type() << " " 
       << addr.value() << " " 
       << std::dec << addr.step();
    return os;
}

}} //namesapce brd::bus

#endif //BRD_BUS_ADDRESS_HPP
