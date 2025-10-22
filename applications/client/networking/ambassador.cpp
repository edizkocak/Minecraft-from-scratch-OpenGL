#include "ambassador.h"
#include <thread>
#include <chrono>

using namespace std;
using boost::asio::ip::tcp;

Ambassador::Ambassador(unsigned int port, const std::string& address) {
    try {
        cout << "connecting..." << endl;
        boost::asio::io_service io_service;
        tcp::resolver resolver(io_service);
        tcp::resolver::query query(address, to_string(port));
        tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);
        tcp::resolver::iterator end;

        // socket creation
        socket = std::make_shared<tcp::socket>(io_service);
        boost::system::error_code error = boost::asio::error::host_not_found;
        while (error && endpoint_iterator != end) {
            socket->close();
            socket->connect(*endpoint_iterator++, error);
        }
        if (error) { throw boost::system::system_error(error); }

        cout << "connected." << endl;

    } catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
        string text = " (" + string(e.what()) + ")";
        exit(EXIT_FAILURE);
    }
}

std::vector<msg::Msg> Ambassador::takeIncoming() {
    mtx.lock();
    if (inQueue.empty()) {
        mtx.unlock();
        return {};
    }

    std::vector<msg::Msg> messages = inQueue;
    inQueue.resize(0);
    mtx.unlock();
    return messages;
}

void Ambassador::addOutgoing(const msg::Msg &msg) {
    if (msg.type == msg::MsgType::SV_UPDATE_PLAYER) {
        mtx.lock();
        outPlayerUpdate = make_unique<msg::Msg>(msg);
        mtx.unlock();
    } else {
        mtx.lock();
        outQueue.push_back(msg);
        mtx.unlock();
    }
}

void Ambassador::run() {
    while (sendAndReceive()) {
#ifndef AMBASSADOR_NO_SLEEP
        int millisLeft = interval - (int) intervalTimer.look();
        if (millisLeft > 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(millisLeft));
        }
#endif
    }
}

bool Ambassador::sendAndReceive() {
#ifndef AMBASSADOR_NO_SLEEP
    intervalTimer.begin();
#endif

    // send
    {
        mtx.lock();
        vector<msg::Msg> messages = outQueue;
        outQueue.resize(0);
        if (outPlayerUpdate) {
            messages.push_back(*outPlayerUpdate);
            outPlayerUpdate = nullptr;
        }
        mtx.unlock();

        networking::send(*socket, messages);
    }

    // receive
    {
        vector<msg::Msg> messages = networking::read(*socket);
        if (!messages.empty() && messages.end()->type == msg::MsgType::CL_EXIT) {
            return false;
        }

        mtx.lock();
        inQueue.insert(end(inQueue), begin(messages), end(messages));
        mtx.unlock();
    }

    return true;
}
