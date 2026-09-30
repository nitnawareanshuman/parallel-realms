#include "Level.hpp"

#include "Constants.hpp"

#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <utility>

Level::Level(std::vector<std::string> tiles, sf::Vector2f origin)
    : tiles_(std::move(tiles)), origin_(origin) {
    if (tiles_.size() != static_cast<std::size_t>(Constants::RoomRows)) {
        throw std::runtime_error("Level must contain exactly 15 rows");
    }

    for (const auto& row : tiles_) {
        if (row.size() != static_cast<std::size_t>(Constants::RoomColumns)) {
            throw std::runtime_error("Every level row must contain 14 tiles");
        }
    }
}

void Level::draw(sf::RenderWindow& window, bool doorOpen) const {
    for (int row = 0; row < Constants::RoomRows; ++row) {
        for (int column = 0; column < Constants::RoomColumns; ++column) {
            const char tile = tileAt(column, row);
            sf::RectangleShape shape({Constants::TileSize - 1.f,
                                      Constants::TileSize - 1.f});
            shape.setPosition({origin_.x + column * Constants::TileSize,
                               origin_.y + row * Constants::TileSize});

            switch (tile) {
            case '#':
                shape.setFillColor(sf::Color(47, 55, 78));
                break;
            case 'D':
                shape.setFillColor(doorOpen ? sf::Color(45, 130, 88, 70)
                                            : sf::Color(137, 72, 184));
                break;
            case 'S':
                shape.setFillColor(sf::Color(244, 197, 66));
                break;
            case 'E':
                shape.setFillColor(sf::Color(54, 211, 153));
                break;
            default:
                shape.setFillColor(sf::Color(22, 27, 39));
                break;
            }

            window.draw(shape);
        }
    }
}

bool Level::collides(const sf::Vector2f& position, const sf::Vector2f& size,
                     bool doorOpen) const {
    const int firstColumn = static_cast<int>(
        std::floor((position.x - origin_.x) / Constants::TileSize));
    const int lastColumn = static_cast<int>(
        std::floor((position.x + size.x - 0.001f - origin_.x) /
                   Constants::TileSize));
    const int firstRow = static_cast<int>(
        std::floor((position.y - origin_.y) / Constants::TileSize));
    const int lastRow = static_cast<int>(
        std::floor((position.y + size.y - 0.001f - origin_.y) /
                   Constants::TileSize));

    for (int row = firstRow; row <= lastRow; ++row) {
        for (int column = firstColumn; column <= lastColumn; ++column) {
            const char tile = tileAt(column, row);
            if (tile == '#' || (tile == 'D' && !doorOpen)) {
                return true;
            }
        }
    }
    return false;
}

bool Level::touches(char target, const sf::Vector2f& position,
                    const sf::Vector2f& size) const {
    for (int row = 0; row < Constants::RoomRows; ++row) {
        for (int column = 0; column < Constants::RoomColumns; ++column) {
            if (tileAt(column, row) == target &&
                overlapsTile(position, size, column, row)) {
                return true;
            }
        }
    }
    return false;
}

sf::Vector2f Level::spawnPosition(char spawnMarker) const {
    for (int row = 0; row < Constants::RoomRows; ++row) {
        for (int column = 0; column < Constants::RoomColumns; ++column) {
            if (tileAt(column, row) == spawnMarker) {
                const float padding =
                    (Constants::TileSize - Constants::PlayerSize) / 2.f;
                return {origin_.x + column * Constants::TileSize + padding,
                        origin_.y + row * Constants::TileSize + padding};
            }
        }
    }
    throw std::runtime_error("Player spawn marker is missing");
}

char Level::tileAt(int column, int row) const {
    if (column < 0 || column >= Constants::RoomColumns || row < 0 ||
        row >= Constants::RoomRows) {
        return '#';
    }
    return tiles_[static_cast<std::size_t>(row)]
                 [static_cast<std::size_t>(column)];
}

bool Level::overlapsTile(const sf::Vector2f& position,
                         const sf::Vector2f& size, int column, int row) const {
    const float tileLeft = origin_.x + column * Constants::TileSize;
    const float tileTop = origin_.y + row * Constants::TileSize;
    const float tileRight = tileLeft + Constants::TileSize;
    const float tileBottom = tileTop + Constants::TileSize;

    return position.x < tileRight && position.x + size.x > tileLeft &&
           position.y < tileBottom && position.y + size.y > tileTop;
}

