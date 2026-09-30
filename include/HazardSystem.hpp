#pragma once

#include <SFML/System/Vector2.hpp>

#include <atomic>
#include <mutex>
#include <thread>
#include <vector>

using namespace std;
using namespace sf;

struct HazardSnapshot {
    Vector2f position;
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
    vector<HazardSnapshot> snapshot() const; // Not modify the HazardSystem object

private:
    struct MovingHazard {
        Vector2f position;
        Vector2f velocity;
        float minX;
        float maxX;
        float radius;
    };

    void simulationLoop();

    mutable mutex mutex_;
    vector<MovingHazard> hazards_;
    atomic<bool> running_{false};
    thread simulationThread_;
};

