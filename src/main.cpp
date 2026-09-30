#include "Game.hpp"
#include "NetworkGame.hpp"
#include "Protocol.hpp"
#include <exception>
#include <iostream>
#include <stdexcept>
#include <string>
int main(int argc, char** argv) {
    try {
        if (argc == 1 || (argc==2 && std::string(argv[1])=="--local")) {
            Game game; game.run(); return 0;
        }
        if (std::string(argv[1])=="--help") {
            std::cout << "Local: parallel_realms [--local]\nLAN: parallel_realms --connect IPv4 [port] [--mute]\n"; return 0;
        }
        if (argc<3 || std::string(argv[1])!="--connect" || argc>5)
            throw std::runtime_error("Use --help for usage");
        const auto address=sf::IpAddress::resolve(argv[2]);
        if (!address) throw std::runtime_error("Invalid server address");
        unsigned short port=Protocol::DefaultPort; bool muted=false, hasPort=false;
        for (int i=3;i<argc;++i) {
            const std::string value=argv[i];
            if (value=="--mute") { muted=true; continue; }
            std::size_t used{}; const int number=std::stoi(value,&used);
            if (hasPort || used!=value.size() || number<1 || number>65535) throw std::runtime_error("Invalid port");
            hasPort=true; port=static_cast<unsigned short>(number);
        }
        runNetworkGame(*address,port,muted);
    } catch (const std::exception& error) {
        std::cerr << "Parallel Realms failed: " << error.what() << '\n'; return 1;
    }
}
