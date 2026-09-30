#include "Network.hpp"
#include "World.hpp"
#include <chrono>
#include <csignal>
#include <exception>
#include <iostream>
#include <stdexcept>
#include <thread>
namespace { volatile std::sig_atomic_t stopped = 0; void stop(int) { stopped = 1; } }
int main(int argc, char** argv) {
    try {
        unsigned short port = Protocol::DefaultPort;
        if (argc > 2) throw std::runtime_error("Usage: parallel_realms_server [port]");
        if (argc == 2) {
            const std::string value = argv[1]; std::size_t used{};
            const int number = std::stoi(value, &used);
            if (used != value.size() || number < 1 || number > 65535) throw std::runtime_error("Invalid port");
            port = static_cast<unsigned short>(number);
        }
        std::signal(SIGINT, stop); std::signal(SIGTERM, stop);
        World world;
        HazardSystem hazards;
        NetworkServer network(port);
        hazards.start();
        std::cout << "Parallel Realms server | TCP " << port << " | Ctrl+C to stop\n" << std::flush;
        std::uint64_t membership = 0, resets = 0;
        using Clock = std::chrono::steady_clock;
        auto next = Clock::now();
        while (!stopped) {
            const auto input = network.inputs();
            if (input.membership != membership || input.resets != resets) {
                world.reset(); membership = input.membership; resets = input.resets;
            }
            world.tick(input.players, hazards.snapshot(), input.ready);
            network.publish(world.state());
            next += std::chrono::microseconds(16667);
            if (next < Clock::now() - std::chrono::milliseconds(100)) next = Clock::now();
            std::this_thread::sleep_until(next);
        }
    } catch (const std::exception& e) { std::cerr << e.what() << '\n'; return 1; }
}
