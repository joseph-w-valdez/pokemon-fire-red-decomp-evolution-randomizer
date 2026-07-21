#!/usr/bin/env python3
"""Offline TypeIcon pill preview (no ROM / emulator needed).

Renders procedural blank pills the same way src/type_icon.c does
(FONT_SMALL + pokemon_types fills + ink AABB centering) and writes PNGs
you can open next to graphics/interface/menu_info.png.

Default writes into build/type_pills/:
  compare.png     — all types, stock | procedural (vertical)
  sheet.png       — procedural pills placed on the menu_info tile map
  sheet_stock.png — stock crop of that same map (reference)
  sheet_compare.png — stock map | procedural map side-by-side
  individual/     — one PNG per type (stock | procedural)

Examples:
  python3 tools/preview_type_pills.py
  python3 tools/preview_type_pills.py --prefix HP- --width fit --min 32 --max 56 --tile-align
  python3 tools/preview_type_pills.py --scale 8
  python3 tools/preview_type_pills.py --stats-only
"""

from __future__ import annotations

import argparse
import re
import struct
import sys
import zlib
from pathlib import Path

REPO = Path(__file__).resolve().parents[1]

PILL_W = 32
PILL_H = 12
IDX_KEY = 0
IDX_OUTLINE = 14
IDX_TEXT = 15
CHAR_A = 0xBB
CHAR_QUESTION = 0xAC
CHAR_HYPHEN = 0xAE
EOS = 0xFF

# Keep in sync with src/type_icon.c sTypeFillTop / sTypeFillBottom.
FILL_TOP = [9, 1, 6, 12, 4, 11, 4, 8, 9, 6, 2, 7, 5, 3, 12, 6, 7, 13]
FILL_BOTTOM = [9, 1, 9, 8, 11, 11, 4, 8, 13, 12, 2, 7, 5, 3, 12, 6, 1, 13]

TYPE_NAMES = [
    "NORMAL",
    "FIGHT",
    "FLYING",
    "POISON",
    "GROUND",
    "ROCK",
    "BUG",
    "GHOST",
    "STEEL",
    "???",
    "FIRE",
    "WATER",
    "GRASS",
    "ELECTR",
    "PSYCHC",
    "ICE",
    "DRAGON",
    "DARK",
]

# Stock crop offsets (tile index) from sMenuInfoIcons[TYPE_* + 1].
STOCK_OFFSETS = [
    0x20,
    0x64,
    0x60,
    0x80,
    0x48,
    0x44,
    0x6C,
    0x68,
    0x88,
    0xA4,
    0x24,
    0x28,
    0x2C,
    0x40,
    0x84,
    0x4C,
    0xA0,
    0x8C,
]

