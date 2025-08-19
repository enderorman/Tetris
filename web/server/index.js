const { spawn } = require("child_process");
const path = require("path");
const WebSocket = require("ws");

const PORT = process.env.PORT || 7071;

// Configure your default inputs here or pass via env vars
const gameBin = path.resolve(__dirname, "..", "..", "tetris");
const grid =
  process.env.GRID ||
  path.resolve(__dirname, "..", "..", "samples", "9_shortplay", "grid.dat");
const blocks =
  process.env.BLOCKS ||
  path.resolve(__dirname, "..", "..", "samples", "9_shortplay", "blocks.dat");
const gravity = process.env.GRAVITY || "0";
const leaderboard =
  process.env.LEADERBOARD ||
  path.resolve(__dirname, "..", "..", "samples", "leaderboard.txt");
const player = process.env.PLAYER || "player";

const wss = new WebSocket.Server({ port: PORT });
console.log(`WebSocket server listening on ws://localhost:${PORT}`);

let game = null;
let clients = new Set();
let fallInterval = null;
const FALL_MS = 700; // fixed soft fall speed

function startAutoFall() {
  stopAutoFall();
  fallInterval = setInterval(() => {
    if (game && game.stdin.writable) {
      game.stdin.write("MOVE_DOWN\n");
    }
  }, FALL_MS);
}

function stopAutoFall() {
  if (fallInterval) {
    clearInterval(fallInterval);
    fallInterval = null;
  }
}

function startGame() {
  if (game) return;
  game = spawn(gameBin, [grid, blocks, gravity, leaderboard, player]);
  console.log("Spawned game:", game.pid);

  startAutoFall();

  game.stdout.setEncoding("utf8");
  game.stdout.on("data", (chunk) => {
    const lines = chunk.split(/\r?\n/).filter(Boolean);
    for (const line of lines) {
      if (line.startsWith("STATE:")) {
        const json = line.slice("STATE:".length);
        for (const ws of clients) {
          if (ws.readyState === WebSocket.OPEN) {
            ws.send(json);
          }
        }
      }
    }
  });

  game.stderr.setEncoding("utf8");
  game.stderr.on("data", (chunk) => {
    console.error("[game stderr]", chunk);
  });

  game.on("close", (code) => {
    console.log("Game exited with code", code);
    stopAutoFall();
    game = null;
    for (const ws of clients) {
      if (ws.readyState === WebSocket.OPEN) {
        ws.send(JSON.stringify({ type: "ended", code }));
      }
    }
  });
}

wss.on("connection", (ws) => {
  clients.add(ws);
  if (!game) startGame();

  ws.on("message", (msg) => {
    const text = msg.toString();
    // Expect either raw commands (e.g., MOVE_LEFT) or JSON {command: "..."}
    let command = text;
    try {
      const obj = JSON.parse(text);
      if (obj && typeof obj.command === "string") command = obj.command;
    } catch (_) {}

    if (game && game.stdin.writable) {
      game.stdin.write(command.trim() + "\n");
    }
  });

  ws.on("close", () => {
    clients.delete(ws);
  });
});
