#ifndef INSYS_BOARD_HPP
#define INSYS_BOARD_HPP

#include <string>
#include <boost/shared_ptr.hpp>

namespace insys { namespace board {

struct iboard {
    virtual void reset(bool flash_boot = false) = 0;
    virtual void start() = 0;
    virtual void load(const std::string& path, int argc, char* argv[]) = 0;
    virtual ~iboard() {};
};

typedef boost::shared_ptr<iboard> board_ptr;

}} //namespace insys::board

#endif //INSYS_BOARD_HPP
