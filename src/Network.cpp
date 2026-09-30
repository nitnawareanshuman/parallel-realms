#include "Network.hpp"
#include <chrono>
#include <memory>
#include <optional>
#include <stdexcept>

namespace {
using Clock = std::chrono::steady_clock;
using namespace std::chrono_literals;
bool failed(sf::Socket::Status s) {
    return s == sf::Socket::Status::Disconnected || s == sf::Socket::Status::Error;
}
struct Peer {
    sf::TcpSocket socket;
    std::optional<sf::Packet> pending;
    std::uint64_t revision = 0;
    Clock::time_point lastInput = Clock::now(), lastSend = Clock::now();
};
}
NetworkServer::NetworkServer(unsigned short port) {
    if (listener_.listen(port) != sf::Socket::Status::Done)
        throw std::runtime_error("Cannot listen on port " + std::to_string(port) + ". Is another server running?");
    port_ = listener_.getLocalPort();
    listener_.setBlocking(false);
    worker_ = std::thread(&NetworkServer::loop, this);
}
NetworkServer::~NetworkServer() {
    running_ = false; wake_.notify_all();
    if (worker_.joinable()) worker_.join();
}
NetworkServer::Inputs NetworkServer::inputs() {
    std::lock_guard<std::mutex> lock(mutex_); return inputs_;
}
void NetworkServer::publish(const Protocol::State& s) {
    { std::lock_guard<std::mutex> lock(mutex_); state_ = s; ++revision_; }
    wake_.notify_one();
}
void NetworkServer::loop() {
    std::array<std::unique_ptr<Peer>, 2> peers;
    auto drop = [&](unsigned i) {
        peers[i].reset();
        std::lock_guard<std::mutex> lock(mutex_);
        inputs_.players[i] = {}; inputs_.ready = false; ++inputs_.membership;
    };
    while (running_) {
        // Accept at most one connection per iteration. Extra clients are closed.
        auto incoming = std::make_unique<Peer>();
        if (listener_.accept(incoming->socket) == sf::Socket::Status::Done) {
            for (unsigned i = 0; i < 2; ++i) if (!peers[i]) {
                incoming->socket.setBlocking(false);
                peers[i] = std::move(incoming);
                std::lock_guard<std::mutex> lock(mutex_);
                inputs_.players[i] = {}; ++inputs_.membership;
                inputs_.ready = bool(peers[0]) && bool(peers[1]);
                break;
            }
        }
        for (unsigned i = 0; i < 2; ++i) {
            if (!peers[i]) continue;
            auto& peer = *peers[i];
            bool bad = false;
            for (unsigned n = 0; n < 16; ++n) {
                sf::Packet p;
                const auto status = peer.socket.receive(p);
                if (failed(status)) { bad = true; break; }
                if (status != sf::Socket::Status::Done) break;
                Protocol::Input input;
                if (!Protocol::decodeInput(p, input)) { bad = true; break; }
                peer.lastInput = Clock::now();
                std::lock_guard<std::mutex> lock(mutex_);
                if (input.reset != inputs_.players[i].reset) ++inputs_.resets;
                inputs_.players[i] = input;
            }
            const auto now = Clock::now();
            if (bad || now-peer.lastInput > 5s || now-peer.lastSend > 5s) { drop(i); continue; }
            // Stop stale input quickly; a missing heartbeat later frees the slot.
            if (now-peer.lastInput > 250ms) {
                std::lock_guard<std::mutex> lock(mutex_);
                auto& input = inputs_.players[i];
                input.up = input.down = input.left = input.right = false;
            }
            if (!peer.pending) {
                std::lock_guard<std::mutex> lock(mutex_);
                if (peer.revision != revision_) {
                    peer.pending = Protocol::encodeState(state_, i);
                    peer.revision = revision_;
                }
            }
            if (peer.pending) {
                // Retain the SAME packet on Partial/NotReady; never interleave frames.
                const auto status = peer.socket.send(*peer.pending);
                if (failed(status)) { drop(i); continue; }
                if (status == sf::Socket::Status::Done) {
                    peer.pending.reset(); peer.lastSend = now;
                }
            }
        }
        std::unique_lock<std::mutex> lock(mutex_);
        wake_.wait_for(lock, 2ms);
    }
}
NetworkClient::NetworkClient(sf::IpAddress address, unsigned short port)
    : worker_(&NetworkClient::loop, this, address, port) {}
NetworkClient::~NetworkClient() {
    running_ = false; wake_.notify_all();
    if (worker_.joinable()) worker_.join();
}
void NetworkClient::submit(const Protocol::Input& input) {
    { std::lock_guard<std::mutex> lock(mutex_); input_ = input; }
    wake_.notify_one();
}
NetworkClient::View NetworkClient::view() {
    std::lock_guard<std::mutex> lock(mutex_); return view_;
}
void NetworkClient::loop(sf::IpAddress address, unsigned short port) {
    sf::TcpSocket socket; // Socket lives exclusively on this worker thread.
    auto error = [&](const std::string& message) {
        std::lock_guard<std::mutex> lock(mutex_);
        view_.disconnected = true; view_.message = message;
    };
    if (socket.connect(address, port, sf::seconds(3.f)) != sf::Socket::Status::Done) {
        error("Connection failed: check server, IP, port and firewall. Esc to quit."); return;
    }
    socket.setBlocking(false);
    std::optional<sf::Packet> pending;
    auto nextSend = Clock::now(), lastReceive = nextSend;
    while (running_) {
        const auto now = Clock::now();
        if (!pending && now >= nextSend) {
            std::lock_guard<std::mutex> lock(mutex_);
            pending = Protocol::encodeInput(input_);
            nextSend = now + 16ms;
        }
        if (pending) {
            const auto status = socket.send(*pending);
            if (failed(status)) { error("Disconnected. Restart client to rejoin. Esc to quit."); return; }
            if (status == sf::Socket::Status::Done) pending.reset();
        }
        for (unsigned n = 0; n < 16; ++n) {
            sf::Packet p;
            const auto status = socket.receive(p);
            if (failed(status)) { error("Server disconnected. Esc to quit."); return; }
            if (status != sf::Socket::Status::Done) break;
            Protocol::State state; unsigned player{};
            if (!Protocol::decodeState(p, state, player)) { error("Incompatible server data. Use the same project version."); return; }
            lastReceive = Clock::now();
            std::lock_guard<std::mutex> lock(mutex_);
            view_.state = state; view_.player = player; view_.received = true;
            view_.message = state.ready ? "Connected" : "Waiting for the other player...";
        }
        if (now-lastReceive > 5s) { error("Server timed out. Restart client to rejoin."); return; }
        std::unique_lock<std::mutex> lock(mutex_);
        wake_.wait_for(lock, 2ms);
    }
}
