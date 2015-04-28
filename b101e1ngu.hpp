#ifndef BRD_B101E1NGU_HPP
#define BRD_B101E1NGU_HPP

#include <stdexcept>

#include <boost/shared_ptr.hpp>

#include "iboard.hpp"

namespace brd { namespace board {

struct board_error : std::runtime_error {
    board_error(const std::string& name, const std::string& what_arg) throw() 
        : std::runtime_error("board " + name + ": " + what_arg) {}
};

struct b101e1ngu : iboard {
    explicit b101e1ngu(const std::string& netaddr);
    void reset(bool flash_boot);
    void start();
    void load(const std::string& path, int argc, char* argv[]);
    struct board_impl;
private:
    boost::shared_ptr<board_impl> pimpl_;
};

}} //namespace brd::board

#endif //BRD_B101E1NGU_HPP
