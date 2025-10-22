#include <algorithm>
#include <thread>

#include <iostream>
#include <boost/program_options.hpp>

#include "network.h"
#include "Server.h"

using namespace std;

namespace {
    unsigned int numPlayers;
    unsigned int port;
    string address;

    void parseArgs(int argc, char *argv[])
    {
        namespace po = boost::program_options;
        po::options_description desc{"Options"};
        desc.add_options()
                ("help,h", "Print usage.")
                ("num_players,n", po::value<unsigned int>()->default_value(1), "Number of players.")
                ("port,p", po::value<unsigned int>()->default_value(networking::server_port), "Port the server listens on.")
                ("address,a", po::value<string>()->default_value(networking::server_address), "Server address.")
        ;

        po::variables_map vm;
        po::store(po::command_line_parser(argc, argv).options(desc).positional({}).run(), vm);
        po::notify(vm);

        if (vm.count("help")) {
            cout << desc << endl;
            exit(EXIT_SUCCESS);
        }

        numPlayers = vm["num_players"].as<unsigned int>();
        port = vm["port"].as<unsigned int>();
        address = vm["address"].as<string>();
    }
}

int main(int argc, char** argv) {
    parseArgs(argc, argv);
    Server server(port, address, int(numPlayers));
    server.run();

    return EXIT_SUCCESS;
}
