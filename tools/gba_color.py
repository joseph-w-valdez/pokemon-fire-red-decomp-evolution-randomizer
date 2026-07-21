#!/usr/bin/env python3
"""GBA palette / color DevX helper.

Convert CSS hex / RGB8 ↔ 15-bit GBA colors, dump JASC `.pal` / binary
`.gbapal` files, and find the closest palette index for a target color.

Examples:
  python3 tools/gba_color.py '#007BC5'
  python3 tools/gba_color.py 0 123 197
  python3 tools/gba_color.py --from-gba 0x61E0
  python3 tools/gba_color.py --pal graphics/text_window/stdpal_0.pal
  python3 tools/gba_color.py --pal graphics/text_window/stdpal_0.pal --match '#007BC5'
  python3 tools/gba_color.py --pal path/to/file.gbapal --match 0 123 197 --top 5
  python3 tools/gba_color.py --c-snippet '#007BC5' --index 1 --bg-pal 5
"""

from __future__ import annotations

import argparse
import struct
import sys
from pathlib import Path


def rgb8_to_gba(r: int, g: int, b: int) -> tuple[int, int, int, int]:
    """Match include/constants/rgb.h RGB8 (truncating integer divide)."""
    R = (r * 31) // 255
    G = (g * 31) // 255
    B = (b * 31) // 255
    val = R | (G << 5) | (B << 10)
    return val, R, G, B


def gba_to_rgb8(val: int) -> tuple[int, int, int]:
    R = val & 31
    G = (val >> 5) & 31
    B = (val >> 10) & 31
    return R * 255 // 31, G * 255 // 31, B * 255 // 31


def parse_hex(s: str) -> tuple[int, int, int]:
    s = s.strip().removeprefix("#").removeprefix("0x")
    if len(s) == 3:
        s = "".join(ch * 2 for ch in s)
    if len(s) != 6:
        raise ValueError(f"expected RRGGBB, got {s!r}")
    v = int(s, 16)
    return (v >> 16) & 255, (v >> 8) & 255, v & 255


def parse_color_args(values: list[str]) -> tuple[int, int, int]:
    if len(values) == 1:
        return parse_hex(values[0])
    if len(values) == 3:
        return int(values[0]), int(values[1]), int(values[2])
    raise ValueError("pass hex (#RRGGBB) or R G B (0–255)")


def color_distance(a: tuple[int, int, int], b: tuple[int, int, int]) -> int:
    return sum((x - y) * (x - y) for x, y in zip(a, b))


def format_color_block(r: int, g: int, b: int) -> None:
    val, R, G, B = rgb8_to_gba(r, g, b)
    print(f"RGB8 {r}, {g}, {b}")
    print(f"HEX  #{r:02X}{g:02X}{b:02X}")
    print(f"GBA  0x{val:04X}  channels {R},{G},{B}")
    print(f"C    RGB8({r}, {g}, {b})")
    print(f"C    RGB_HEX(0x{r:02X}{g:02X}{b:02X})")
    print(f"C    RGB({R}, {G}, {B})")


def load_jasc_pal(path: Path) -> list[tuple[int, int, int]]:
    lines = path.read_text(encoding="utf-8", errors="replace").splitlines()
    if len(lines) < 3 or lines[0].strip() != "JASC-PAL":
        raise ValueError(f"{path}: not a JASC-PAL file")
    count = int(lines[2].strip())
    colors: list[tuple[int, int, int]] = []
    for line in lines[3 : 3 + count]:
        parts = line.split()
        if len(parts) < 3:
            continue
        colors.append((int(parts[0]), int(parts[1]), int(parts[2])))
    return colors


def load_gbapal(path: Path) -> list[tuple[int, int, int]]:
    data = path.read_bytes()
    if len(data) % 2:
        raise ValueError(f"{path}: odd gbapal size {len(data)}")
    colors: list[tuple[int, int, int]] = []
    for i in range(0, len(data), 2):
        val = struct.unpack_from("<H", data, i)[0] & 0x7FFF
        colors.append(gba_to_rgb8(val))
    return colors


def load_palette(path: Path) -> list[tuple[int, int, int]]:
    suffix = path.suffix.lower()
    if suffix == ".pal":
        return load_jasc_pal(path)
    if suffix == ".gbapal":
        return load_gbapal(path)
    # Peek: JASC text vs binary
    head = path.read_bytes()[:16]
    if head.startswith(b"JASC-PAL"):
        return load_jasc_pal(path)
    return load_gbapal(path)


def dump_palette(path: Path, colors: list[tuple[int, int, int]]) -> None:
    print(f"# {path}  ({len(colors)} colors)")
    print(f"{'idx':>3}  {'hex':8}  {'rgb8':16}  {'gba':6}  rgb5")
    for i, (r, g, b) in enumerate(colors):
        val, R, G, B = rgb8_to_gba(r, g, b)
        # Prefer true GBA bits when source was gbapal-quantized already
        print(f"{i:3d}  #{r:02X}{g:02X}{b:02X}  ({r:3d},{g:3d},{b:3d})  0x{val:04X}  ({R},{G},{B})")


