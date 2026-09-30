#pragma once

#include <SFML/Graphics.hpp>

class Level;

struct PlayerControls {
    sf::Keyboard::Key up;
    sf::Keyboard::Key down;
    sf::Keyboard::Key left;
    sf::Keyboard::Key right;
};

class Player {
public:
    Player(sf::Vector2f spawn, sf::Color color, PlayerControls controls);

    void update(float deltaSeconds, const Level& level, bool doorOpen);
    void move(sf::Vector2f direction, float deltaSeconds, const Level& level, bool doorOpen);
    void draw(sf::RenderWindow& window) const;
    void reset(sf::Vector2f spawn);

    sf::Vector2f position() const;
    sf::Vector2f size() const;

private:
    void tryMove(sf::Vector2f movement, const Level& level, bool doorOpen);

    sf::Vector2f position_;
    sf::Color color_;
    PlayerControls controls_;
};

