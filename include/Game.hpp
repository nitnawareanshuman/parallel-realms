#pragma once

#include "HazardSystem.hpp"
#include "Level.hpp"
#include "Player.hpp"

#include <SFML/Graphics.hpp>

using namespace sf;

class Game {
public:
    Game();
    ~Game();

    void run();

private:
    void processEvents();
    void update(float deltaSeconds);
    void render();
    void updateObjectives();
    void checkHazardCollisions();
    bool playerTouchesHazard(const Player& player,
                             const HazardSnapshot& hazard) const;
    void resetPlayers();

    RenderWindow window_;
    Level leftLevel_;
    Level rightLevel_;

    Vector2f playerOneSpawn_;
    Vector2f playerTwoSpawn_;

    Player playerOne_;
    Player playerTwo_;
    HazardSystem hazardSystem_;

    bool leftSwitchActivated_{false};
    bool rightSwitchActivated_{false};
    bool playerOneFinished_{false};
    bool playerTwoFinished_{false};
    bool gameWon_{false};
};
