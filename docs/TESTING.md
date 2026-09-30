# Verification and demonstration

Automated tests are optional and require the `tests/` folder. Enable them explicitly:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

The test executable uses exceptions for checks, so Release builds do not disable
assertions. It exercises real TCP connections on an OS-assigned local port.

Automated coverage:
- Input/state round trips, invalid protocol version and trailing data rejection.
- Wall collision and no movement while waiting.
- Reach both switches and both exits through actual movement/collision code.
- A hazard resets both players while retaining activated switches.
- Full reset clears switches and victory.
- One-client lobby, two-client assignment, input delivery and replicated movement.
- Disconnect, neutral input and reuse of the vacant player slot.

When Python 3 is available on macOS/Linux, CTest also runs `server_smoke.py`
against the actual server executable. It checks fragmented TCP input, replicated
movement, a shared reset and the disconnect lobby, then terminates the server.

Delivery verification: the client, server and tests were compiled in Release
mode with GCC 13 and SFML 3.0.2 on Linux. Both test suites passed. Interactive
graphics/audio and macOS/Windows builds were not verified in this environment.
The physical two-computer checklist below remains to be run on your devices.

## Two-physical-computer checklist

1. Start the server and host client; verify the waiting panel.
2. Connect the other computer using the host's LAN IP; verify blue/red assignments.
3. Move each player and check that both windows display the same positions.
4. Try passing the purple doors before switches: movement must be blocked.
5. Activate each yellow switch: the other room's door opens on both screens.
6. Touch a hazard: both return to spawn, with switches still active.
7. Reach both exits: both clients show the victory overlay and play a sound.
8. Press R on either computer: both clients start a fresh round.
9. Change focus while holding movement: that player's movement stops.
10. Close one client: the survivor waits; relaunch the closed client to rejoin.
11. Stop the server: clients show a connection message and still respond to Esc.
12. Try an incorrect IP or occupied port: there should be an error, not an endless hang.

Also listen for switch, hit and win cues. `--mute` disables sound construction.
For an unplugged network, allow up to five seconds for timeout detection.

Record the server terminal and both physical computers for the distributed
computing demonstration. Explain that the hazard thread demonstrates concurrency
within one machine, while TCP clients/server demonstrate communication between
machines. A two-window localhost demo alone does not satisfy the physical-computer
requirement.
