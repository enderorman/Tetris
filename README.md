# Tetris (BlockFall)

## What’s inside

- C++11 game core and tests
- Browser UI (React) with keyboard controls, ghost piece, and next-piece preview
- WebSocket server that bridges the C++ binary and the UI

## Build (C++)

```
make            # builds: blockfall (tests) and tetris (CLI game)
```

## Run tests

```
./blockfall
```

## Play in browser

1. Start the WebSocket bridge (spawns the C++ game):

```
cd web/server
npm install
npm start   # listens on ws://localhost:7071
```

2. Start the UI:

```
cd ../client
npm install
npm run dev
```

3. Open the Local URL that Vite prints (e.g. http://localhost:5174 or 5175/5176 if ports are busy)

- Controls: Left/Right to move, Up/Down to rotate, Space to drop, G to toggle gravity, Q to quit.
- The UI connects to the WebSocket server at ws://localhost:7071.

## Notes

- The UI shows a dashed ghost where the piece will land and a next-piece preview.
- If ports collide, stop previous processes and retry.
