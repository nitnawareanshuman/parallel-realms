#pragma once

#include <SFML/Graphics.hpp>

#include <string>
#include <vector>


class Level {
public:
    Level(std::vector<std::string> tiles, sf::Vector2f origin);

    void draw(sf::RenderWindow& window, bool doorOpen) const;
    bool collides(const sf::Vector2f& position, const sf::Vector2f& size, bool doorOpen) const;
    bool touches(char tile, const sf::Vector2f& position, const sf::Vector2f& size) const;
    sf::Vector2f spawnPosition(char spawnMarker) const;

private:
    char tileAt(int column, int row) const;
    bool overlapsTile(const sf::Vector2f& position, const sf::Vector2f& size, int column, int row) const;

    std::vector<std::string> tiles_;
    sf::Vector2f origin_;
};

