#include <iostream>
#include <boost/lexical_cast.hpp>
#include <boost/bind.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/deadline_timer.hpp>
#include <boost/asio/ip/udp.hpp>
#include <boost/range.hpp>

#include "udp_bus.hpp"
#include "spell.hpp"

namespace insys { namespace bus {

/*http://www.boost.org/doc/libs/1_52_0/doc/html/boost_asio/example/timeouts/blocking_udp_client.cpp*/
struct udp_bus::bus_impl {
    typedef boost::asio::io_service io_service;
    typedef boost::asio::deadline_timer deadline_timer;
    typedef boost::asio::ip::udp udp;
    typedef boost::posix_time::time_duration time_duration;

    enum command {
        READMEM = 0x300,
        WRITEMEM = 0x400
    };

    bus_impl( const std::string& host,
              unsigned short port,
              time_duration timeout  = boost::posix_time::seconds(5) )
        : socket_(io_service_, udp::v4())
        , timeout_(timeout)
        , deadline_(io_service_) {
        udp::resolver resolver(io_service_);
        udp::resolver::query query(udp::v4(), host, boost::lexical_cast<std::string>(port));
        udp::endpoint endpoint = *resolver.resolve(query);
        socket_.connect(endpoint);
        check_deadline();
        capture();
    }

    value_type read(const address& addr) { 
        std::vector<value_type> request(8);
        request[0] = 0x12344321;
        request[1] = READMEM;
        request[2] = addr.type();
        request[3] = addr.value();
        request[4] = sizeof(value_type);
        request[5] = 0;
        send(request);

        std::vector<value_type> answer(9);
        receive(answer);
        return answer.back();
    }

    void write(const address& addr, value_type value) {
        std::vector<value_type> request(9);
        request[0] = 0x12344321;
        request[1] = WRITEMEM;
        request[2] = addr.type();
        request[3] = addr.value();
        request[4] = sizeof(value_type);
        request[5] = 0;
        request[8] = value;

        send(request);

        std::vector<value_type> answer(8);
        receive(answer);
    }
private:
    void capture() {
        for (int i = 0; i < 2; ++i) {
            std::vector<int> data(std::begin(spell), std::end(spell));
            send(boost::asio::buffer(data));
            std::fill(data.begin(), data.end(), 0);
            receive(data);
    
            if (!std::equal(std::begin(data), std::end(data), std::begin(spell))) {
                throw std::runtime_error("bus capture fail");
            } else {
                break;
            }
        }
    }
private:

    template <typename T>
    void send( const T& buffer) {
        socket_.send(boost::asio::buffer(buffer));
    }

    template <typename T>
    void receive(T& buffer) {
        boost::system::error_code ec;
        
        std::size_t size = receive(boost::asio::buffer(buffer), timeout_, ec);
        if (ec) {
            throw timeout_error(ec.message());
        }

        if (size/sizeof(typename T::value_type) != buffer.size()) {
            throw bus_error("size error");
        }
    }

private:
    void check_deadline() {
        if (deadline_.expires_at() <= deadline_timer::traits_type::now()) {
            socket_.cancel();
            deadline_.expires_at(boost::posix_time::pos_infin);
        }

        deadline_.async_wait(boost::bind(&bus_impl::check_deadline, this));
    }

    std::size_t receive( const boost::asio::mutable_buffer& buffer,
                         boost::posix_time::time_duration timeout,
                         boost::system::error_code& ec ) {
        deadline_.expires_from_now(timeout);
        ec = boost::asio::error::would_block;
        std::size_t length = 0;

        socket_.async_receive(boost::asio::buffer(buffer),
                              boost::bind(&bus_impl::handle_receive, _1, _2, &ec, &length));

        do io_service_.run_one(); while (ec == boost::asio::error::would_block);
        return length;
    }

    static void handle_receive(const boost::system::error_code& ec, std::size_t length,
                               boost::system::error_code* out_ec, std::size_t* out_length) {
        *out_ec = ec;
        *out_length = length;
    }
private:
    static io_service io_service_;
    udp::socket socket_;
    time_duration timeout_;
    deadline_timer deadline_;
};


udp_bus::bus_impl::io_service udp_bus::bus_impl::io_service_;

udp_bus::udp_bus(const std::string& host, unsigned short port) : pimpl_(new bus_impl(host, port)) {}
udp_bus::value_type udp_bus::read(const address& addr) { return pimpl_->read(addr); }
void udp_bus::write(const address& addr, value_type value) { pimpl_->write(addr, value); }

}} //namespace insys::bus