# From src/text_printer.c sFontHalfRowOffsets.
HALF_ROW_OFFSETS = [
    0x00, 0x01, 0x02, 0x00, 0x03, 0x04, 0x05, 0x03, 0x06, 0x07, 0x08, 0x06, 0x00, 0x01, 0x02, 0x00,
    0x09, 0x0A, 0x0B, 0x09, 0x0C, 0x0D, 0x0E, 0x0C, 0x0F, 0x10, 0x11, 0x0F, 0x09, 0x0A, 0x0B, 0x09,
    0x12, 0x13, 0x14, 0x12, 0x15, 0x16, 0x17, 0x15, 0x18, 0x19, 0x1A, 0x18, 0x12, 0x13, 0x14, 0x12,
    0x00, 0x01, 0x02, 0x00, 0x03, 0x04, 0x05, 0x03, 0x06, 0x07, 0x08, 0x06, 0x00, 0x01, 0x02, 0x00,
    0x1B, 0x1C, 0x1D, 0x1B, 0x1E, 0x1F, 0x20, 0x1E, 0x21, 0x22, 0x23, 0x21, 0x1B, 0x1C, 0x1D, 0x1B,
    0x24, 0x25, 0x26, 0x24, 0x27, 0x28, 0x29, 0x27, 0x2A, 0x2B, 0x2C, 0x2A, 0x24, 0x25, 0x26, 0x24,
    0x2D, 0x2E, 0x2F, 0x2D, 0x30, 0x31, 0x32, 0x30, 0x33, 0x34, 0x35, 0x33, 0x2D, 0x2E, 0x2F, 0x2D,
    0x1B, 0x1C, 0x1D, 0x1B, 0x1E, 0x1F, 0x20, 0x1E, 0x21, 0x22, 0x23, 0x21, 0x1B, 0x1C, 0x1D, 0x1B,
    0x36, 0x37, 0x38, 0x36, 0x39, 0x3A, 0x3B, 0x39, 0x3C, 0x3D, 0x3E, 0x3C, 0x36, 0x37, 0x38, 0x36,
    0x3F, 0x40, 0x41, 0x3F, 0x42, 0x43, 0x44, 0x42, 0x45, 0x46, 0x47, 0x45, 0x3F, 0x40, 0x41, 0x3F,
    0x48, 0x49, 0x4A, 0x48, 0x4B, 0x4C, 0x4D, 0x4B, 0x4E, 0x4F, 0x50, 0x4E, 0x48, 0x49, 0x4A, 0x48,
    0x36, 0x37, 0x38, 0x36, 0x39, 0x3A, 0x3B, 0x39, 0x3C, 0x3D, 0x3E, 0x3C, 0x36, 0x37, 0x38, 0x36,
    0x00, 0x01, 0x02, 0x00, 0x03, 0x04, 0x05, 0x03, 0x06, 0x07, 0x08, 0x06, 0x00, 0x01, 0x02, 0x00,
    0x09, 0x0A, 0x0B, 0x09, 0x0C, 0x0D, 0x0E, 0x0C, 0x0F, 0x10, 0x11, 0x0F, 0x09, 0x0A, 0x0B, 0x09,
    0x12, 0x13, 0x14, 0x12, 0x15, 0x16, 0x17, 0x15, 0x18, 0x19, 0x1A, 0x18, 0x12, 0x13, 0x14, 0x12,
    0x00, 0x01, 0x02, 0x00, 0x03, 0x04, 0x05, 0x03, 0x06, 0x07, 0x08, 0x06, 0x00, 0x01, 0x02, 0x00,
]


def gba_to_rgb8(val: int) -> tuple[int, int, int]:
    r = val & 31
    g = (val >> 5) & 31
    b = (val >> 10) & 31
    return r * 255 // 31, g * 255 // 31, b * 255 // 31


def load_jasc_pal(path: Path) -> list[tuple[int, int, int]]:
    lines = path.read_text(encoding="utf-8", errors="replace").splitlines()
    count = int(lines[2].strip())
    colors: list[tuple[int, int, int]] = []
    for line in lines[3 : 3 + count]:
        parts = line.split()
        colors.append((int(parts[0]), int(parts[1]), int(parts[2])))
    while len(colors) < 16:
        colors.append((0, 0, 0))
    return colors[:16]


def load_gbapal(path: Path) -> list[tuple[int, int, int]]:
    data = path.read_bytes()
    return [gba_to_rgb8(struct.unpack_from("<H", data, i * 2)[0]) for i in range(16)]


def load_palette(repo: Path) -> list[tuple[int, int, int]]:
    pal = repo / "graphics/interface/pokemon_types.pal"
    gba = repo / "graphics/interface/pokemon_types.gbapal"
    if pal.exists():
        return load_jasc_pal(pal)
    return load_gbapal(gba)


def read_nudge_defaults(repo: Path) -> tuple[int, int]:
    header = (repo / "include/type_icon.h").read_text(encoding="utf-8", errors="replace")
    mx = re.search(r"#define\s+TYPE_ICON_LABEL_NUDGE_X\s+(-?\d+)", header)
    my = re.search(r"#define\s+TYPE_ICON_LABEL_NUDGE_Y\s+(-?\d+)", header)
    return (int(mx.group(1)) if mx else 0, int(my.group(1)) if my else 0)


def load_font_small_widths(repo: Path) -> list[int]:
    text = (repo / "src/text.c").read_text(encoding="utf-8", errors="replace")
    start = text.index("sFontSmallLatinGlyphWidths[]")
    brace = text.index("{", start)
    end = text.index("};", brace)
    nums = [int(x) for x in re.findall(r"\d+", text[brace:end])]
    if len(nums) < 256:
        raise RuntimeError(f"expected >=256 font widths, got {len(nums)}")
    return nums


