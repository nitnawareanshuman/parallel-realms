#pragma once

#include <SFML/Graphics.hpp>

#include <string>
#include <vector>

using namespace std;
using namespace sf;

class Level {
public:
    Level(vector<string> tiles, Vector2f origin);

    void draw(RenderWindow& window, bool doorOpen) const;
    bool collides(const Vector2f& position, const Vector2f& size, bool doorOpen) const;
    bool touches(char tile, const Vector2f& position, const Vector2f& size) const;
    Vector2f spawnPosition(char spawnMarker) const;

private:
    char tileAt(int column, int row) const;
    bool overlapsTile(const Vector2f& position, const Vector2f& size, int column, int row) const;

    vector<string> tiles_;
    Vector2f origin_;
};

