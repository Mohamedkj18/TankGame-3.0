import React, { useEffect, useMemo, useRef, useState } from "react";

// TankGame Multi-Game Visualizer — load FOLDERS of .viz.txt (board states) and .moves.txt (actions)
// Features: pick two folders, auto-pair files by shared stem, game list + search, per-game playback,
// loop/auto-advance to next game, speed control, keyboard shortcuts, step scrubber, move filter.
// Works with standard <input type="file" webkitdirectory directory multiple>.

export default function MultiGameVisualizer() {
  // ---------- Global state ----------
  type Summary = {
    winner?: string;
    reason?: string;
    totalSteps?: number;
    p1Remain?: string;
    p2Remain?: string;
  };
  type MovesTable = string[][]; // tokens[row][col]

  type Game = {
    id: string; // stem key
    name: string; // pretty file-derived name
    frames: string[]; // viz frames
    rows: number;
    cols: number;
    movesTable: MovesTable;
    movesMode: "unknown" | "perLine" | "perColumn";
    summary: Summary | null;
    sourceViz?: File;
    sourceMoves?: File;
  };

  const [games, setGames] = useState<Game[]>([]);
  const [gameIndex, setGameIndex] = useState<number>(0);

  const [playing, setPlaying] = useState<boolean>(false);
  const [loopGames, setLoopGames] = useState<boolean>(true);
  const [speedMs, setSpeedMs] = useState<number>(500);

  const [step, setStep] = useState<number>(0);
  const [filterStr, setFilterStr] = useState<string>("");
  const [search, setSearch] = useState<string>("");

  const playRef = useRef<number | null>(null);

  // ---------- File loading ----------
  function stripSuffix(name: string): string {
    // remove trailing .viz.txt or .moves.txt
    return name.replace(/\.(viz|moves)\.txt$/i, "");
  }
  function commonStem(path: string): string {
    // drop directory and suffix, keep distinctive stem
    const base = path.split(/[/\\]/).pop() || path;
    return stripSuffix(base);
  }

  async function readFileAsText(file: File): Promise<string> {
    return new Promise((resolve, reject) => {
      const r = new FileReader();
      r.onload = () => resolve(String(r.result || ""));
      r.onerror = () => reject(r.error);
      r.readAsText(file);
    });
  }

  function parseViz(text: string): {
    frames: string[];
    rows: number;
    cols: number;
  } {
    const blocks: string[] = [];
    const re =
      /===\s*Game\s*Step\s*\d+\s*===\s*([\s\S]*?)(?=(?:===\s*Game\s*Step\s*\d+\s*===)|$)/g;
    let m: RegExpExecArray | null;
    while ((m = re.exec(text)) !== null) {
      let block = m[1].trim().replace(/\r/g, "");
      const lines = block.split(/\n/);
      const maxLen = Math.max(0, ...lines.map((l) => l.length));
      const padded = lines.map((l) => l.padEnd(maxLen, "."));
      blocks.push(padded.join("\n"));
    }
    let rows = 0,
      cols = 0;
    if (blocks.length > 0) {
      const first = blocks[0].split("\n");
      rows = first.length;
      cols = first[0]?.length ?? 0;
    }
    return { frames: blocks, rows, cols };
  }

  function splitCSVLine(line: string): string[] {
    return line
      .split(",")
      .map((t) => t.trim())
      .filter(Boolean);
  }

  function parseMoves(
    text: string,
    numSteps: number
  ): { table: MovesTable; mode: Game["movesMode"]; summary: Summary | null } {
    const lines = text.replace(/\r/g, "").split(/\n/);
    const table: string[][] = [];
    let winner: string | undefined;
    let reason: string | undefined;
    let totalSteps: number | undefined;
    let p1Remain: string | undefined;
    let p2Remain: string | undefined;

    for (const raw of lines) {
      const line = raw.trim();
      if (!line) continue;
      const sumMatch = line.match(
        /Summary:\s*winner\s*=\s*(\d+)\s+reason\s*=\s*(\d+)\s+total\s+game\s+steps\s*=\s*(\d+)\s+player1\s+remaining\s+tanks\s*=\s*(\d+)\s+player2\s+remaining\s+tanks\s*=\s*(\d+)/i
      );
      if (sumMatch) {
        winner = sumMatch[1];
        reason = sumMatch[2];
        totalSteps = parseInt(sumMatch[3], 10);
        p1Remain = sumMatch[4];
        p2Remain = sumMatch[5];
        continue;
      }
      table.push(splitCSVLine(line));
    }

    let mode: Game["movesMode"] = "unknown";
    if (table.length === numSteps) mode = "perLine";
    const maxCols = Math.max(0, ...table.map((r) => r.length));
    if (mode === "unknown" && maxCols >= numSteps) mode = "perColumn";

    return {
      table,
      mode,
      summary: { winner, reason, totalSteps, p1Remain, p2Remain },
    };
  }

  async function onPickVizFiles(files: FileList | null) {
    if (!files || files.length === 0) return;
    // Stage 1: stash texts by stem
    const byStem: Record<
      string,
      {
        viz?: { text: string; file: File };
        moves?: { text: string; file: File };
      }
    > = {};

    // Keep existing moves if already selected earlier
    for (const g of games) {
      byStem[g.id] = byStem[g.id] || {};
      byStem[g.id].viz =
        byStem[g.id].viz ||
        (g.frames.length
          ? { text: g.frames.join("\n\n"), file: g.sourceViz! }
          : undefined);
      byStem[g.id].moves =
        byStem[g.id].moves ||
        (g.movesTable.length ? { text: "", file: g.sourceMoves! } : undefined);
    }

    const promises: Promise<void>[] = [];
    Array.from(files).forEach((file) => {
      const stem = commonStem(file.name);
      byStem[stem] = byStem[stem] || {};
      promises.push(
        readFileAsText(file).then((text) => {
          if (/\.viz\.txt$/i.test(file.name)) byStem[stem].viz = { text, file };
          else if (/\.moves\.txt$/i.test(file.name))
            byStem[stem].moves = { text, file };
        })
      );
    });
    await Promise.all(promises);
    buildGamesFrom(byStem);
  }

  async function onPickMovesFiles(files: FileList | null) {
    // Same handler as viz — simpler to just call the other
    await onPickVizFiles(files);
  }

  function prettifyName(stem: string): string {
    // Shorten long auto names, keep the interesting tail
    // e.g., GameManager__mapA__Algo_X_vs_Y__run3__ts1234 → mapA | X vs Y | run3
    const parts = stem.split("__");
    if (parts.length >= 3) {
      const map =
        parts.find((p) => p.includes("maps") || p.includes("map")) || parts[0];
      const vs = parts.find((p) => p.includes("vs")) || parts[1];
      const run =
        parts.find((p) => p.startsWith("run")) || parts[parts.length - 1];
      return `${map.replace(/^.*_/, "")} | ${vs.replace(
        /Algorithm_/g,
        ""
      )} | ${run}`;
    }
    return stem;
  }

  function buildGamesFrom(
    byStem: Record<
      string,
      {
        viz?: { text: string; file: File };
        moves?: { text: string; file: File };
      }
    >
  ) {
    const next: Game[] = [];
    for (const [stem, pair] of Object.entries(byStem)) {
      if (!pair.viz && !pair.moves) continue; // ignore empties

      let frames: string[] = [];
      let rows = 0,
        cols = 0;
      if (pair.viz) {
        const pv = parseViz(pair.viz.text);
        frames = pv.frames;
        rows = pv.rows;
        cols = pv.cols;
      }

      let movesTable: MovesTable = [];
      let movesMode: Game["movesMode"] = "unknown";
      let summary: Summary | null = null;
      if (pair.moves) {
        const pm = parseMoves(pair.moves.text, frames.length);
        movesTable = pm.table;
        movesMode = pm.mode;
        summary = pm.summary;
      }

      next.push({
        id: stem,
        name: prettifyName(stem),
        frames,
        rows,
        cols,
        movesTable,
        movesMode,
        summary,
        sourceViz: pair.viz?.file,
        sourceMoves: pair.moves?.file,
      });
    }
    // Sort by name for stable UI
    next.sort((a, b) => a.name.localeCompare(b.name));
    setGames(next);
    setGameIndex(0);
    setStep(0);
  }

  // ---------- Playback across frames & games ----------
  const cur = games[gameIndex];
  const totalSteps = cur?.frames.length || 0;

  // per-game step moves
  function getStepMoves(g: Game | undefined, s: number): string[] {
    if (!g || !g.movesTable.length) return [];
    if (g.movesMode === "perLine") return g.movesTable[s] || [];
    if (g.movesMode === "perColumn") {
      const out: string[] = [];
      for (let r = 0; r < g.movesTable.length; r++) {
        const tok = g.movesTable[r][s];
        if (tok !== undefined) out.push(`Row ${r + 1}: ${tok}`);
      }
      return out;
    }
    return [];
  }

  const stepMoves = useMemo(() => {
    const mv = getStepMoves(cur, step);
    if (!filterStr) return mv;
    const f = filterStr.toLowerCase();
    return mv.filter((t) => t.toLowerCase().includes(f));
  }, [cur, step, filterStr]);

  // auto-play timer
  useEffect(() => {
    if (!playing) {
      if (playRef.current) {
        window.clearInterval(playRef.current);
        playRef.current = null;
      }
      return;
    }
    if (playRef.current) window.clearInterval(playRef.current);
    playRef.current = window.setInterval(() => {
      setStep((s) => {
        if (!cur || totalSteps === 0) return 0;
        const next = s + 1;
        if (next < totalSteps) return next;
        // reached end of game → advance to next game or loop within
        if (games.length <= 1) return 0; // loop this game
        // move to next game
        setGameIndex((gi) => {
          const ni = gi + 1;
          if (ni < games.length) return ni;
          return loopGames ? 0 : gi; // loop or stay on last
        });
        return 0;
      });
    }, Math.max(60, speedMs));
    return () => {
      if (playRef.current) {
        window.clearInterval(playRef.current);
        playRef.current = null;
      }
    };
  }, [playing, speedMs, totalSteps, games.length, loopGames, cur]);

  // keyboard shortcuts
  useEffect(() => {
    function onKey(e: KeyboardEvent) {
      if (e.key === " ") {
        e.preventDefault();
        setPlaying((p) => !p);
      } else if (e.key === "ArrowRight") {
        nextStep();
      } else if (e.key === "ArrowLeft") {
        prevStep();
      } else if (e.key === "ArrowDown") {
        nextGame();
      } else if (e.key === "ArrowUp") {
        prevGame();
      } else if (e.key === "Home") {
        setStep(0);
      } else if (e.key === "End") {
        if (totalSteps) setStep(totalSteps - 1);
      }
    }
    window.addEventListener("keydown", onKey);
    return () => window.removeEventListener("keydown", onKey);
  }, [totalSteps, games.length]);

  function nextStep() {
    if (!cur) return;
    setStep((s) => Math.min(totalSteps - 1, s + 1));
  }
  function prevStep() {
    if (!cur) return;
    setStep((s) => Math.max(0, s - 1));
  }
  function nextGame() {
    if (!games.length) return;
    setGameIndex((i) => (i + 1) % games.length);
    setStep(0);
  }
  function prevGame() {
    if (!games.length) return;
    setGameIndex((i) => (i - 1 + games.length) % games.length);
    setStep(0);
  }

  // ---------- Render helpers ----------
  function styleForChar(ch: string): { bg: string; fg: string; text: string } {
    switch (ch) {
      case "#":
        return { bg: "bg-slate-600", fg: "text-slate-200", text: "#" };
      case "@":
        return { bg: "bg-amber-500", fg: "text-white", text: "@" };
      case "1":
        return { bg: "bg-blue-600", fg: "text-white", text: "1" };
      case "2":
        return { bg: "bg-rose-600", fg: "text-white", text: "2" };
      case "/":
        return { bg: "bg-emerald-700", fg: "text-emerald-50", text: "/" };
      case ".":
        return { bg: "bg-slate-800", fg: "text-slate-800", text: "." };
      default:
        return { bg: "bg-slate-900", fg: "text-slate-300", text: ch };
    }
  }
  function describeChar(ch: string): string {
    switch (ch) {
      case "#":
        return "Wall";
      case "@":
        return "Mine";
      case "1":
        return "Player 1 Tank";
      case "2":
        return "Player 2 Tank";
      case "/":
        return "Shell";
      case ".":
        return "Empty";
      default:
        return `Unknown: ${ch}`;
    }
  }

  const grid = useMemo(() => {
    if (!cur || !cur.frames.length) return null;
    const lines = cur.frames[step].split("\n");
    const gridStyle: React.CSSProperties = {
      display: "grid",
      gridTemplateColumns: `repeat(${cur.cols}, 18px)`,
      gridAutoRows: "18px",
      gap: 1,
      background: "#0f172a",
      padding: 6,
      borderRadius: 12,
    };
    const cells: React.ReactNode[] = [];
    for (let y = 0; y < lines.length; y++) {
      const line = lines[y];
      for (let x = 0; x < line.length; x++) {
        const ch = line[x] ?? ".";
        const { bg, fg, text } = styleForChar(ch);
        cells.push(
          <div
            key={`${x}-${y}-${step}-${gameIndex}`}
            className={`flex items-center justify-center text-[12px] font-mono select-none rounded ${bg} ${fg}`}
            title={`(${x},${y}) ${describeChar(ch)}`}
          >
            {text}
          </div>
        );
      }
    }
    return (
      <div className="overflow-auto rounded-2xl shadow-inner" style={gridStyle}>
        {cells}
      </div>
    );
  }, [cur, step, gameIndex]);

  // ---------- UI ----------
  const filteredGames = useMemo(() => {
    const q = search.trim().toLowerCase();
    if (!q) return games;
    return games.filter(
      (g) => g.name.toLowerCase().includes(q) || g.id.toLowerCase().includes(q)
    );
  }, [games, search]);

  useEffect(() => {
    // keep gameIndex pointing to a valid filtered item when search changes
    if (!filteredGames.length) return;
    const current = games[gameIndex];
    const idx = filteredGames.findIndex((g) => g.id === current?.id);
    if (idx === -1) {
      // jump to first filtered
      const firstId = filteredGames[0].id;
      const newIdx = games.findIndex((g) => g.id === firstId);
      if (newIdx !== -1) {
        setGameIndex(newIdx);
        setStep(0);
      }
    }
  }, [search]);

  return (
    <div className="p-5 max-w-[1400px] mx-auto text-slate-100">
      <h1 className="text-2xl font-bold mb-2">
        TankGame Multi-Game Visualizer (Folders)
      </h1>
      <p className="text-sm text-slate-300 mb-4">
        Pick a folder of <span className="font-semibold">.viz.txt</span> files
        (game states) and a folder of{" "}
        <span className="font-semibold">.moves.txt</span> files (actions). Files
        are auto‑paired by matching name stem. Use{" "}
        <kbd className="mx-1 px-1 py-0.5 rounded bg-slate-700">Space</kbd> to
        play/pause,{" "}
        <kbd className="mx-1 px-1 py-0.5 rounded bg-slate-700">←/→</kbd> to
        step, <kbd className="mx-1 px-1 py-0.5 rounded bg-slate-700">↑/↓</kbd>{" "}
        to change game.
      </p>

      {/* Folder pickers */}
      <div className="grid md:grid-cols-2 gap-3 mb-4">
        <div className="rounded-2xl p-4 bg-slate-900/70 border border-slate-800">
          <div className="text-sm mb-2 font-semibold">
            Game States Folder (.viz.txt)
          </div>
          <input
            type="file"
            multiple // @ts-ignore non-standard
            webkitdirectory=""
            directory=""
            onChange={(e) => onPickVizFiles(e.target.files)}
            className="block w-full text-sm file:mr-3 file:py-2 file:px-3 file:rounded-xl file:border-0 file:bg-slate-700 file:text-slate-100 hover:file:bg-slate-600"
          />
          <div className="mt-2 text-xs text-slate-400">
            Loaded games: {games.length}
          </div>
        </div>
        <div className="rounded-2xl p-4 bg-slate-900/70 border border-slate-800">
          <div className="text-sm mb-2 font-semibold">
            Moves Folder (.moves.txt)
          </div>
          <input
            type="file"
            multiple // @ts-ignore non-standard
            webkitdirectory=""
            directory=""
            onChange={(e) => onPickMovesFiles(e.target.files)}
            className="block w-full text-sm file:mr-3 file:py-2 file:px-3 file:rounded-xl file:border-0 file:bg-slate-700 file:text-slate-100 hover:file:bg-slate-600"
          />
          <div className="mt-2 text-xs text-slate-400">
            Files from both pickers are merged by name.
          </div>
        </div>
      </div>

      {/* Top controls */}
      <div className="flex flex-wrap items-center gap-2 mb-4">
        <button
          onClick={() => setPlaying((p) => !p)}
          className="px-4 py-2 rounded-2xl bg-indigo-600 hover:bg-indigo-500"
          disabled={!cur || !totalSteps}
        >
          {playing ? "Pause" : "Play"}
        </button>
        <button
          onClick={() => setStep(0)}
          className="px-3 py-2 rounded-2xl bg-slate-700 hover:bg-slate-600"
          disabled={!cur}
        >
          ⏮ First
        </button>
        <button
          onClick={prevStep}
          className="px-3 py-2 rounded-2xl bg-slate-700 hover:bg-slate-600"
          disabled={!cur}
        >
          ◀ Prev
        </button>
        <button
          onClick={nextStep}
          className="px-3 py-2 rounded-2xl bg-slate-700 hover:bg-slate-600"
          disabled={!cur}
        >
          Next ▶
        </button>
        <button
          onClick={() => cur && setStep(cur.frames.length - 1)}
          className="px-3 py-2 rounded-2xl bg-slate-700 hover:bg-slate-600"
          disabled={!cur}
        >
          Last ⏭
        </button>

        <div className="ml-3 flex items-center gap-2">
          <label className="text-sm text-slate-300">Step</label>
          <input
            type="number"
            min={totalSteps ? 1 : 0}
            max={totalSteps || 0}
            value={totalSteps ? step + 1 : 0}
            onChange={(e) => {
              const v = parseInt(e.target.value || "1", 10);
              if (!Number.isFinite(v)) return;
              const idx = Math.min(Math.max(1, v), totalSteps) - 1;
              setStep(idx);
            }}
            className="w-20 px-3 py-2 rounded-xl bg-slate-800 border border-slate-700"
            disabled={!cur}
          />
          <span className="text-sm text-slate-400">/ {totalSteps || 0}</span>
        </div>

        <div className="ml-auto flex items-center gap-3">
          <label className="text-sm text-slate-300">Speed (ms)</label>
          <input
            type="range"
            min={60}
            max={2000}
            step={20}
            value={speedMs}
            onChange={(e) => setSpeedMs(parseInt(e.target.value, 10))}
            className="w-48"
            disabled={!cur}
          />
          <span className="text-xs text-slate-400 w-10 text-right">
            {speedMs}
          </span>
          <label className="flex items-center gap-2 text-sm text-slate-300">
            <input
              type="checkbox"
              checked={loopGames}
              onChange={(e) => setLoopGames(e.target.checked)}
            />{" "}
            Loop games
          </label>
        </div>
      </div>

      {/* Main layout: left = grid, right = sidebar */}
      <div className="grid lg:grid-cols-[1fr_380px] gap-4">
        {/* Board + legend */}
        <div>
          <div className="mb-2 text-sm text-slate-300 flex items-center gap-3">
            <span className="font-semibold">Game:</span>
            <span
              className="px-2 py-1 rounded-lg bg-slate-800 border border-slate-700 truncate max-w-[520px]"
              title={cur?.name || "-"}
            >
              {cur?.name || "-"}
            </span>
            <span className="text-xs text-slate-400">
              ({games.length} total)
            </span>
          </div>
          <div className="mb-2 text-sm text-slate-300 flex items-center gap-3">
            <span className="font-semibold">Game Step:</span>
            <span className="px-2 py-1 rounded-lg bg-slate-800 border border-slate-700">
              {totalSteps ? step + 1 : 0}
            </span>
            {cur?.summary?.totalSteps ? (
              <span className="text-slate-400 text-xs">
                (Total: {cur.summary.totalSteps})
              </span>
            ) : null}
          </div>
          {grid || (
            <div className="h-64 rounded-2xl bg-slate-900/50 border border-slate-800 flex items-center justify-center text-slate-400">
              Load folders to view games
            </div>
          )}
          <div className="mt-3 text-xs text-slate-300 flex flex-wrap gap-2">
            {legendItem("bg-blue-600 text-white", "1", "Player 1 Tank")}
            {legendItem("bg-rose-600 text-white", "2", "Player 2 Tank")}
            {legendItem("bg-amber-500 text-white", "@", "Mine")}
            {legendItem("bg-slate-600 text-white", "#", "Wall")}
            {legendItem("bg-emerald-700 text-emerald-50", "/", "Shell")}
          </div>
        </div>

        {/* Sidebar: Game list + Moves + Summary */}
        <div className="rounded-2xl p-4 bg-slate-900/70 border border-slate-800 max-h-[660px] overflow-auto">
          {/* Game picker */}
          <div className="mb-3">
            <div className="text-sm font-semibold mb-1">Games</div>
            <input
              value={search}
              onChange={(e) => setSearch(e.target.value)}
              placeholder="Search games..."
              className="w-full mb-2 px-3 py-2 rounded-xl bg-slate-800 border border-slate-700 text-sm"
            />
            <ul className="space-y-1 text-xs">
              {filteredGames.map((g, i) => (
                <li key={g.id}>
                  <button
                    onClick={() => {
                      setGameIndex(games.findIndex((x) => x.id === g.id));
                      setStep(0);
                    }}
                    className={`w-full text-left px-2 py-1 rounded-lg border ${
                      i ===
                      filteredGames.findIndex(
                        (x) => x.id === games[gameIndex]?.id
                      )
                        ? "bg-indigo-600/30 border-indigo-600"
                        : "bg-slate-800/60 border-slate-700"
                    }`}
                    title={g.id}
                  >
                    <div className="flex justify-between">
                      <span className="truncate mr-2">{g.name}</span>
                      <span className="opacity-70">
                        {g.frames.length || 0} steps
                      </span>
                    </div>
                  </button>
                </li>
              ))}
              {!filteredGames.length && (
                <li className="text-slate-400">No games match.</li>
              )}
            </ul>
          </div>

          {/* Moves */}
          <div className="border-t border-slate-800 pt-3">
            <div className="flex items-center justify-between mb-2">
              <div className="text-sm font-semibold">
                Moves for Step {totalSteps ? step + 1 : 0}
              </div>
              <span className="text-[10px] text-slate-400">
                Mode: {cur?.movesMode || "-"}
              </span>
            </div>
            <input
              type="text"
              placeholder="Filter (e.g., shoot, rotate, killed)"
              value={filterStr}
              onChange={(e) => setFilterStr(e.target.value)}
              className="w-full mb-2 px-3 py-2 rounded-xl bg-slate-800 border border-slate-700 text-sm"
              disabled={!cur?.movesTable.length}
            />
            {cur?.movesTable.length ? (
              stepMoves.length ? (
                <ul className="space-y-1 text-xs">
                  {stepMoves.map((t, i) => (
                    <li
                      key={i}
                      className="px-2 py-1 rounded-lg bg-slate-800/70 border border-slate-700/60"
                    >
                      {t}
                    </li>
                  ))}
                </ul>
              ) : (
                <div className="text-xs text-slate-400">
                  No moves for this step (or filtered out).
                </div>
              )
            ) : (
              <div className="text-xs text-slate-400">
                Load a moves folder to show actions.
              </div>
            )}

            {/* Summary */}
            {cur?.summary && (
              <div className="mt-4 border-t border-slate-800 pt-3 text-sm">
                <div className="font-semibold mb-1">Summary</div>
                <div className="grid grid-cols-2 gap-x-2 gap-y-1 text-slate-300">
                  <div className="text-slate-400">Winner</div>
                  <div>{cur.summary.winner ?? "-"}</div>
                  <div className="text-slate-400">Reason</div>
                  <div>{cur.summary.reason ?? "-"}</div>
                  <div className="text-slate-400">Total steps</div>
                  <div>
                    {cur.summary.totalSteps ?? (cur.frames.length || "-")}
                  </div>
                  <div className="text-slate-400">P1 remaining</div>
                  <div>{cur.summary.p1Remain ?? "-"}</div>
                  <div className="text-slate-400">P2 remaining</div>
                  <div>{cur.summary.p2Remain ?? "-"}</div>
                </div>
              </div>
            )}
          </div>
        </div>
      </div>
    </div>
  );
}

function legendItem(bgClasses: string, ch: string, label: string) {
  return (
    <div className="flex items-center gap-2 mr-3">
      <div
        className={`w-5 h-5 rounded ${bgClasses} flex items-center justify-center text-[11px] font-mono`}
      >
        {ch}
      </div>
      <span className="text-slate-400">{label}</span>
    </div>
  );
}