def encode_char(ch: str) -> int:
    if ch == "?":
        return CHAR_QUESTION
    if ch == "-":
        return CHAR_HYPHEN
    if "A" <= ch <= "Z":
        return CHAR_A + (ord(ch) - ord("A"))
    raise ValueError(f"unsupported char {ch!r}")


def name_to_codes(name: str) -> list[int]:
    return [encode_char(c) for c in name]


def resolve_width(
    font: FontSmall,
    codes: list[int],
    mode: str,
    fixed: int,
    min_w: int,
    max_w: int,
    pad_x: int,
    tile_align: bool,
) -> int:
    """Mirror TypeIcon_ResolveWidth (FIT uses advance width like GetStringWidth)."""
    if mode == "fit":
        label_w = sum(font.widths[c] for c in codes)
        w = label_w + 2 * pad_x
        if min_w:
            w = max(w, min_w)
        if max_w:
            w = min(w, max_w)
    else:
        w = fixed
    if tile_align:
        w = ((w + 7) // 8) * 8
    return max(2, w)


class FontSmall:
    """Mirrors DecompressGlyph_Small + DecompressGlyphTile for latin glyphs."""

    def __init__(self, font_path: Path, widths: list[int]):
        raw = font_path.read_bytes()
        self.words = list(struct.unpack(f"<{len(raw) // 2}H", raw))
        self.widths = widths
        self.lut = self._build_lut(IDX_TEXT, IDX_KEY, IDX_OUTLINE)

    @staticmethod
    def _build_lut(fg: int, bg: int, shadow: int) -> list[int]:
        colors = (bg, fg, shadow)
        lut: list[int] = []
        for i in range(3):
            for j in range(3):
                for k in range(3):
                    for l in range(3):
                        lut.append(
                            (colors[l] << 12)
                            | (colors[k] << 8)
                            | (colors[j] << 4)
                            | colors[i]
                        )
        return lut

    def _decompress_tile(self, src_off: int) -> bytes:
        # 16 half-rows → 32 bytes (8x8 @ 4bpp). Matches DecompressGlyphTile:
        # even i → high byte (no advance), odd i → low byte (advance) — C uses (i << 31).
        dest = [0] * 16
        src = src_off
        for i in range(16):
            word = self.words[src]
            if i & 1:
                offset_index = word & 0xFF
                src += 1
            else:
                offset_index = word >> 8
            dest[i] = self.lut[HALF_ROW_OFFSETS[offset_index]]
        out = bytearray(32)
        for i, half in enumerate(dest):
            struct.pack_into("<H", out, i * 2, half)
        return bytes(out)

    def glyph(self, glyph_id: int) -> tuple[bytearray, int, int]:
        # Two 8x8 tiles stacked; DecompressGlyph_Small → pixels+0 / pixels+0x40, height 13.
        base = 0x10 * glyph_id
        top = self._decompress_tile(base)
        bot = self._decompress_tile(base + 8)
        pixels = bytearray(0x80)
        pixels[0:32] = top
        pixels[0x40 : 0x40 + 32] = bot
        width = self.widths[glyph_id]
        return pixels, width, 13

    @staticmethod
    def row_nibbles(pixels: bytearray, y: int, width: int) -> list[int]:
        if y < 8:
            src = pixels[y * 4 : y * 4 + 4]
        else:
            src = pixels[0x40 + (y - 8) * 4 : 0x40 + (y - 8) * 4 + 4]
        pixrow = struct.unpack("<I", src)[0]
        return [(pixrow >> (x * 4)) & 0xF for x in range(width)]


def measure_ink(font: FontSmall, codes: list[int]) -> tuple[int, int, int, int] | None:
    """White (fg idx 15) AABB only — matches TypeIcon_AccumulateGlyphInk / stock."""
    min_x = min_y = 10**9
    max_x = max_y = -10**9
    pen_x = 0
    has = False
    for gid in codes:
        pixels, width, height = font.glyph(gid)
        for y in range(height):
            nibs = font.row_nibbles(pixels, y, width)
            for x, n in enumerate(nibs):
                if n != IDX_TEXT:
                    continue
                gx, gy = pen_x + x, y
                min_x = min(min_x, gx)
                min_y = min(min_y, gy)
                max_x = max(max_x, gx)
                max_y = max(max_y, gy)
                has = True
        pen_x += width
    if not has:
        return None
    return min_x, min_y, max_x, max_y


def label_origin(
    font: FontSmall,
    codes: list[int],
    pill_x: int,
    pill_y: int,
    pill_w: int,
    pill_h: int,
    nudge_x: int,
    nudge_y: int,
) -> tuple[int, int]:
    # Same leftover-pad math as TypeIcon_GetLabelOrigin (avoids signed / truncation).
    ink = measure_ink(font, codes)
    if ink is None:
        text_w = sum(font.widths[c] for c in codes)
        return pill_x + (pill_w - text_w) // 2 + nudge_x, pill_y + nudge_y
    min_x, min_y, max_x, max_y = ink
    ink_w = max_x - min_x + 1
    ink_h = max_y - min_y + 1
    return (
        pill_x + (pill_w - ink_w) // 2 - min_x + nudge_x,
        pill_y + (pill_h - ink_h) // 2 - min_y + nudge_y,
    )

def draw_procedural(
    font: FontSmall,
    type_id: int,
    nudge_x: int,
    nudge_y: int,
    surround: int = IDX_KEY,
    prefix: str = "",
    pill_w: int | None = None,
) -> list[list[int]]:
    """Return PILL_H x width palette indices."""
    label = prefix + TYPE_NAMES[type_id]
    codes = name_to_codes(label)
    width = pill_w if pill_w is not None else PILL_W
    buf = [[surround for _ in range(width)] for _ in range(PILL_H)]
    top = FILL_TOP[type_id]
    bot = FILL_BOTTOM[type_id]
    half = PILL_H // 2
    for y in range(half):
        for x in range(width):
            buf[y][x] = top
    for y in range(half, PILL_H):
        for x in range(width):
            buf[y][x] = bot
    for x, y in ((0, 0), (width - 1, 0), (0, PILL_H - 1), (width - 1, PILL_H - 1)):
        buf[y][x] = surround

    ox, oy = label_origin(font, codes, 0, 0, width, PILL_H, nudge_x, nudge_y)
    pen_x = ox
    for gid in codes:
        pixels, gw, height = font.glyph(gid)
        for y in range(height):
            ypos = oy + y
            if ypos < 0 or ypos >= PILL_H:
                continue
            nibs = font.row_nibbles(pixels, y, gw)
            for x, n in enumerate(nibs):
                if n == 0:
                    continue
                xpos = pen_x + x
                if 0 <= xpos < width:
                    buf[ypos][xpos] = n
        pen_x += gw
    return buf


def decode_menu_info_sheet(data: bytes) -> list[list[int]]:
    """128x128 sheet of 4bpp tiles, 16 tiles wide."""
    sheet_w, sheet_h = 128, 128
    out = [[0] * sheet_w for _ in range(sheet_h)]

    def decode_tile(tile_idx: int) -> list[list[int]]:
        off = tile_idx * 32
        pix = [[0] * 8 for _ in range(8)]
        for y in range(8):
            for xpair in range(4):
                b = data[off + y * 4 + xpair]
                pix[y][xpair * 2] = b & 0xF
                pix[y][xpair * 2 + 1] = (b >> 4) & 0xF
        return pix

    for ty in range(sheet_h // 8):
        for tx in range(sheet_w // 8):
            tile = decode_tile(ty * 16 + tx)
            for y in range(8):
                for x in range(8):
                    out[ty * 8 + y][tx * 8 + x] = tile[y][x]
    return out


def crop_stock(sheet: list[list[int]], tile_offset: int) -> list[list[int]]:
    ox = (tile_offset % 16) * 8
    oy = (tile_offset // 16) * 8
    return [row[ox : ox + PILL_W] for row in sheet[oy : oy + PILL_H]]


def ink_pads(buf: list[list[int]]) -> tuple[int, int, int, int, int, int] | None:
    ink = [(x, y) for y, row in enumerate(buf) for x, v in enumerate(row) if v in (14, 15)]
    if not ink:
        return None
    xs = [p[0] for p in ink]
    ys = [p[1] for p in ink]
    ix0, ix1 = min(xs), max(xs)
    iy0, iy1 = min(ys), max(ys)
    pill_w = len(buf[0])
    left = ix0
    right = pill_w - 1 - ix1
    top = iy0
    bottom = PILL_H - 1 - iy1
    return left, right, top, bottom, ix1 - ix0 + 1, iy1 - iy0 + 1


def indices_to_rgb(
    buf: list[list[int]], palette: list[tuple[int, int, int]], chroma0: tuple[int, int, int]
) -> list[list[tuple[int, int, int]]]:
    out: list[list[tuple[int, int, int]]] = []
    for row in buf:
        out.append([chroma0 if v == 0 else palette[v] for v in row])
    return out


def scale_rgb(
    img: list[list[tuple[int, int, int]]], scale: int
) -> list[list[tuple[int, int, int]]]:
    if scale <= 1:
        return img
    h = len(img)
    w = len(img[0])
    out: list[list[tuple[int, int, int]]] = []
    for y in range(h * scale):
        src = img[y // scale]
        row: list[tuple[int, int, int]] = []
        for x in range(w * scale):
            row.append(src[x // scale])
        out.append(row)
    return out


def write_png(path: Path, pixels: list[list[tuple[int, int, int]]]) -> None:
    h = len(pixels)
    w = len(pixels[0])
    raw = bytearray()
    for row in pixels:
        raw.append(0)  # filter None
        for r, g, b in row:
            raw.extend((r, g, b))
    compressed = zlib.compress(bytes(raw), 9)

    def chunk(tag: bytes, data: bytes) -> bytes:
        return struct.pack(">I", len(data)) + tag + data + struct.pack(">I", zlib.crc32(tag + data) & 0xFFFFFFFF)

    ihdr = struct.pack(">IIBBBBB", w, h, 8, 2, 0, 0, 0)
    png = b"\x89PNG\r\n\x1a\n" + chunk(b"IHDR", ihdr) + chunk(b"IDAT", compressed) + chunk(b"IEND", b"")
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_bytes(png)


def blit_indices(
    canvas: list[list[tuple[int, int, int]]],
    src: list[list[int]],
    palette: list[tuple[int, int, int]],
    dx: int,
    dy: int,
    chroma0: tuple[int, int, int],
) -> None:
    rgb = indices_to_rgb(src, palette, chroma0)
    for y, row in enumerate(rgb):
        for x, c in enumerate(row):
            canvas[dy + y][dx + x] = c


def build_compare_strip(
    stocks: list[list[list[int]]],
    procs: list[list[list[int]]],
    palette: list[tuple[int, int, int]],
    scale: int,
    gap: int = 2,
    pad: int = 4,
) -> list[list[tuple[int, int, int]]]:
    """Vertical list: stock | procedural for every type."""
    bg = (40, 40, 48)
    n = len(TYPE_NAMES)
    left_w = max(len(s[0]) for s in stocks)
    right_w = max(len(p[0]) for p in procs)
    sheet_w = pad * 2 + left_w + gap + right_w
    sheet_h = pad * 2 + n * PILL_H + (n - 1) * gap
    canvas = [[bg for _ in range(sheet_w)] for _ in range(sheet_h)]
    for i in range(n):
        y = pad + i * (PILL_H + gap)
        blit_indices(canvas, stocks[i], palette, pad, y, bg)
        blit_indices(canvas, procs[i], palette, pad + left_w + gap, y, bg)
    return scale_rgb(canvas, scale)


def build_pair(
    stock: list[list[int]],
    proc: list[list[int]],
    palette: list[tuple[int, int, int]],
    scale: int,
    gap: int = 2,
    pad: int = 4,
) -> list[list[tuple[int, int, int]]]:
    bg = (40, 40, 48)
    left_w = len(stock[0])
    right_w = len(proc[0])
    w = pad * 2 + left_w + gap + right_w
    h = pad * 2 + PILL_H
    canvas = [[bg for _ in range(w)] for _ in range(h)]
    blit_indices(canvas, stock, palette, pad, pad, bg)
    blit_indices(canvas, proc, palette, pad + left_w + gap, pad, bg)
    return scale_rgb(canvas, scale)


def build_menu_info_map(
    pills: list[list[list[int]]],
    palette: list[tuple[int, int, int]],
    scale: int,
    base_sheet: list[list[int]] | None = None,
) -> list[list[tuple[int, int, int]]]:
    """Place pills into the same 128x128 tile slots as menu_info.4bpp (clips to 32px)."""
    bg = (40, 40, 48)
    if base_sheet is None:
        indices = [[IDX_KEY for _ in range(128)] for _ in range(128)]
    else:
        indices = [row[:] for row in base_sheet]

    for pill, off in zip(pills, STOCK_OFFSETS):
        ox = (off % 16) * 8
        oy = (off // 16) * 8
        pw = min(PILL_W, len(pill[0]))
        for y in range(PILL_H):
            for x in range(pw):
                indices[oy + y][ox + x] = pill[y][x]

    rgb = indices_to_rgb(indices, palette, bg)
    return scale_rgb(rgb, scale)


def safe_name(name: str) -> str:
    return "MYSTERY" if name == "???" else name


def main() -> int:
    repo = REPO
    def_nx, def_ny = read_nudge_defaults(repo)
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument(
        "--out-dir",
        type=Path,
        default=repo / "build/type_pills",
        help="directory for compare / sheet / individual PNGs",
    )
    ap.add_argument(
        "--out",
        type=Path,
        default=None,
        help="optional single compare-strip path (in addition to --out-dir)",
    )
    ap.add_argument("--scale", type=int, default=6, help="nearest-neighbor upscale")
    ap.add_argument("--nudge-x", type=int, default=None, help=f"override TYPE_ICON_LABEL_NUDGE_X (default {def_nx})")
    ap.add_argument("--nudge-y", type=int, default=None, help=f"override TYPE_ICON_LABEL_NUDGE_Y (default {def_ny})")
    ap.add_argument("--stats-only", action="store_true", help="print pad table only, no PNG")
    ap.add_argument("--no-label", action="store_true", help="procedural body only (no text)")
    ap.add_argument("--no-individual", action="store_true", help="skip per-type PNGs")
    ap.add_argument("--no-sheet", action="store_true", help="skip menu_info map sheets")
    ap.add_argument("--prefix", type=str, default="", help='label prefix, e.g. "HP-" (MAKEOVER)')
    ap.add_argument("--width", choices=("fixed", "fit"), default="fixed", help="TypeIcon width mode")
    ap.add_argument("--fixed", type=int, default=32, help="FIXED mode width")
    ap.add_argument("--min", type=int, default=0, dest="min_w", help="FIT min width (0=none)")
    ap.add_argument("--max", type=int, default=0, dest="max_w", help="FIT max width (0=none)")
    ap.add_argument("--pad-x", type=int, default=2, help="FIT side pad each side")
    ap.add_argument("--tile-align", action="store_true", help="ceil width to 8px")
    args = ap.parse_args()

    nudge_x = def_nx if args.nudge_x is None else args.nudge_x
    nudge_y = def_ny if args.nudge_y is None else args.nudge_y
    scale = max(1, args.scale)
    prefix = args.prefix.upper() if args.prefix else ""

    palette = load_palette(repo)
    widths = load_font_small_widths(repo)
    font = FontSmall(repo / "graphics/fonts/latin_small.hwlatfont", widths)
    sheet = decode_menu_info_sheet((repo / "graphics/interface/menu_info.4bpp").read_bytes())

    stocks = [crop_stock(sheet, off) for off in STOCK_OFFSETS]
    resolved_ws: list[int] = []
    if args.no_label:
        procs = []
        for tid in range(len(TYPE_NAMES)):
            w = args.fixed if args.width == "fixed" else 32
            resolved_ws.append(w)
            buf = [[IDX_KEY] * w for _ in range(PILL_H)]
            top, bot = FILL_TOP[tid], FILL_BOTTOM[tid]
            half = PILL_H // 2
            for y in range(half):
                for x in range(w):
                    buf[y][x] = top
            for y in range(half, PILL_H):
                for x in range(w):
                    buf[y][x] = bot
            for x, y in ((0, 0), (w - 1, 0), (0, PILL_H - 1), (w - 1, PILL_H - 1)):
                buf[y][x] = IDX_KEY
            procs.append(buf)
    else:
        procs = []
        for tid in range(len(TYPE_NAMES)):
            codes = name_to_codes(prefix + TYPE_NAMES[tid])
            w = resolve_width(
                font, codes, args.width, args.fixed, args.min_w, args.max_w, args.pad_x, args.tile_align
            )
            resolved_ws.append(w)
            procs.append(
                draw_procedural(font, tid, nudge_x, nudge_y, prefix=prefix, pill_w=w)
            )

    print(
        f"nudge=({nudge_x},{nudge_y}) prefix={prefix!r} width={args.width} "
        f"min={args.min_w} max={args.max_w} padX={args.pad_x} tileAlign={args.tile_align}"
    )
    print(f"{'type':8} {'W':>3}  {'stock LRTB':14} {'proc LRTB':14}")
    for i, name in enumerate(TYPE_NAMES):
        sp = ink_pads(stocks[i])
        pp = ink_pads(procs[i])
        sw = resolved_ws[i]
        if sp is None or pp is None:
            print(f"{name:8} {sw:3}  (no ink)")
            continue
        sl, sr, st, sb, _, _ = sp
        # proc pads relative to its own width
        pl, pr, pt, pb, _, _ = pp
        # recompute right pad for variable width
        ink = [(x, y) for y, row in enumerate(procs[i]) for x, v in enumerate(row) if v in (14, 15)]
        if ink:
            xs = [p[0] for p in ink]
            pl = min(xs)
            pr = len(procs[i][0]) - 1 - max(xs)
        print(f"{name:8} {sw:3}  {sl},{sr},{st},{sb:<4}     {pl},{pr},{pt},{pb}")

    if args.stats_only:
        return 0

    # Wider-than-stock pills don't fit menu_info tile slots cleanly.
    if prefix or args.width == "fit":
        args.no_sheet = True

    out_dir: Path = args.out_dir
    out_dir.mkdir(parents=True, exist_ok=True)
    written: list[Path] = []

    compare = build_compare_strip(stocks, procs, palette, scale)
    compare_path = out_dir / "compare.png"
    write_png(compare_path, compare)
    written.append(compare_path)

    if args.out is not None:
        write_png(args.out, compare)
        written.append(args.out)

    if not args.no_sheet:
        stock_map = build_menu_info_map(stocks, palette, scale, base_sheet=sheet)
        proc_map = build_menu_info_map(procs, palette, scale, base_sheet=sheet)
        stock_path = out_dir / "sheet_stock.png"
        proc_path = out_dir / "sheet.png"
        write_png(stock_path, stock_map)
        write_png(proc_path, proc_map)
        written.extend([stock_path, proc_path])

        # Side-by-side stock map | procedural map for easy flick review.
        gap = 4 * scale
        sh, sw = len(stock_map), len(stock_map[0])
        side = [[(40, 40, 48) for _ in range(sw * 2 + gap)] for _ in range(sh)]
        for y in range(sh):
            for x in range(sw):
                side[y][x] = stock_map[y][x]
                side[y][sw + gap + x] = proc_map[y][x]
        side_path = out_dir / "sheet_compare.png"
        write_png(side_path, side)
        written.append(side_path)

    if not args.no_individual:
        ind_dir = out_dir / "individual"
        ind_dir.mkdir(parents=True, exist_ok=True)
        for i, name in enumerate(TYPE_NAMES):
            path = ind_dir / f"{safe_name(name)}.png"
            write_png(path, build_pair(stocks[i], procs[i], palette, scale))
            written.append(path)

    print(f"wrote {len(written)} PNG(s) under {out_dir} (scale={scale})")
    for p in written[:4]:
        print(f"  {p}")
    if len(written) > 4:
        print(f"  ... and {len(written) - 4} more in individual/")
    return 0


if __name__ == "__main__":
    sys.exit(main())
