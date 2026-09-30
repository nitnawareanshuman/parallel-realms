#pragma once

#include <SFML/System/Vector2.hpp>

#include <atomic>
#include <mutex>
#include <thread>
#include <vector>


struct HazardSnapshot {
    sf::Vector2f position;
    float radius;
};

class HazardSystem {
public:
    HazardSystem();  // Constructor, it runs automaticall when object is created.
    ~HazardSystem(); // Destructor, it runs automatically when object lifetime ends.

    HazardSystem(const HazardSystem &) = delete; // HazardSystem constructor can't be copied, delete the copy constructor
    HazardSystem &operator=(const HazardSystem &) = delete; // Delete copy assignment e.g. HazardSystem a; HazardSystem b; a = b (error)

    void start();
    void stop();
    std::vector<HazardSnapshot> snapshot() const; // Not modify the HazardSystem object

private:
    struct MovingHazard {
        sf::Vector2f position;
        sf::Vector2f velocity;
        float minX;
        float maxX;
        float radius;
    };

    void simulationLoop();

    mutable std::mutex mutex_;
    std::vector<MovingHazard> hazards_;
    std::atomic<bool> running_{false};
    std::thread simulationThread_;
};

