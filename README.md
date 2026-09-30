# Parallel Realms — Day 1

Parallel Realms is a custom 2D cooperative puzzle game written in C++17. SFML
is used only for the window, input and primitive drawing. The game loop, tile
map, collision system, player movement, objectives and hazard simulation are
implemented by this project.

## Day 1 features

- Custom fixed-timestep-friendly game loop
- Two tile-based rooms rendered without image assets
- Two local players
- Axis-separated wall and door collision
- Cross-room switches: each player opens the other player's door
- Exit detection and shared victory condition
- Moving hazards simulated on a background `std::thread`
- Thread-safe hazard snapshots protected by `std::mutex`

## Controls

| Action | Player 1 | Player 2 |
| --- | --- | --- |
| Move | W A S D | Arrow keys |
| Reset | R | R |
| Quit | Escape | Escape |

Touch the yellow switch in each room. Player 1's switch opens Player 2's purple
door, and Player 2's switch opens Player 1's door. Avoid the orange hazards and
move both players onto their green exits.

## macOS setup

Install Apple's command-line tools, Homebrew and CMake if necessary:

```bash
xcode-select --install
brew install cmake
```

SFML does not need to be installed separately. CMake downloads the pinned SFML
3.0.2 source and builds the required modules.

Build and run:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
./build/parallel_realms
```

The first build takes longer because CMake downloads and compiles SFML.

## Linux setup

Install a compiler, Git, CMake and SFML's system dependencies. On Ubuntu:

```bash
sudo apt update
sudo apt install build-essential git cmake \
    libxrandr-dev libxcursor-dev libxi-dev libudev-dev \
    libfreetype-dev libflac-dev libvorbis-dev libgl1-mesa-dev \
    libegl1-mesa-dev libdrm-dev libgbm-dev
```

Then use the same CMake build commands shown above.

## Engine design

`Game` owns the main loop and high-level rules. `Level` stores and renders the
tile map and performs collision tests. `Player` translates real-time keyboard
input into collision-safe movement. `HazardSystem` runs its simulation on a
background OS thread and exposes immutable snapshots to the rendering thread.

On macOS, SFML window creation, event polling and rendering stay on the main
thread. Only game simulation work is moved to a background thread.

## Day 2 direction

Day 2 separates the two players across physical computers:

1. Add an authoritative server executable.
2. Add a network-receive thread to each client.
3. Send input commands from clients to the server.
4. Simulate the world on the server.
5. broadcast state snapshots to both clients.
6. Protect shared network state with a mutex and condition variable.

