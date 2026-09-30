#include "Game.hpp"

#include "Constants.hpp"

#include <algorithm>
#include <cmath>
#include <string>
#include <vector>

namespace {
std::vector<std::string> makeLeftRoom() {
    return {
        "##############",
        "#1...........#",
        "#....####....#",
        "#............#",
        "#..S.........#",
        "#.......###..#",
        "#............#",
        "#....####D...#",
        "#............#",
        "#..#####.....#",
        "#............#",
        "#......###...#",
        "#..........E.#",
        "#............#",
        "##############",
    };
}

std::vector<std::string> makeRightRoom() {
    return {
        "##############",
        "#2...........#",
        "#......###...#",
        "#............#",
        "#.........S..#",
        "#..####......#",
        "#............#",
        "#...D####....#",
        "#............#",
        "#.....#####..#",
        "#............#",
        "#..###.......#",
        "#.E..........#",
        "#............#",
        "##############",
    };
}
} // namespace

Game::Game()
    : window_(sf::VideoMode({Constants::WindowWidth, Constants::WindowHeight}),
              "Parallel Realms - Day 1"),
      leftLevel_(makeLeftRoom(), {0.f, 0.f}),
      rightLevel_(makeRightRoom(),
                  {Constants::RoomColumns * Constants::TileSize, 0.f}),
      playerOneSpawn_(leftLevel_.spawnPosition('1')),
      playerTwoSpawn_(rightLevel_.spawnPosition('2')),
      playerOne_(playerOneSpawn_, sf::Color(66, 153, 225),
                 {sf::Keyboard::Key::W, sf::Keyboard::Key::S,
                  sf::Keyboard::Key::A, sf::Keyboard::Key::D}),
      playerTwo_(playerTwoSpawn_, sf::Color(244, 96, 108),
                 {sf::Keyboard::Key::Up, sf::Keyboard::Key::Down,
                  sf::Keyboard::Key::Left, sf::Keyboard::Key::Right}) {
    window_.setVerticalSyncEnabled(true);
    hazardSystem_.start();
}

Game::~Game() {
    hazardSystem_.stop();
}

void Game::run() {
    sf::Clock clock;

    while (window_.isOpen()) {
        processEvents();
        const float deltaSeconds =
            std::min(clock.restart().asSeconds(), 1.f / 20.f);
        update(deltaSeconds);
        render();
    }
}

void Game::processEvents() {
    while (const auto event = window_.pollEvent()) {
        if (event->is<sf::Event::Closed>()) {
            window_.close();
        }

        if (const auto* key = event->getIf<sf::Event::KeyPressed>()) {
            if (key->code == sf::Keyboard::Key::Escape) {
                window_.close();
            }
            if (key->code == sf::Keyboard::Key::R) {
                leftSwitchActivated_ = false;
                rightSwitchActivated_ = false;
                playerOneFinished_ = false;
                playerTwoFinished_ = false;
                gameWon_ = false;
                resetPlayers();
            }
        }
    }
}

void Game::update(float deltaSeconds) {
    if (gameWon_) {
        return;
    }

    // The left room's door is controlled by Player 2's switch and vice versa.
    playerOne_.update(deltaSeconds, leftLevel_, rightSwitchActivated_);
    playerTwo_.update(deltaSeconds, rightLevel_, leftSwitchActivated_);

    updateObjectives();
    checkHazardCollisions();
}

void Game::render() {
    window_.clear(sf::Color(13, 17, 28));
    leftLevel_.draw(window_, rightSwitchActivated_);
    rightLevel_.draw(window_, leftSwitchActivated_);

    for (const auto& hazard : hazardSystem_.snapshot()) {
        sf::CircleShape circle(hazard.radius);
        circle.setOrigin({hazard.radius, hazard.radius});
        circle.setPosition(hazard.position);
        circle.setFillColor(sf::Color(255, 126, 36));
        circle.setOutlineColor(sf::Color(255, 225, 138));
        circle.setOutlineThickness(2.f);
        window_.draw(circle);
    }

    if (!playerOneFinished_) {
        playerOne_.draw(window_);
    }
    if (!playerTwoFinished_) {
        playerTwo_.draw(window_);
    }

    window_.display();
}

void Game::updateObjectives() {
    if (leftLevel_.touches('S', playerOne_.position(), playerOne_.size())) {
        leftSwitchActivated_ = true;
    }
    if (rightLevel_.touches('S', playerTwo_.position(), playerTwo_.size())) {
        rightSwitchActivated_ = true;
    }

    if (rightSwitchActivated_ &&
        leftLevel_.touches('E', playerOne_.position(), playerOne_.size())) {
        playerOneFinished_ = true;
    }
    if (leftSwitchActivated_ &&
        rightLevel_.touches('E', playerTwo_.position(), playerTwo_.size())) {
        playerTwoFinished_ = true;
    }

    gameWon_ = playerOneFinished_ && playerTwoFinished_;

    std::string title = "Parallel Realms | P1: WASD  P2: Arrows  R: Reset | ";
    if (gameWon_) {
        title += "BOTH REALMS ESCAPED!";
    } else {
        title += leftSwitchActivated_ ? "Blue switch ON" : "Find blue switch";
        title += " | ";
        title += rightSwitchActivated_ ? "Red switch ON" : "Find red switch";
    }
    window_.setTitle(title);
}

void Game::checkHazardCollisions() {
    for (const auto& hazard : hazardSystem_.snapshot()) {
        if ((!playerOneFinished_ && playerTouchesHazard(playerOne_, hazard)) ||
            (!playerTwoFinished_ && playerTouchesHazard(playerTwo_, hazard))) {
            resetPlayers();
            return;
        }
    }
}

bool Game::playerTouchesHazard(const Player& player,
                               const HazardSnapshot& hazard) const {
    const auto position = player.position();
    const auto size = player.size();
    const float closestX =
        std::clamp(hazard.position.x, position.x, position.x + size.x);
    const float closestY =
        std::clamp(hazard.position.y, position.y, position.y + size.y);
    const float deltaX = hazard.position.x - closestX;
    const float deltaY = hazard.position.y - closestY;
    return deltaX * deltaX + deltaY * deltaY < hazard.radius * hazard.radius;
}

void Game::resetPlayers() {
    playerOne_.reset(playerOneSpawn_);
    playerTwo_.reset(playerTwoSpawn_);
    playerOneFinished_ = false;
    playerTwoFinished_ = false;
}


