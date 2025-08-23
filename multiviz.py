#!/usr/bin/env python3
import os, sys, math, argparse, pygame

# --------- Colors / UI ----------
BG_COLOR     = (18, 18, 24)
PANEL_BG     = (28, 28, 36)
GRID_COLOR   = (60, 60, 72)
WALL_COLOR   = (120, 120, 130)
HEADER_BG    = (40, 40, 52)
HEADER_TEXT  = (250, 250, 255)
TEXT_COLOR   = (230, 230, 235)
BTN_BG       = (58, 58, 72)
BTN_BG_HOVER = (76, 76, 96)
BTN_TEXT     = (245, 245, 250)

ASSET_DIRS = [os.getcwd(), os.path.dirname(__file__)]

def find_asset(name):
    for base in ASSET_DIRS:
        p = os.path.join(base, name)
        if os.path.isfile(p): return p
    return None

def load_image(name, fallback=(200,200,200), size=None):
    path = find_asset(name)
    if not path:
        surf = pygame.Surface(size or (32,32), pygame.SRCALPHA)
        surf.fill((*fallback,255))
        return surf
    img = pygame.image.load(path).convert_alpha()
    if size: img = pygame.transform.smoothscale(img, size)
    return img

# --------- Parse .viz.txt ----------
def parse_viz_txt(path):
    with open(path, "r", encoding="utf-8") as f:
        lines = [ln.rstrip("\n") for ln in f]
    rounds, i, step = [], 0, -1
    while i < len(lines):
        ln = lines[i].strip()
        if ln.startswith("===") and "Game Step" in ln:
            try:
                parts = ln.split(); idx = parts.index("Step"); step = int(parts[idx+1])
            except Exception:
                step += 1
            i += 1
            grid = []
            while i < len(lines):
                nxt = lines[i].strip()
                if nxt.startswith("==="): break  # <-- fixed
                if nxt == "" and (i+1 < len(lines) and lines[i+1].strip().startswith("===")): break
                grid.append(lines[i]); i += 1
            W = max((len(r) for r in grid), default=0)
            grid = [row + " "*(W - len(row)) for row in grid]
            rounds.append({"round": step, "cells": grid})
        else:
            i += 1
    if not rounds:
        raise ValueError(f"No steps found in {os.path.basename(path)}")
    W = max((max((len(r) for r in fr["cells"]), default=0) for fr in rounds), default=0)
    H = max((len(fr["cells"]) for fr in rounds), default=0)
    for fr in rounds:
        rows = fr["cells"]
        if len(rows) < H: rows += [""] * (H - len(rows))
        fr["cells"] = [row + " "*(W - len(row)) for row in rows]
    return {"width": W, "height": H, "rounds": rounds, "meta": {"source": os.path.basename(path)}}

# --------- Naming helpers ----------
def common_prefix(strings):
    if not strings: return ""
    s1, s2 = min(strings), max(strings)
    i = 0
    while i < len(s1) and s1[i] == s2[i]: i += 1
    return s1[:i]

def common_suffix(strings):
    rev = [s[::-1] for s in strings]
    return common_prefix(rev)[::-1]

def compress_middle(s, max_len=24):
    if len(s) <= max_len: return s
    keep = max_len - 3; left = keep // 2; right = keep - left
    return s[:left] + "…" + s[-right:]

def make_short_unique_names(paths):
    bases = [os.path.basename(p) for p in paths]
    pre = common_prefix(bases); suf = common_suffix(bases)
    shorts = []
    for i, b in enumerate(bases, 1):
        core = b[len(pre):len(b)-len(suf)] if len(suf) else b[len(pre):]
        core = core.strip("_-. ") or f"game{i}"
        shorts.append(compress_middle(core, 24))
    seen = {}; out = []
    for i, s in enumerate(shorts, 1):
        if s not in seen: seen[s] = 1; out.append(s)
        else: seen[s]+=1; out.append(f"{s}#{seen[s]}")
    return out

