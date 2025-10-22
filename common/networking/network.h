#pragma once

#include <boost/asio.hpp>
#include "Message.h"

namespace networking {
    static constexpr unsigned max_bit_length{30};
    static constexpr const char *server_address{"127.0.0.1"};
    static constexpr unsigned int server_port{8080};

    std::vector<msg::Msg> read(boost::asio::ip::tcp::socket &socket);

    void send(boost::asio::ip::tcp::socket &socket, const std::vector<msg::Msg>& msgs);
}
