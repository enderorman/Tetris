/* Minimal chiptune player for the Tetris "A" theme (Korobeiniki).
   Not an exact replica, just a lightweight square-wave approximation. */

let ctx: AudioContext | null = null;
let masterGain: GainNode | null = null;
let running = false;
let stopFlag = false;

// Frequencies for notes (approx, A4=440)
const F: Record<string, number> = {
  C4: 261.63,
  Cs4: 277.18,
  D4: 293.66,
  Ds4: 311.13,
  E4: 329.63,
  F4: 349.23,
  Fs4: 369.99,
  G4: 392.0,
  Gs4: 415.3,
  A4: 440.0,
  As4: 466.16,
  B4: 493.88,
  C5: 523.25,
  Cs5: 554.37,
  D5: 587.33,
  Ds5: 622.25,
  E5: 659.25,
  F5: 698.46,
  Fs5: 739.99,
  G5: 783.99,
  Gs5: 830.61,
  A5: 880.0,
};

// A tiny slice of the melody (enough to be recognizable), repeated.
// Each tuple: [noteName or 'R' rest, beats]
const MELODY: Array<[string, number]> = [
  ["E5", 1],
  ["B4", 0.5],
  ["C5", 0.5],
  ["D5", 1],
  ["C5", 0.5],
  ["B4", 0.5],
  ["A4", 1],
  ["A4", 0.5],
  ["C5", 0.5],
  ["E5", 1],
  ["D5", 1],
  ["C5", 1],
  ["B4", 1],
  ["C5", 0.5],
  ["D5", 0.5],
  ["E5", 1],
  ["C5", 1],
  ["A4", 1],
  ["A4", 2],
];

function scheduleToneAt(time: number, freq: number, duration: number) {
  if (!ctx || !masterGain) return;
  const osc = ctx.createOscillator();
  const gain = ctx.createGain();
  osc.type = "square";
  osc.frequency.setValueAtTime(freq, time);
  gain.gain.setValueAtTime(0.0001, time);
  gain.gain.exponentialRampToValueAtTime(0.25, time + 0.01);
  gain.gain.exponentialRampToValueAtTime(0.0001, time + duration);
  osc.connect(gain).connect(masterGain);
  osc.start(time);
  osc.stop(time + duration + 0.05);
}

export async function startMusic(bpm = 160) {
  if (running) return;
  stopFlag = false;
  ctx =
    ctx || new (window.AudioContext || (window as any).webkitAudioContext)();
  if (ctx.state === "suspended") await ctx.resume();
  masterGain = masterGain || ctx.createGain();
  masterGain.gain.value = 0.6;
  masterGain.connect(ctx.destination);

  running = true;
  const beatSec = 60 / bpm;

  const loop = async () => {
    while (!stopFlag && ctx) {
      const startT = ctx.currentTime + 0.05;
      let t = startT;
      for (const [n, beats] of MELODY) {
        if (stopFlag) break;
        const dur = beats * beatSec;
        if (n !== "R") {
          const f = F[n];
          if (f) scheduleToneAt(t, f, Math.max(0.08, dur - 0.02));
        }
        t += dur;
      }
      // Wait for the section to finish before scheduling again
      const total = t - startT;
      await new Promise((res) =>
        setTimeout(res, Math.max(0, (total - 0.05) * 1000))
      );
    }
  };
  loop();
}

export function stopMusic() {
  stopFlag = true;
  running = false;
}
