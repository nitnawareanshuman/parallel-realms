#pragma once

#include <SFML/Graphics.hpp>
using namespace sf;

class Level;

struct PlayerControls {
    Keyboard::Key up;
    Keyboard::Key down;
    Keyboard::Key left;
    Keyboard::Key right;
};

class Player {
public:
    Player(Vector2f spawn, Color color, PlayerControls controls);

    void update(float deltaSeconds, const Level& level, bool doorOpen);
    void draw(RenderWindow& window) const;
    void reset(Vector2f spawn);

    Vector2f position() const;
    Vector2f size() const;

private:
    void tryMove(Vector2f movement, const Level& level, bool doorOpen);

    Vector2f position_;
    Color color_;
    PlayerControls controls_;
};

