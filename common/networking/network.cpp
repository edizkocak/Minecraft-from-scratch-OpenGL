#include <sstream>
#include <string>

#include "network.h"

using boost::asio::ip::tcp;

namespace networking {
    void send(tcp::socket &socket, const std::vector<msg::Msg>& msgs) {
        nlohmann::json json(msgs);
        std::string json_str = json.dump();
        unsigned size = json_str.size();

        std::ostringstream os;
        os << size << "#" << json_str;
        // TODO uncomment to debug
//        std::cout << "OUT: " << os.str() << std::endl;
        boost::asio::write(socket, boost::asio::buffer(os.str()));
    }

    std::vector<msg::Msg> read(tcp::socket &socket) {
        msg::Msg msg;
        std::string str_header;
        for (unsigned i{0}; i < networking::max_bit_length; i++) {
            char buffer[1], c;
            size_t bytes_read = boost::asio::read(socket, boost::asio::buffer(buffer, 1));
            if (bytes_read < 1) {
                throw std::runtime_error("network error");
            }
            c = buffer[0];
            if (c == '#') {
                break;
            } else {
                str_header += c;
            }
        }

        boost::asio::streambuf buf;
        unsigned bytes_json = std::stoi(str_header);
        size_t bytes_read = boost::asio::read(socket, buf, boost::asio::transfer_exactly(bytes_json));
        if (bytes_read != bytes_json) {
            throw std::runtime_error("read: " + std::to_string(bytes_read) + ", json: " + std::to_string(bytes_json));
        }

        std::string json_str = boost::asio::buffer_cast<const char *>(buf.data());
        if (json_str.length() != bytes_json) {
//            TODO
//            std::cout << "WARNING>> "
//                      << ("actual: " + std::to_string(json_str.length()) + ", expected: " + std::to_string(bytes_json))
//                      << std::endl;

            if (json_str.length() > bytes_json) {
                json_str = json_str.substr(0, bytes_json);
            } else {
                throw std::runtime_error("can't correct broken json string");
            }
        }
        // TODO uncomment to debug
//        std::cout << "IN:  " << str_header << "#" << json_str << std::endl;

        nlohmann::json json = nlohmann::json::parse(json_str);
        if(!json.is_array()){
            throw std::runtime_error("not an array");
        }
        return json.get<std::vector<msg::Msg>>();
    }
}
