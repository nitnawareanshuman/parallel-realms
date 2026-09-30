#include "Protocol.hpp"
#include <cmath>

namespace Protocol {
namespace {
void putPosition(sf::Packet& p, sf::Vector2f v) {
    // Integer millipixels use SFML's endian-safe integer serialization.
    p << static_cast<std::int32_t>(std::lround(v.x * 1000.f))
      << static_cast<std::int32_t>(std::lround(v.y * 1000.f));
}
bool getPosition(sf::Packet& p, sf::Vector2f& v) {
    std::int32_t x{}, y{};
    p >> x >> y;
    if (!p || x < 0 || x > 896000 || y < 0 || y > 480000) return false;
    v = {x / 1000.f, y / 1000.f};
    return true;
}
}
sf::Packet encodeInput(const Input& i) {
    sf::Packet p;
    const auto bits = static_cast<std::uint8_t>(i.up | (i.down << 1) | (i.left << 2) | (i.right << 3));
    p << Version << std::uint8_t{1} << bits << i.reset;
    return p;
}
bool decodeInput(sf::Packet p, Input& i) {
    std::uint32_t version{}, reset{};
    std::uint8_t kind{}, bits{};
    p >> version >> kind >> bits >> reset;
    if (!p || !p.endOfPacket() || version != Version || kind != 1 || bits > 15) return false;
    i = {bool(bits & 1), bool(bits & 2), bool(bits & 4), bool(bits & 8), reset};
    return true;
}
sf::Packet encodeState(const State& s, unsigned player) {
    sf::Packet p;
    p << Version << std::uint8_t{2} << static_cast<std::uint8_t>(player);
    for (auto v : s.players) putPosition(p, v);
    for (auto h : s.hazards) putPosition(p, h.position);
    p << s.switches[0] << s.switches[1] << s.finished[0] << s.finished[1]
      << s.ready << s.won << s.hits << s.round;
    return p;
}
bool decodeState(sf::Packet p, State& state, unsigned& player) {
    State s;
    std::uint32_t version{};
    std::uint8_t kind{}, id{};
    p >> version >> kind >> id;
    if (!p || version != Version || kind != 2 || id > 1) return false;
    for (auto& v : s.players) if (!getPosition(p, v)) return false;
    for (auto& h : s.hazards) { if (!getPosition(p, h.position)) return false; h.radius = 9.f; }
    p >> s.switches[0] >> s.switches[1] >> s.finished[0] >> s.finished[1]
      >> s.ready >> s.won >> s.hits >> s.round;
    if (!p || !p.endOfPacket()) return false;
    state = s; player = id;
    return true;
}
}
