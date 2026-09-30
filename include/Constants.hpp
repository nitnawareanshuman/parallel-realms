#pragma once

namespace Constants {
constexpr float TileSize = 32.f;
constexpr int RoomColumns = 14;
constexpr int RoomRows = 15;
constexpr float PlayerSize = 22.f;
constexpr float PlayerSpeed = 180.f;
constexpr unsigned int WindowWidth =
    static_cast<unsigned int>(RoomColumns * 2 * TileSize);
constexpr unsigned int WindowHeight =
    static_cast<unsigned int>(RoomRows * TileSize);
} // namespace Constants

