#pragma once
#include "Level.hpp"
#include "Player.hpp"
#include "Protocol.hpp"
#include <array>

// No keyboard polling or window: the server alone owns gameplay decisions.
class World {
public:
    World();
    void reset();
    void tick(const std::array<Protocol::Input, 2>& inputs,
              const std::vector<HazardSnapshot>& hazards, bool ready);
    Protocol::State state() const { return state_; }
private:
    std::array<Level, 2> levels_;
    std::array<Player, 2> players_;
    std::array<sf::Vector2f, 2> spawns_;
    Protocol::State state_;
};
