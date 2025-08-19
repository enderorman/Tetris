import React, { useEffect, useMemo, useRef, useState } from "react";

interface GameState {
  score: number;
  highScore: number;
  gravity: boolean;
  rows: number;
  cols: number;
  grid: number[][];
  active?: { row: number; col: number; shape: number[][] };
  next?: { shape: number[][] };
}

export const App: React.FC = () => {
  const [state, setState] = useState<GameState | null>(null);
  const wsRef = useRef<WebSocket | null>(null);
  const prevGridRef = useRef<number[][] | null>(null);
  const [animating, setAnimating] = useState<Set<string>>(new Set());

  const keyframes = (
    <style>{`
      @keyframes fall-in { 0% { transform: translateY(-12px); opacity: 0; } 100% { transform: translateY(0); opacity: 1; } }
      button { background:#1f2937; color:#e5e7eb; border:1px solid #374151; padding:8px 12px; border-radius:8px; cursor:pointer; }
      button:hover { background:#111827; }
    `}</style>
  );

  useEffect(() => {
    const ws = new WebSocket("ws://localhost:7071");
    wsRef.current = ws;
    ws.onmessage = (evt) => {
      try {
        const data = JSON.parse(evt.data as string);
        if (data && Array.isArray(data.grid)) {
          const prev = prevGridRef.current;
          if (prev) {
            const newly = new Set<string>();
            for (let i = 0; i < data.grid.length; i++) {
              for (let j = 0; j < data.grid[i].length; j++) {
                if ((prev[i]?.[j] ?? 0) === 0 && data.grid[i][j] === 1) {
                  newly.add(`${i}-${j}`);
                }
              }
            }
            if (newly.size) {
              setAnimating(
                (old) => new Set([...Array.from(old), ...Array.from(newly)])
              );
              setTimeout(() => {
                setAnimating((old) => {
                  const next = new Set(old);
                  newly.forEach((k) => next.delete(k));
                  return next;
                });
              }, 220);
            }
          }
          prevGridRef.current = data.grid;
          setState(data as GameState);
        }
      } catch {}
    };
    return () => {
      ws.close();
    };
  }, []);

  useEffect(() => {
    const onKey = (e: KeyboardEvent) => {
      if (!wsRef.current) return;
      switch (e.key) {
        case "ArrowLeft":
          e.preventDefault();
          wsRef.current.send("MOVE_LEFT");
          break;
        case "ArrowRight":
          e.preventDefault();
          wsRef.current.send("MOVE_RIGHT");
          break;
        case "ArrowUp":
          e.preventDefault();
          wsRef.current.send("ROTATE_RIGHT");
          break;
        case "ArrowDown":
          e.preventDefault();
          wsRef.current.send("ROTATE_LEFT");
          break;
        case " ":
          e.preventDefault();
          wsRef.current.send("DROP");
          break;
        case "g":
        case "G":
          wsRef.current.send("GRAVITY_SWITCH");
          break;
        case "q":
        case "Q":
          wsRef.current.send("QUIT");
          break;
      }
    };
    window.addEventListener("keydown", onKey);
    return () => window.removeEventListener("keydown", onKey);
  }, []);

  const send = (cmd: string) => {
    wsRef.current?.send(cmd);
  };

  const cellSize = 22;

  const ghostOffset = useMemo(() => {
    const s = state;
    if (!s?.active) return 0;
    const { row, col, shape } = s.active;
    const h = shape.length,
      w = shape[0]?.length || 0;
    let r = row;
    outer: for (;;) {
      const nextR = r + 1;
      if (nextR + h > s.rows) break;
      for (let i = 0; i < h; i++) {
        for (let j = 0; j < w; j++) {
          if (shape[i][j] && s.grid[nextR + i][col + j]) {
            break outer;
          }
        }
      }
      r = nextR;
    }
    return r - row;
  }, [state]);

  const board = useMemo(() => {
    const s = state;
    const g = s?.grid || [];
    const a = s?.active;
    const ghostRow = a ? a.row + ghostOffset : -999;
    return g.map((row: number[], i: number) => (
      <div
        key={i}
        style={{
          display: "grid",
          gridTemplateColumns: `repeat(${row.length}, ${cellSize}px)`,
          gap: 2,
        }}
      >
        {row.map((v: number, j: number) => {
          let bg = v
            ? "linear-gradient(135deg, #22d3ee 0%, #0ea5e9 50%, #0284c7 100%)"
            : "#0b1220";
          let border = v
            ? "1px solid rgba(255,255,255,0.2)"
            : "1px solid #0f172a";
          let anim: string | undefined;
          if (
            a &&
            i >= a.row &&
            i < a.row + a.shape.length &&
            j >= a.col &&
            j < a.col + (a.shape[0]?.length || 0) &&
            a.shape[i - a.row][j - a.col]
          ) {
            bg =
              "linear-gradient(135deg, #f59e0b 0%, #ef4444 50%, #dc2626 100%)";
            border = "1px solid rgba(255,255,255,0.25)";
            anim = animating.has(`${i}-${j}`)
              ? "fall-in 180ms ease-out"
              : undefined;
          }
          if (
            a &&
            i >= ghostRow &&
            i < ghostRow + a.shape.length &&
            j >= a.col &&
            j < a.col + (a.shape[0]?.length || 0) &&
            a.shape[i - ghostRow][j - a.col] &&
            !v
          ) {
            bg =
              "repeating-linear-gradient(45deg, rgba(255,255,255,0.12) 0 3px, transparent 3px 6px)";
            border = "1px dashed rgba(255,255,255,0.25)";
          }
          return (
            <div
              key={j}
              style={{
                width: cellSize,
                height: cellSize,
                background: bg,
                borderRadius: 4,
                boxShadow: v
                  ? "inset 0 0 6px rgba(255,255,255,0.25), 0 2px 6px rgba(0,0,0,0.35)"
                  : "inset 0 0 2px rgba(255,255,255,0.08)",
                border,
                animation: anim,
              }}
            />
          );
        })}
      </div>
    ));
  }, [state, ghostOffset, animating]);

  const nextPreview = useMemo(() => {
    const n = state?.next?.shape || [];
    if (!n.length) return null;
    const w = Math.max(...n.map((r) => r.length));
    return (
      <div
        style={{
          display: "inline-block",
          padding: 8,
          background: "#0b1220",
          border: "1px solid #1f2937",
          borderRadius: 8,
        }}
      >
        {n.map((row, i) => (
          <div
            key={i}
            style={{
              display: "grid",
              gridTemplateColumns: `repeat(${w}, ${cellSize}px)`,
              gap: 2,
            }}
          >
            {row.map((v, j) => (
              <div
                key={j}
                style={{
                  width: cellSize,
                  height: cellSize,
                  background: v
                    ? "linear-gradient(135deg, #22d3ee 0%, #0ea5e9 50%, #0284c7 100%)"
                    : "#0b1220",
                  borderRadius: 4,
                  border: v
                    ? "1px solid rgba(255,255,255,0.2)"
                    : "1px solid #0f172a",
                }}
              />
            ))}
          </div>
        ))}
      </div>
    );
  }, [state]);

  return (
    <div
      style={{
        minHeight: "100vh",
        background:
          "radial-gradient(1000px 600px at 20% 0%, #0b1220 0%, #030712 60%)",
        color: "white",
        padding: 24,
      }}
    >
      {keyframes}
      <div
        style={{
          maxWidth: 1100,
          margin: "0 auto",
          display: "grid",
          gridTemplateColumns: "minmax(460px, 1fr) 320px",
          gap: 24,
        }}
      >
        <div>
          <h2 style={{ marginTop: 0, marginBottom: 8, letterSpacing: 1 }}>
            Tetris
          </h2>
          <div
            style={{
              display: "flex",
              gap: 12,
              alignItems: "center",
              marginBottom: 12,
            }}
          >
            <div
              style={{
                background: "#0b1220",
                border: "1px solid #1f2937",
                padding: "8px 12px",
                borderRadius: 8,
              }}
            >
              <div>
                Score: <b>{state?.score ?? 0}</b>
              </div>
              <div>
                Gravity: <b>{state?.gravity ? "ON" : "OFF"}</b>
              </div>
            </div>
            <button onClick={() => send("GRAVITY_SWITCH")}>
              Toggle Gravity (G)
            </button>
          </div>
          <div
            style={{
              display: "inline-block",
              padding: 10,
              background: "#0b1220",
              border: "1px solid #1f2937",
              borderRadius: 10,
            }}
          >
            {board}
          </div>
        </div>
        <div style={{ display: "flex", flexDirection: "column", gap: 12 }}>
          <div>
            <div style={{ marginBottom: 6, opacity: 0.85 }}>Next</div>
            {nextPreview}
          </div>
          <div style={{ display: "flex", flexDirection: "column", gap: 8 }}>
            <button onClick={() => wsRef.current?.send("ROTATE_LEFT")}>
              Rotate Left (↓)
            </button>
            <button onClick={() => wsRef.current?.send("ROTATE_RIGHT")}>
              Rotate Right (↑)
            </button>
            <button onClick={() => wsRef.current?.send("MOVE_LEFT")}>
              Move Left (←)
            </button>
            <button onClick={() => wsRef.current?.send("MOVE_RIGHT")}>
              Move Right (→)
            </button>
            <button onClick={() => wsRef.current?.send("DROP")}>
              Drop (Space)
            </button>
            <button onClick={() => wsRef.current?.send("QUIT")}>
              Quit (Q)
            </button>
            <p style={{ maxWidth: 280, opacity: 0.85 }}>
              Arrow keys to move/rotate, Space to drop, G to toggle gravity.
            </p>
          </div>
        </div>
      </div>
    </div>
  );
};
