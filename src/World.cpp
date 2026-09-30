#include "World.hpp"
#include "Constants.hpp"
#include "Rooms.hpp"
#include <algorithm>

World::World()
    : levels_{Level(Rooms::makeLeftRoom(), {0.f, 0.f}),
              Level(Rooms::makeRightRoom(), {448.f, 0.f})},
      players_{Player({}, sf::Color(66,153,225), {}), Player({}, sf::Color(244,96,108), {})},
      spawns_{levels_[0].spawnPosition('1'), levels_[1].spawnPosition('2')} { reset(); }
void World::reset() {
    const auto round = state_.round + 1;
    state_ = {};
    state_.round = round;
    for (unsigned i = 0; i < 2; ++i) {
        players_[i].reset(spawns_[i]);
        state_.players[i] = spawns_[i];
    }
}
void World::tick(const std::array<Protocol::Input, 2>& inputs,
                 const std::vector<HazardSnapshot>& hazards, bool ready) {
    state_.ready = ready;
    for (unsigned i = 0; i < std::min<std::size_t>(2, hazards.size()); ++i) state_.hazards[i] = hazards[i];
    if (!ready || state_.won) return;
    for (unsigned i = 0; i < 2; ++i) {
        if (state_.finished[i]) continue;
        const auto& in = inputs[i];
        players_[i].move({float(in.right) - float(in.left), float(in.down) - float(in.up)},
                          1.f / 60.f, levels_[i], state_.switches[1-i]);
    }
    // Resolve hazards before objectives, using exactly the snapshot sent to clients.
    bool hit = false;
    for (unsigned i = 0; i < 2; ++i) {
        if (state_.finished[i]) continue;
        const auto p = players_[i].position();
        for (auto h : state_.hazards) {
            const float dx = h.position.x - std::clamp(h.position.x, p.x, p.x + Constants::PlayerSize);
            const float dy = h.position.y - std::clamp(h.position.y, p.y, p.y + Constants::PlayerSize);
            hit |= dx*dx + dy*dy < h.radius*h.radius;
        }
    }
    if (hit) {
        ++state_.hits;
        for (unsigned i = 0; i < 2; ++i) players_[i].reset(spawns_[i]);
        state_.finished = {};
    } else {
        for (unsigned i = 0; i < 2; ++i) {
            if (levels_[i].touches('S', players_[i].position(), players_[i].size())) state_.switches[i] = true;
        }
        for (unsigned i = 0; i < 2; ++i) {
            if (state_.switches[1-i] && levels_[i].touches('E', players_[i].position(), players_[i].size()))
                state_.finished[i] = true;
        }
    }
    for (unsigned i = 0; i < 2; ++i) state_.players[i] = players_[i].position();
    state_.won = state_.finished[0] && state_.finished[1];
}
