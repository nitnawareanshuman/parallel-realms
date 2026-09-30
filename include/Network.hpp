#pragma once
#include "Protocol.hpp"
#include <SFML/Network.hpp>
#include <atomic>
#include <condition_variable>
#include <mutex>
#include <string>
#include <thread>

class NetworkServer {
public:
    struct Inputs {
        std::array<Protocol::Input, 2> players{};
        bool ready{};
        std::uint64_t membership{}, resets{};
    };
    explicit NetworkServer(unsigned short port);
    ~NetworkServer();
    unsigned short port() const { return port_; }
    Inputs inputs();
    void publish(const Protocol::State& state);
private:
    void loop();
    sf::TcpListener listener_;
    unsigned short port_{};
    std::atomic<bool> running_{true};
    std::mutex mutex_;
    std::condition_variable wake_;
    Inputs inputs_;
    Protocol::State state_;
    std::uint64_t revision_{};
    std::thread worker_;
};

class NetworkClient {
public:
    struct View {
        Protocol::State state;
        unsigned player{};
        bool received{}, disconnected{};
        std::string message{"Connecting..."};
    };
    NetworkClient(sf::IpAddress address, unsigned short port);
    ~NetworkClient();
    void submit(const Protocol::Input& input);
    View view();
private:
    void loop(sf::IpAddress address, unsigned short port);
    std::atomic<bool> running_{true};
    std::mutex mutex_;
    std::condition_variable wake_;
    Protocol::Input input_;
    View view_;
    std::thread worker_;
};
