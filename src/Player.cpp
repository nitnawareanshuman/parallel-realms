#include "Player.hpp"

#include "Constants.hpp"
#include "Level.hpp"

#include <cmath>

Player::Player(sf::Vector2f spawn, sf::Color color, PlayerControls controls)
    : position_(spawn), color_(color), controls_(controls) {}

void Player::update(float deltaSeconds, const Level& level, bool doorOpen) {
    sf::Vector2f direction{0.f, 0.f};

    if (sf::Keyboard::isKeyPressed(controls_.up)) {
        direction.y -= 1.f;
    }
    if (sf::Keyboard::isKeyPressed(controls_.down)) {
        direction.y += 1.f;
    }
    if (sf::Keyboard::isKeyPressed(controls_.left)) {
        direction.x -= 1.f;
    }
    if (sf::Keyboard::isKeyPressed(controls_.right)) {
        direction.x += 1.f;
    }

    move(direction, deltaSeconds, level, doorOpen);
}

void Player::move(sf::Vector2f direction, float deltaSeconds, const Level& level, bool doorOpen) {
    const float length =
        std::sqrt(direction.x * direction.x + direction.y * direction.y);
    if (length > 0.f) {
        direction.x /= length;
        direction.y /= length;
    }

    const sf::Vector2f movement =
        direction * (Constants::PlayerSpeed * deltaSeconds);
    tryMove({movement.x, 0.f}, level, doorOpen);
    tryMove({0.f, movement.y}, level, doorOpen);
}

void Player::draw(sf::RenderWindow& window) const {
    sf::RectangleShape shape(size());
    shape.setPosition(position_);
    shape.setFillColor(color_);
    shape.setOutlineColor(sf::Color::White);
    shape.setOutlineThickness(2.f);
    window.draw(shape);
}

void Player::reset(sf::Vector2f spawn) {
    position_ = spawn;
}

sf::Vector2f Player::position() const {
    return position_;
}

sf::Vector2f Player::size() const {
    return {Constants::PlayerSize, Constants::PlayerSize};
}

void Player::tryMove(sf::Vector2f movement, const Level& level,
                     bool doorOpen) {
    const sf::Vector2f candidate = position_ + movement;
    if (!level.collides(candidate, size(), doorOpen)) {
        position_ = candidate;
    }
}

