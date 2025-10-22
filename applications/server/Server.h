#ifndef GRAPA_SERVER_H
#define GRAPA_SERVER_H

#include <string>
#include <memory>
#include <vector>
#include <limits>
#include <boost/asio.hpp>
#include "Message.h"
#include "SvMap.h"
#include "WeatherType.h"

class Server {
public:
    Server(unsigned int port, const std::string& address, int numPlayers);

    [[noreturn]] void run();

protected:
    void connectWithPlayers();

    void receiveAndSend(int i);
    void doSend(int i);

    void shutdown();

    void updateWeather();
    static WeatherType randWeather(WeatherType currentWeather);
    void spawnOrRemoveNpcs();

    void updateMovables(int i);

    void handleMsg(const msg::Msg& msg, int i);
    void handleSvLogout(const msg::Msg& msg, int i);
    void handleSvUpdatePlayer(const msg::Msg& msg, int i);
    void handleSvUpdateHp(const msg::Msg& msg, int i);
    void handleSvDig(const msg::Msg& msg, int i);
    void handleSvPlace(const msg::Msg& msg, int i);
    void handleSvAddProjectile(const msg::Msg& msg, int i);
    void handleSvUpdateProjectile(const msg::Msg& msg, int i);


    //
    // CREATE MESSAGES FOR CLIENTS
    //

    void addInitialMapNpcStateTo(int i);

public:
    static void addMsgToAll(const msg::Msg& msg, int except = -1);
    static void addMsgTo(const msg::Msg& msg, int i);

protected:
    double lastUpdateMs = 0;
    Timer gameTimer;

    int minId = -1, maxId = -1, nextId = -1;

    std::map<WeatherType, int> numCloudsPerWeather;

    SvMap map;
    inline static std::map<int,std::vector<msg::Msg>> msgQueue;

    boost::asio::io_service ioService;
    boost::asio::ip::tcp::acceptor acceptor;
    // socket does not have a default constructor, thus we use pointers
    std::vector<std::unique_ptr<boost::asio::ip::tcp::socket>> sockets;
    std::vector<bool> connected;

public:
    /*
     * In OO programming it is typical to prohibit copying of objects. <br>
     * <br>
     * Jede Klasse, deren Klassendaten Zeiger verwenden, sollte selbst-
     * erstellte Kopierkonstruktoren (copy constructor) und Zuweisungs-Operatoren (assignment operator) enthal-
     * ten. Ansonsten sollte man sie unwirksam machen [...]. <br>
     * <br>
     * ISBN 3-8348-0125-9, May 2006, Grundkurs Software Entwicklung mit C++, 2. Auflage
     * <br>
     * boost::non_copyable or delete copy constructor and assignment operator? https://stackoverflow.com/a/7841332/6334421
     */

    Server(const Server &obj) = delete;

    Server &operator=(const Server &obj) = delete;
};

#endif //GRAPA_SERVER_H
