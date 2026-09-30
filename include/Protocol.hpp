#pragma once
#include "HazardSystem.hpp"
#include <SFML/Network/Packet.hpp>
#include <array>
#include <cstdint>

namespace Protocol {
constexpr std::uint32_t Version = 2;
constexpr unsigned short DefaultPort = 53000;
struct Input {
    bool up{}, down{}, left{}, right{};
    std::uint32_t reset{}; // Monotonic request counter, not a held key.
};
struct State {
    std::array<sf::Vector2f, 2> players{};
    std::array<HazardSnapshot, 2> hazards{};
    std::array<bool, 2> switches{}, finished{};
    bool ready{}, won{};
    std::uint32_t hits{}, round{};
};
sf::Packet encodeInput(const Input& input);
bool decodeInput(sf::Packet packet, Input& input);
sf::Packet encodeState(const State& state, unsigned player);
bool decodeState(sf::Packet packet, State& state, unsigned& player);
}