# --------- Panel for one game ----------
class GamePanel:
    NAME_BAR_H = 28   # height if names are visible
    PAD = 8
    def __init__(self, rect, font, sfont, assets, data, display_name):
        self.rect = pygame.Rect(rect)
        self.font = font
        self.sfont = sfont
        self.assets = assets
        self.data = data
        self.display_name = display_name
        self.full_name = data["meta"].get("source","")
        self.cell = 16
        self._scaled = {}

    def _compute_cell(self, name_bar_visible):
        W,H = self.data["width"], self.data["height"]
        aw = self.rect.width - 2*self.PAD
        ah = self.rect.height - ( (self.NAME_BAR_H if name_bar_visible else 0) + 2*self.PAD)
        if W==0 or H==0: return 16
        return max(4, min(aw//W, ah//H))

    def _scale_assets(self, name_bar_visible):
        cell = self._compute_cell(name_bar_visible)
        if cell == self.cell and self._scaled: return
        self.cell = cell
        self._scaled = {
            "tank1": pygame.transform.smoothscale(self.assets["tank1"], (cell,cell)),
            "tank2": pygame.transform.smoothscale(self.assets["tank2"], (cell,cell)),
            "mine" : pygame.transform.smoothscale(self.assets["mine"],  (cell,cell)),
        }

    def draw(self, surf, frame_idx, hold_last=True, show_names=False, show_full=False):
        self._scale_assets(show_names)
        # panel bg
        pygame.draw.rect(surf, PANEL_BG, self.rect, border_radius=14)

        top_y = self.rect.y
        # optional name/header bar
        if show_names:
            hdr = pygame.Rect(self.rect.x, self.rect.y, self.rect.w, self.NAME_BAR_H)
            pygame.draw.rect(surf, HEADER_BG, hdr, border_radius=14)
            name = self.full_name if show_full else self.display_name
            surf.blit(self.font.render(name, True, HEADER_TEXT), (self.rect.x+8, self.rect.y+6))
            top_y = hdr.bottom

        rounds = self.data["rounds"]
        if not rounds:
            return

        frame = rounds[-1] if frame_idx >= len(rounds) and hold_last else rounds[min(frame_idx, len(rounds)-1)]

        sub = self.sfont.render(
            f"Frame {min(frame_idx+1,len(rounds))}/{len(rounds)} • Round {frame.get('round','-')}",
            True, (210,210,220))
        surf.blit(sub, (self.rect.x+8, top_y + 4))

        # board origin
        W,H = self.data["width"], self.data["height"]
        bw,bh = W*self.cell, H*self.cell
        left = self.rect.x + (self.rect.w - bw)//2
        grid_top = top_y + sub.get_height() + 8

        # grid
        for r in range(H+1):
            y = grid_top + r*self.cell
            pygame.draw.line(surf, GRID_COLOR, (left,y), (left+bw,y), 1)
        for c in range(W+1):
            x = left + c*self.cell
            pygame.draw.line(surf, GRID_COLOR, (x,grid_top), (x,grid_top+bh), 1)

        # cells
        rows = frame["cells"]
        for r in range(H):
            row = rows[r]
            for c in range(W):
                ch = row[c]; x = left + c*self.cell; y = grid_top + r*self.cell
                if ch == "#": pygame.draw.rect(surf, WALL_COLOR, (x+1,y+1,self.cell-2,self.cell-2), border_radius=4)
                elif ch == "@": surf.blit(self._scaled["mine"], (x,y))
                elif ch == "1": surf.blit(self._scaled["tank1"], (x,y))
                elif ch == "2": surf.blit(self._scaled["tank2"], (x,y))

# --------- Simple Button ----------
class Button:
    def __init__(self, rect, label, on_click):
        self.rect = pygame.Rect(rect)
        self.label = label
        self.on_click = on_click
        self.hover=False
    def draw(self, surf, font):
        pygame.draw.rect(surf, BTN_BG_HOVER if self.hover else BTN_BG, self.rect, border_radius=8)
        txt = font.render(self.label, True, BTN_TEXT)
        surf.blit(txt, (self.rect.centerx - txt.get_width()//2, self.rect.centery - txt.get_height()//2))
    def handle(self, event):
        if event.type == pygame.MOUSEMOTION:
            self.hover = self.rect.collidepoint(event.pos)
        elif event.type == pygame.MOUSEBUTTONDOWN and event.button == 1 and self.rect.collidepoint(event.pos):
            self.on_click()

# --------- App ----------
class MultiViz:
    HUD_H = 48
    def __init__(self, folder, fps=8, hold_last=True):
        pygame.init()
        pygame.display.set_caption("TankGame Multi‑Viewer")
        self.clock = pygame.time.Clock()
        self.fps = fps
        self.hold_last = hold_last

        self.screen = pygame.display.set_mode((1280, 800), pygame.RESIZABLE)
        self.font  = pygame.font.SysFont("Arial", 16)
        self.sfont = pygame.font.SysFont("Arial", 13)

        self.assets = {
            "tank1": load_image("tank_blue.png", (80,160,255), (32,32)),
            "tank2": load_image("tank_red.png",  (255,80,80),  (32,32)),
            "mine" : load_image("mine_skull.png",(230,230,230),(32,32)),
        }

        raw_paths = sorted([os.path.join(folder,f) for f in os.listdir(folder) if f.endswith(".viz.txt")])
        if not raw_paths:
            raise SystemExit(f"No .viz.txt files found in: {folder}")

        # parse data
        self.data = []
        for p in raw_paths:
            try: self.data.append(parse_viz_txt(p))
            except Exception as e: print(f"[WARN] Skipping {os.path.basename(p)}: {e}", file=sys.stderr)

        self.short_names = make_short_unique_names(raw_paths)

        self.n = len(self.data)
        self.panels = []
        self._layout_cache = (0,0,0)
        self.frame = 0
        self.paused = False

        # name display toggles
        self.show_names = False       # <-- default: hidden
        self.show_full_names = False  # short vs full when visible

        self.max_frames = max(len(d["rounds"]) for d in self.data)
        self._layout()
        self._build_buttons()

    # layout of panels
    def _layout(self):
        cols = math.ceil(math.sqrt(self.n)); rows = math.ceil(self.n / cols)
        W,H = self.screen.get_width(), self.screen.get_height() - self.HUD_H
        pw, ph = W // cols, H // rows
        rects = []
        for i in range(self.n):
            r, c = divmod(i, cols)
            x, y = c*pw + 6, self.HUD_H + r*ph + 6
            rects.append((x, y, pw-12, ph-12))
        self.panels = [GamePanel(rects[i], self.font, self.sfont, self.assets, self.data[i], self.short_names[i])
                       for i in range(self.n)]
        self._layout_cache = (self.screen.get_width(), self.screen.get_height(), self.n)

    def _maybe_layout(self):
        cur = (self.screen.get_width(), self.screen.get_height(), self.n)
        if cur != self._layout_cache: self._layout()

    # HUD buttons
    def _build_buttons(self):
        x = 10; y = 6; w = 82; h = 36; gap = 8
        def mk(label, cb):
            nonlocal x
            b = Button((x,y,w,h), label, cb); x += w + gap; return b
        self.btns = [
            mk("⏮ -10", lambda: self._pause_and_step(-10)),
            mk("◀ -1",  lambda: self._pause_and_step(-1)),
            mk("⏯ Play" if self.paused else "⏸ Pause", self._toggle_play),
            mk("+1 ▶",  lambda: self._pause_and_step(+1)),
            mk("+10 ⏭", lambda: self._pause_and_step(+10)),
            mk("Show Names", self._toggle_names),    # <-- new: show/hide entirely
            mk("Full",        self._toggle_full),    # when names are shown: short vs full
            mk("Blank",       self._toggle_blank),
            mk("Speed-",      lambda: self._speed(-1)),
            mk("Speed+",      lambda: self._speed(+1)),
        ]

    def _refresh_play_label(self):
        for b in self.btns:
            if "⏯" in b.label or "⏸" in b.label:
                b.label = "⏯ Play" if self.paused else "⏸ Pause"

    # actions
    def _pause_and_step(self, delta):
        self.paused = True
        self.frame = max(0, min(self.max_frames-1, self.frame + delta))

    def _toggle_play(self):
        self.paused = not self.paused
        self._refresh_play_label()

    def _toggle_names(self):
        self.show_names = not self.show_names
        # Button label reflect state
        for b in self.btns:
            if b.label in ("Show Names", "Hide Names"):
                b.label = "Hide Names" if self.show_names else "Show Names"
                break

    def _toggle_full(self):
        self.show_full_names = not self.show_full_names

    def _toggle_blank(self):
        self.hold_last = not self.hold_last

    def _speed(self, delta):
        self.fps = max(1, min(60, self.fps + delta))

    # draw & events
    def draw(self):
        self.screen.fill(BG_COLOR)
        # HUD bar
        pygame.draw.rect(self.screen, HEADER_BG, (0,0,self.screen.get_width(), self.HUD_H))
        title = self.font.render(
            f"{self.n} games • Frame {self.frame+1}/{self.max_frames} • FPS {self.fps} • {'PAUSED' if self.paused else 'PLAYING'}",
            True, HEADER_TEXT)
        self.screen.blit(title, (10, 10))
        for b in self.btns: b.draw(self.screen, self.font)

        self._maybe_layout()
        for p in self.panels:
            p.draw(self.screen, self.frame,
                   hold_last=self.hold_last,
                   show_names=self.show_names,
                   show_full=self.show_full_names)
        pygame.display.flip()

    def run(self):
        while True:
            for ev in pygame.event.get():
                if ev.type == pygame.QUIT: return
                if ev.type == pygame.VIDEORESIZE:
                    self.screen = pygame.display.set_mode((ev.w, ev.h), pygame.RESIZABLE)
                    self._layout()
                for b in self.btns: b.handle(ev)
                if ev.type == pygame.KEYDOWN:
                    if ev.key == pygame.K_ESCAPE: return
                    if ev.key in (pygame.K_SPACE, pygame.K_p): self._toggle_play()
                    if ev.key == pygame.K_RIGHT: self._pause_and_step(+1)
                    if ev.key == pygame.K_LEFT:  self._pause_and_step(-1)
                    if ev.key == pygame.K_RIGHTBRACKET: self._pause_and_step(+10)
                    if ev.key == pygame.K_LEFTBRACKET:  self._pause_and_step(-10)
                    if ev.key == pygame.K_n: self._toggle_names()     # <-- keyboard toggle names
                    if ev.key == pygame.K_f: self._toggle_full()
                    if ev.key == pygame.K_b: self._toggle_blank()
                    if ev.key in (pygame.K_PLUS, pygame.K_EQUALS): self._speed(+1)
                    if ev.key in (pygame.K_MINUS, pygame.K_UNDERSCORE): self._speed(-1)

            if not self.paused:
                self.frame = (self.frame + 1) % self.max_frames
            self.draw()
            self.clock.tick(self.fps)

# --------- CLI ----------
def main():
    ap = argparse.ArgumentParser(description="Visualize multiple .viz.txt games in sync (with on‑screen controls).")
    ap.add_argument("--folder", required=True, help="Folder containing *.viz.txt files")
    ap.add_argument("--fps", type=int, default=8, help="Playback speed (frames/sec)")
    ap.add_argument("--blank-after-end", action="store_true",
                    help="Blank panels after they reach last frame (default: hold last frame)")
    args = ap.parse_args()
    if not os.path.isdir(args.folder):
        print(f"Folder not found: {args.folder}", file=sys.stderr); sys.exit(1)
    app = MultiViz(args.folder, fps=args.fps, hold_last=(not args.blank_after_end))
    app.run()

if __name__ == "__main__":
    main()
