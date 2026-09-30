#include "HazardSystem.hpp"

#include "Constants.hpp"

#include <algorithm>
#include <chrono>
#include <cmath>

HazardSystem::HazardSystem() {
    constexpr float roomWidth = Constants::RoomColumns * Constants::TileSize;

    hazards_.push_back({{5.f * Constants::TileSize, 8.5f * Constants::TileSize},
                        {90.f, 0.f}, 2.f * Constants::TileSize,
                        11.f * Constants::TileSize, 9.f});
    hazards_.push_back(
        {{roomWidth + 4.f * Constants::TileSize,
          10.5f * Constants::TileSize},
         {110.f, 0.f}, roomWidth + 2.f * Constants::TileSize,
         roomWidth + 11.f * Constants::TileSize, 9.f});
}

HazardSystem::~HazardSystem() {
    stop();
}

void HazardSystem::start() {
    if (running_.exchange(true)) {
        return;
    }
    simulationThread_ = std::thread(&HazardSystem::simulationLoop, this);
}

void HazardSystem::stop() {
    running_ = false;
    if (simulationThread_.joinable()) {
        simulationThread_.join();
    }
}

std::vector<HazardSnapshot> HazardSystem::snapshot() const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<HazardSnapshot> result;
    result.reserve(hazards_.size());
    for (const auto& hazard : hazards_) {
        result.push_back({hazard.position, hazard.radius});
    }
    return result;
}

void HazardSystem::simulationLoop() {
    using clock = std::chrono::steady_clock;
    auto previous = clock::now();

    while (running_) {
        const auto now = clock::now();
        const float deltaSeconds =
            std::chrono::duration<float>(now - previous).count();
        previous = now;

        {
            std::lock_guard<std::mutex> lock(mutex_);
            for (auto& hazard : hazards_) {
                hazard.position += hazard.velocity * deltaSeconds;
                if (hazard.position.x < hazard.minX) {
                    hazard.position.x = hazard.minX;
                    hazard.velocity.x = std::abs(hazard.velocity.x);
                } else if (hazard.position.x > hazard.maxX) {
                    hazard.position.x = hazard.maxX;
                    hazard.velocity.x = -std::abs(hazard.velocity.x);
                }
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(8));
    }
}
