#include <cstdlib>
#include <cassert>
#include <iostream>
#include <string>
#include <exception>
#include <stdexcept>

#include <boost/filesystem/path.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/lexical_cast.hpp>

#include "b101e1ngu.hpp"


int main(int argc, char* argv[]) {
    try {
        namespace fs = boost::filesystem;
        if (argc < 2 || 
            std::string(argv[1]) == "-h" ||
            std::string(argv[1]) == "--help") {
            fs::path program(argv[0]);
            std::cout << "Usage: "
                      << program.filename()
                      << " address[:port] [dxepath dxeargs...]"
                      << std::endl;
            std::exit(EXIT_SUCCESS);
        }
    
        insys::board::board_ptr board(new insys::board::b101e1ngu(argv[1]));
        bool flashboot = argc < 3;
        board->reset(flashboot);
        if (!flashboot) {
            board->load(argv[2], argc - 3, argv + 3);
            board->start();
        }
    } catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
        std::exit(EXIT_FAILURE);
    }
    return EXIT_SUCCESS;
}