def match_palette(
    colors: list[tuple[int, int, int]],
    target: tuple[int, int, int],
    top: int,
) -> None:
    scored = sorted(
        ((color_distance(c, target), i, c) for i, c in enumerate(colors)),
        key=lambda t: (t[0], t[1]),
    )
    tr, tg, tb = target
    tval, _, _, _ = rgb8_to_gba(tr, tg, tb)
    print(f"target #{tr:02X}{tg:02X}{tb:02X}  RGB8({tr},{tg},{tb})  GBA 0x{tval:04X}")
    print(f"{'rank':>4}  {'idx':>3}  {'hex':8}  {'dist2':>6}  note")
    for rank, (dist, i, (r, g, b)) in enumerate(scored[:top], start=1):
        note = ""
        if dist == 0:
            note = "exact RGB8"
        else:
            # Exact after GBA quantization?
            tv, *_ = rgb8_to_gba(tr, tg, tb)
            cv, *_ = rgb8_to_gba(r, g, b)
            if tv == cv:
                note = "same after RGB8->GBA"
        print(f"{rank:4d}  {i:3d}  #{r:02X}{g:02X}{b:02X}  {dist:6d}  {note}")

    free = [i for i, c in enumerate(colors) if c == (0, 0, 0) or c == (255, 0, 255)]
    if free:
        print(f"likely-free / magenta indices: {', '.join(map(str, free))}")
    else:
        print("no obvious free slots (no pure black/magenta); pick unused indices carefully")


def print_c_snippet(r: int, g: int, b: int, index: int, bg_pal: int | None) -> None:
    val, R, G, B = rgb8_to_gba(r, g, b)
    print("// Inject one color into a BG 4bpp palette slot")
    print(f"static const u16 sColor = RGB8({r}, {g}, {b}); // #{r:02X}{g:02X}{b:02X} -> 0x{val:04X}")
    if bg_pal is None:
        print(f"LoadPalette(&sColor, /*plttOffset*/ + {index}, PLTT_SIZEOF(1));")
        print(f"// or: LoadPalette(&sColor, BG_PLTT_ID(N) + {index}, PLTT_SIZEOF(1));")
    else:
        print(f"LoadPalette(&sColor, BG_PLTT_ID({bg_pal}) + {index}, PLTT_SIZEOF(1));")
    print(f"// then paint with raw index {index} (never PIXEL_FILL({index}) for multi-color plots)")
    print(f"// RGB5 channels: RGB({R}, {G}, {B})")


def main() -> int:
    p = argparse.ArgumentParser(
        description=__doc__,
        formatter_class=argparse.RawDescriptionHelpFormatter,
    )
    p.add_argument("values", nargs="*", help="hex (#007BC5) or R G B (0-255)")
    p.add_argument("--from-gba", metavar="COLOR", help="15-bit value (e.g. 0x61E0) → RGB8 + hex")
    p.add_argument("--pal", metavar="PATH", help="dump JASC .pal or binary .gbapal")
    p.add_argument(
        "--match",
        nargs="+",
        metavar="COLOR",
        help="with --pal: find closest indices for hex or R G B",
    )
    p.add_argument("--top", type=int, default=5, help="how many --match hits to show (default 5)")
    p.add_argument(
        "--c-snippet",
        nargs="+",
        metavar="COLOR",
        help="print LoadPalette snippet for hex or R G B",
    )
    p.add_argument("--index", type=int, default=1, help="palette index for --c-snippet (default 1)")
    p.add_argument("--bg-pal", type=int, help="BG palette bank N for --c-snippet BG_PLTT_ID(N)")
    args = p.parse_args()

    if args.from_gba:
        raw = args.from_gba.strip().lower().removeprefix("0x")
        val = int(raw, 16) & 0x7FFF
        r, g, b = gba_to_rgb8(val)
        print(f"GBA  0x{val:04X}  RGB({val & 31}, {(val >> 5) & 31}, {(val >> 10) & 31})")
        print(f"RGB8 {r}, {g}, {b}")
        print(f"HEX  #{r:02X}{g:02X}{b:02X}")
        print(f"C    RGB8({r}, {g}, {b})  /  RGB_HEX(0x{r:02X}{g:02X}{b:02X})")
        return 0

    if args.pal:
        path = Path(args.pal)
        try:
            colors = load_palette(path)
        except (OSError, ValueError) as e:
            print(e, file=sys.stderr)
            return 1
        if args.match:
            try:
                target = parse_color_args(args.match)
            except ValueError as e:
                print(e, file=sys.stderr)
                return 1
            match_palette(colors, target, max(1, args.top))
        else:
            dump_palette(path, colors)
        return 0

    if args.c_snippet:
        try:
            r, g, b = parse_color_args(args.c_snippet)
        except ValueError as e:
            print(e, file=sys.stderr)
            return 1
        for c in (r, g, b):
            if not 0 <= c <= 255:
                print("channels must be 0–255", file=sys.stderr)
                return 1
        print_c_snippet(r, g, b, args.index, args.bg_pal)
        return 0

    if not args.values:
        p.print_help()
        return 1

    try:
        r, g, b = parse_color_args(args.values)
    except ValueError as e:
        print(e, file=sys.stderr)
        return 1

    for c in (r, g, b):
        if not 0 <= c <= 255:
            print("channels must be 0–255", file=sys.stderr)
            return 1

    format_color_block(r, g, b)
    return 0


if __name__ == "__main__":
    sys.exit(main())
