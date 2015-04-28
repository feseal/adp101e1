#include <cstdlib>
#include <algorithm>
#include <string>
#include <iostream>

#include <boost/program_options.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/asio.hpp>
#include <boost/range.hpp>

#include "spell.hpp"

namespace impl {
using boost::asio::ip::udp;

void initialize_board_connection(udp::socket& sock, int try_count = 10) {
    bool ready = false;
    for (int i = 0; i < try_count; ++i) {
        std::vector<int> data(std::begin(brd::spell),
                              std::end(brd::spell));
        sock.send(boost::asio::buffer(data));
        std::fill(data.begin(), data.end(), 0);
        sock.receive(boost::asio::buffer(data));
    
        if (std::equal(std::begin(data), std::end(data),
                       std::begin(brd::spell))) {
            ready = true;
            break;
        }
    }

    if (!ready)
        throw std::runtime_error("board initialization fail");
}

}

int main(int argc, char* argv[]) {
    try {
        namespace fs = boost::filesystem;
        namespace po = boost::program_options;

        fs::path path(argv[0]);
        std::string program_name(path.filename().string());

        po::options_description 
            desc("Usage: " + program_name + " [options] address");
        std::string address;
        std::size_t size;
        std::size_t sobuffsize;
        desc.add_options()
            ("help,h", "produce help message")
            ("address,a", po::value<std::string>(&address), 
             "board address, host[:port], default port is 3002")
            ("size,s", po::value<std::size_t>(&size)->default_value(1024), 
             "buffer size in bytes, default: 1024")
            ("bsize,b", 
             po::value<std::size_t>(&sobuffsize)->default_value(4*1024*1024), 
             "socket buffer size in bytes");

        po::positional_options_description p;
        p.add("address", 1);

        po::variables_map vm;
        po::store(po::command_line_parser(argc, argv).
                  options(desc).
                  positional(p).run(), vm);
        po::notify(vm);
        if (vm.count("help") || !vm.count("address")) {
            std::cerr << desc << std::endl;
            std::exit(EXIT_SUCCESS);
        }

        using boost::asio::ip::udp;
        boost::asio::io_service io_service;

        udp::resolver resolver(io_service);
        std::size_t pos = address.find(':');
        std::string host = address.substr(0, pos);
        std::string port = (pos == std::string::npos ? 
                            "3002" : address.substr(pos + 1));
        udp::resolver::query query(udp::v4(), host, port);
        udp::endpoint endpoint = *resolver.resolve(query);
        udp::socket socket(io_service, udp::v4());
        socket.connect(endpoint);
        boost::system::error_code ec;
        boost::asio::socket_base::receive_buffer_size reque_opt(sobuffsize);
        socket.set_option(reque_opt, ec);
        if (ec) {
            throw std::runtime_error(ec.message());
        } else {
            boost::asio::socket_base::receive_buffer_size option;
            socket.get_option(option);
            if (static_cast<std::size_t>(option.value()) != sobuffsize) {
                std::cerr << "warning: socket buffer size is "
                          << option.value() << " bytes" << std::endl
                          << "change /proc/sys/net/core/rmem_max "
                             "to fix this warning" 
                          << std::endl
                          << "e.g: sudo echo 4194304 >"
                             " /proc/sys/net/core/rmem_max" 
                          << std::endl;
            }
        }
        impl::initialize_board_connection(socket);
        std::vector<char> data(size);
        while (true) {
            std::size_t sz = socket.receive(boost::asio::buffer(data));
            std::cout.write(&data[0], sz);
        }
        
    } catch(std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

