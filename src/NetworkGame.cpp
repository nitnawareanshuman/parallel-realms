#include "NetworkGame.hpp"
#include "Constants.hpp"
#include "Level.hpp"
#include "Network.hpp"
#include "Rooms.hpp"
#include <SFML/Audio.hpp>
#include <array>
#include <cmath>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace {
// Tiny built-in 5x7 alphabet: no external font or working-directory dependency.
void text(sf::RenderWindow& window, const std::string& value, sf::Vector2f at, float scale, sf::Color color) {
    static const std::array<std::array<unsigned,7>,26> glyphs{{
        {{14,17,17,31,17,17,17}},{{30,17,17,30,17,17,30}},{{14,17,16,16,16,17,14}},
        {{30,17,17,17,17,17,30}},{{31,16,16,30,16,16,31}},{{31,16,16,30,16,16,16}},
        {{14,17,16,23,17,17,15}},{{17,17,17,31,17,17,17}},{{31,4,4,4,4,4,31}},
        {{7,2,2,2,2,18,12}},{{17,18,20,24,20,18,17}},{{16,16,16,16,16,16,31}},
        {{17,27,21,21,17,17,17}},{{17,25,21,19,17,17,17}},{{14,17,17,17,17,17,14}},
        {{30,17,17,30,16,16,16}},{{14,17,17,17,21,18,13}},{{30,17,17,30,20,18,17}},
        {{15,16,16,14,1,1,30}},{{31,4,4,4,4,4,4}},{{17,17,17,17,17,17,14}},
        {{17,17,17,17,17,10,4}},{{17,17,17,21,21,21,10}},{{17,17,10,4,10,17,17}},
        {{17,17,10,4,4,4,4}},{{31,1,2,4,8,16,31}}
    }};
    sf::RectangleShape pixel({scale,scale}); pixel.setFillColor(color);
    for (char c : value) {
        if (c >= 'A' && c <= 'Z') for (unsigned y=0; y<7; ++y) for (unsigned x=0; x<5; ++x)
            if (glyphs[c-'A'][y] & (1u << (4-x))) {
                pixel.setPosition({at.x+x*scale,at.y+y*scale}); window.draw(pixel);
            }
        at.x += 6*scale;
    }
}
class Tone {
    sf::SoundBuffer buffer_;
    std::unique_ptr<sf::Sound> sound_;
public:
    void play(float hz) {
        sound_.reset();
        constexpr unsigned rate = 22050, count = rate / 6;
        std::vector<std::int16_t> samples(count);
        for (unsigned i=0; i<count; ++i) {
            const float envelope = std::sin(3.14159265f*i/count);
            samples[i] = static_cast<std::int16_t>(3500*envelope*std::sin(6.2831853f*hz*i/rate));
        }
        if (buffer_.loadFromSamples(samples.data(),samples.size(),1,rate,{sf::SoundChannel::Mono})) {
            sound_ = std::make_unique<sf::Sound>(buffer_); sound_->play();
        }
    }
};
void panel(sf::RenderWindow& window, const std::string& heading, const std::string& detail) {
    sf::RectangleShape shade({896.f,480.f}); shade.setFillColor(sf::Color(8,12,23,225)); window.draw(shade);
    text(window,heading,{(896.f-heading.size()*18.f)/2.f,192.f},3.f,sf::Color(80,220,175));
    text(window,detail,{(896.f-detail.size()*12.f)/2.f,246.f},2.f,sf::Color::White);
}
}
void runNetworkGame(sf::IpAddress address, unsigned short port, bool muted) {
    sf::RenderWindow window(sf::VideoMode({896,544}), "Parallel Realms - Connecting");
    window.setView(sf::View(sf::FloatRect({0.f,0.f},{896.f,544.f})));
    window.setVerticalSyncEnabled(true); window.setKeyRepeatEnabled(false);
    NetworkClient client(address,port);
    Level left(Rooms::makeLeftRoom(),{0.f,0.f}), right(Rooms::makeRightRoom(),{448.f,0.f});
    Protocol::Input input;
    Protocol::State previous;
    bool havePrevious = false;
    std::unique_ptr<Tone> tone;
    if (!muted) tone = std::make_unique<Tone>();
    while (window.isOpen()) {
        while (auto event=window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) window.close();
            if (const auto* key=event->getIf<sf::Event::KeyPressed>()) {
                if (key->code==sf::Keyboard::Key::Escape) window.close();
                if (key->code==sf::Keyboard::Key::R) ++input.reset;
            }
        }
        if (!window.isOpen()) break;
        const auto pressed = [&](sf::Keyboard::Key a,sf::Keyboard::Key b) {
            return window.hasFocus() && (sf::Keyboard::isKeyPressed(a)||sf::Keyboard::isKeyPressed(b));
        };
        input.up=pressed(sf::Keyboard::Key::W,sf::Keyboard::Key::Up);
        input.down=pressed(sf::Keyboard::Key::S,sf::Keyboard::Key::Down);
        input.left=pressed(sf::Keyboard::Key::A,sf::Keyboard::Key::Left);
        input.right=pressed(sf::Keyboard::Key::D,sf::Keyboard::Key::Right);
        client.submit(input);
        const auto view=client.view(); const auto& s=view.state;
        window.setTitle("Parallel Realms | " + (view.received ? std::string(view.player==0?"BLUE":"RED") : "CONNECTING") +
                        " | WASD / Arrows | R: reset both | " + view.message);
        if (tone && havePrevious && s.round==previous.round && !view.disconnected) {
            if (s.won && !previous.won) tone->play(880.f);
            else if (s.hits!=previous.hits) tone->play(140.f);
            else if (s.switches!=previous.switches) tone->play(550.f);
        }
        previous=s; havePrevious=view.received;
        window.clear(sf::Color(13,17,28));
        left.draw(window,s.switches[1]); right.draw(window,s.switches[0]);
        if (view.received) {
            for (auto h:s.hazards) {
                sf::CircleShape shape(h.radius); shape.setOrigin({h.radius,h.radius});
                shape.setPosition(h.position); shape.setFillColor(sf::Color(255,126,36)); window.draw(shape);
            }
            for (unsigned i=0;i<2;++i) {
                sf::RectangleShape shape({Constants::PlayerSize,Constants::PlayerSize});
                shape.setPosition(s.players[i]);
                shape.setFillColor(s.finished[i]?sf::Color(80,220,175):(i==0?sf::Color(66,153,225):sf::Color(244,96,108)));
                shape.setOutlineThickness(i==view.player?3.f:1.f); shape.setOutlineColor(sf::Color::White); window.draw(shape);
            }
        }
        const auto white=sf::Color(220,225,240);
        text(window,view.player==0?"YOU ARE BLUE":"YOU ARE RED",{16,489},2,white);
        text(window,"WASD OR ARROWS     R RESET     ESC QUIT",{16,518},1.5f,white);
        text(window,s.switches[0]?"BLUE SWITCH ON":"BLUE SWITCH OFF",{452,489},1.5f,white);
        text(window,s.switches[1]?"RED SWITCH ON":"RED SWITCH OFF",{670,489},1.5f,white);
        if (view.disconnected) panel(window,"CONNECTION LOST","CHECK TERMINAL OR TITLE BAR   ESC TO QUIT");
        else if (!view.received) panel(window,"CONNECTING","PLEASE WAIT");
        else if (!s.ready) panel(window,"WAITING FOR PARTNER","CONNECT THE OTHER COMPUTER");
        else if (s.won) panel(window,"BOTH REALMS ESCAPED","TEAMWORK WINS   PRESS R TO PLAY AGAIN");
        window.display();
    }
}
