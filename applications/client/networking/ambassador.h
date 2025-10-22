#pragma once

#include <queue>
#include <memory>
#include <mutex>
#include <boost/asio.hpp>

#include "cppgl/src/cppgl.h"
#include "Message.h"
#include "network.h"

class Ambassador {
public:
    Ambassador(unsigned int port, const std::string& address);

    /**
     * @return Take all incoming messages.
     */
    std::vector<msg::Msg> takeIncoming();

    /**
     * Add outgoing message.
     */
    void addOutgoing(const msg::Msg &msg);

    /**
     * Called by network thread.
     */
    void run();

    /**
     * Called by network thread.
     * <br>
     * Sends all queued messages to server.
     * <br>
     * Receives all new messages from server.
     *
     * @returns False if CL_EXIT message was received
     */
    bool sendAndReceive();

#ifndef AMBASSADOR_NO_SLEEP
    /**
     * Periodic interval time in ms.
     * <br>
     * 50ms <-> 20x per second
     */
    int interval = 1000; // changed e.g. to 50 during player login
    Timer intervalTimer;
#endif

private:
    std::mutex mtx;

    std::vector<msg::Msg> inQueue;
    std::vector<msg::Msg> outQueue;
    /**
     * holds the latest outgoing "updatePlayer" message
     */
    std::unique_ptr<msg::Msg> outPlayerUpdate;

    // no default constructor so use ptr
    std::shared_ptr<boost::asio::ip::tcp::socket> socket;
};

extern std::shared_ptr<Ambassador> the_ambassador;
