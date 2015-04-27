#ifndef INSYS_UDP_BUS
#define INSYS_UDP_BUS

#include <boost/smart_ptr.hpp>
#include "ibus.hpp"

namespace insys { namespace bus {

struct udp_bus : ibus {
    explicit udp_bus(const std::string& host, unsigned short port = 3001);
    value_type read(const address&);
    void write(const address&, value_type);
    struct bus_impl;
private:
    boost::shared_ptr<bus_impl> pimpl_;
};


}} //namespace insys::bus

#endif //INSYS_UDP_BUS
