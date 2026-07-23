#!/usr/bin/env python3
"""Estimate window / StatRadar fit before rebuild cycles.

Mirrors the sizing rules in src/stat_radar.c (PlaceInWindow + label reserve).

Examples:
  python3 tools/ui_layout.py radar --tiles 15 13 --pad 10 --labels --scale 64
  python3 tools/ui_layout.py radar --preset medium
  python3 tools/ui_layout.py radar --preset preview --tiles 26 3
  python3 tools/ui_layout.py window --tiles 26 3
"""

from __future__ import annotations

import argparse
import math
import sys

# Keep in sync with StatRadar_PlaceInWindow / ApplySizePreset.
LABEL_RESERVE_PX = 12
MIN_RADIUS = 8
MIN_SCALED_RADIUS = 4
ANGLE_UP = 192  # tip-up; Cos/Sin units where 256 = full circle

PRESETS = {
    # MAKEOVER / Stat Editor footer mini chart
    "preview": {
        "labels": False,
        "values": False,
        "radius": 11,
        "scale": 100,
        "pad": 2,
        "nudge_x": 0,
        "midline_bias": None,
        "note": "unlabeled mini; place via FooterStrip mid (MAKEOVER), not full-window nudge",
    },
    # unlabeled auto-fit, slightly shrunk
    "small": {
        "labels": False,
        "values": False,
        "radius": 0,
        "scale": 80,
        "pad": 2,
        "nudge_x": 0,
        "midline_bias": None,
        "note": "unlabeled auto-fit @ 80% scale",
    },
    # summary Moves memo EV radar
    "medium": {
        "labels": True,
        "values": True,
        "radius": 0,
        "scale": 64,
        "pad": 10,
        "nudge_x": -2,
        "midline_bias": 1,
        "note": "labeled Moves pane chart (summary)",
    },
    # full labeled fit
    "large": {
        "labels": True,
        "values": True,
        "radius": 0,
        "scale": 100,
        "pad": 8,
        "nudge_x": 0,
        "midline_bias": 0,
        "note": "labeled full auto-fit @ 100% scale",
    },
}

# Known call-site windows (tiles W×H)
KNOWN_WINDOWS = {
    "summary-moves-memo": (15, 13),  # POKESUM_WIN_MOVES_4 / TRAINER_MEMO
    "stat-editor-footer": (26, 3),   # WIN_FOOTER
}


def gba_sin(angle: int, amp: int) -> int:
    """Approximate engine Sin: angle 0–255, returns roughly amp*sin."""
    # Good enough for tip position estimates (not pixel-perfect).
    rad = (angle / 256.0) * 2.0 * math.pi
    return int(round(math.sin(rad) * amp))


def gba_cos(angle: int, amp: int) -> int:
    rad = (angle / 256.0) * 2.0 * math.pi
    return int(round(math.cos(rad) * amp))


def place_radius(win_w: int, win_h: int, pad: int, labels: bool, scale: int, fixed_r: int) -> dict:
    max_r = min(win_w, win_h) // 2 - pad
    if labels:
        max_r -= LABEL_RESERVE_PX
    if max_r < MIN_RADIUS:
        max_r = MIN_RADIUS

    radius = fixed_r if fixed_r else max_r
    if scale not in (0, 100):
        radius = max(MIN_SCALED_RADIUS, (radius * scale) // 100)

    cx, cy = win_w // 2, win_h // 2
    tip_up_y = cy + gba_sin(ANGLE_UP, radius)  # tip-up ≈ cy - radius in screen Y
    # Engine: Sin(192) is -amp (up). Our math.sin(192/256*2pi) = sin(3pi/2) = -1 → cy - radius.
    midline_nudge = radius  # StatRadar_NudgeTipUpToMidline base

    # Bounding box of cage tips (6 axes tip-up)
    tips = []
    for i in range(6):
        ang = (ANGLE_UP + (i * 256) // 6) & 0xFF
        tips.append((cx + gba_cos(ang, radius), cy + gba_sin(ang, radius)))
    xs = [t[0] for t in tips]
    ys = [t[1] for t in tips]

    return {
        "win_w": win_w,
        "win_h": win_h,
        "cx": cx,
        "cy": cy,
        "max_r_before_scale": max_r,
        "radius": radius,
        "cage_bbox": (min(xs), min(ys), max(xs), max(ys)),
        "tip_up": (cx, tip_up_y),
        "midline_nudge_y": midline_nudge,
        "fits_cage": min(xs) >= 0 and max(xs) < win_w and min(ys) >= 0 and max(ys) < win_h,
        "label_headroom_y": min(ys),  # after place, before midline nudge
        "tips": tips,
    }


def print_window(tiles_w: int, tiles_h: int) -> None:
    print(f"window tiles {tiles_w}x{tiles_h}  ->  {tiles_w * 8}x{tiles_h * 8} px")
    print(f"center would be ({tiles_w * 4}, {tiles_h * 4})")


def print_radar(args: argparse.Namespace) -> int:
    preset = PRESETS.get(args.preset) if args.preset else None
    if args.preset and preset is None:
        print(f"unknown preset {args.preset!r}; choose {', '.join(PRESETS)}", file=sys.stderr)
        return 1

    if args.window:
        if args.window not in KNOWN_WINDOWS:
            print(f"unknown window {args.window!r}; choose {', '.join(KNOWN_WINDOWS)}", file=sys.stderr)
            return 1
        tw, th = KNOWN_WINDOWS[args.window]
    elif args.tiles:
        tw, th = args.tiles
    elif preset and args.preset == "preview":
        tw, th = KNOWN_WINDOWS["stat-editor-footer"]
    elif preset and args.preset == "medium":
        tw, th = KNOWN_WINDOWS["summary-moves-memo"]
    else:
        tw, th = (15, 13)

    pad = args.pad if args.pad is not None else (preset["pad"] if preset else 4)
    labels = args.labels if args.labels is not None else (preset["labels"] if preset else False)
    # values imply label reserve in PlaceInWindow too
    if preset and preset.get("values"):
        labels = True
    scale = args.scale if args.scale is not None else (preset["scale"] if preset else 100)
    fixed_r = args.radius if args.radius is not None else (preset["radius"] if preset else 0)

    win_w, win_h = tw * 8, th * 8
    print_window(tw, th)
    if preset:
        print(f"preset {args.preset}: {preset['note']}")

    info = place_radius(win_w, win_h, pad, labels, scale, fixed_r)
    print()
    print(f"pad {pad}  labels/values reserve {'yes (-%dpx)' % LABEL_RESERVE_PX if labels else 'no'}")
    print(f"maxR before scale: {info['max_r_before_scale']}")
    print(f"radius after scale {scale}%: {info['radius']}")
    print(f"center (cx,cy): ({info['cx']}, {info['cy']})")
    x0, y0, x1, y1 = info["cage_bbox"]
    print(f"cage tip bbox: ({x0},{y0})-({x1},{y1})  fits_window={info['fits_cage']}")
    print(f"tip-up at place: {info['tip_up']}")
    print(f"NudgeTipUpToMidline base nudgeY: {info['midline_nudge_y']}")

    nudge_x = args.nudge_x if args.nudge_x is not None else (preset["nudge_x"] if preset else 0)
    bias = args.midline_bias
    if bias is None and preset is not None:
        bias = preset["midline_bias"]
    if bias is not None:
        print(f"suggested nudgeX={nudge_x}, nudgeY={info['midline_nudge_y'] + bias} (midline + bias {bias})")
        cy2 = info["cy"] + info["midline_nudge_y"] + bias
        tip2 = cy2 + gba_sin(ANGLE_UP, info["radius"])
        print(f"after midline nudge: center y={cy2}, tip-up y={tip2} (window mid y={info['cy']})")
    elif nudge_x:
        print(f"suggested nudgeX={nudge_x} (no midline nudge for this preset)")

    print()
    print("C sketch:")
    size_name = {
        "preview": "STAT_RADAR_SIZE_PREVIEW",
        "small": "STAT_RADAR_SIZE_SMALL",
        "medium": "STAT_RADAR_SIZE_MEDIUM",
        "large": "STAT_RADAR_SIZE_LARGE",
    }.get(args.preset or "", None)
    if size_name:
        print(f"  StatRadar_SetDefaults(&cfg);")
        print(f"  StatRadar_ApplySizePreset(&cfg, {size_name});")
        print(f"  // set values / colors / labels ...")
        print(f"  StatRadar_PlaceInWindow(&cfg, windowId, StatRadar_PadForSize({size_name}));")
        if bias is not None:
            print(f"  StatRadar_NudgeTipUpToMidline(&cfg, {bias});")
        if nudge_x and size_name != "STAT_RADAR_SIZE_MEDIUM":
            # MEDIUM preset already sets nudgeX; PREVIEW uses FooterStrip mid.
            print(f"  cfg.nudgeX = {nudge_x};")
        print(f"  StatRadar_Draw(&cfg);")
    else:
        print(f"  // radius~{info['radius']}, pad={pad}, scalePercent={scale}")
    return 0


def main() -> int:
    p = argparse.ArgumentParser(
        description=__doc__,
        formatter_class=argparse.RawDescriptionHelpFormatter,
    )
    sub = p.add_subparsers(dest="cmd", required=True)

    w = sub.add_parser("window", help="tiles → pixel size")
    w.add_argument("--tiles", nargs=2, type=int, metavar=("W", "H"), required=True)

    r = sub.add_parser("radar", help="StatRadar fit estimate")
    r.add_argument("--preset", choices=sorted(PRESETS.keys()))
    r.add_argument("--window", choices=sorted(KNOWN_WINDOWS.keys()), help="known call-site window")
    r.add_argument("--tiles", nargs=2, type=int, metavar=("W", "H"), help="window size in tiles")
    r.add_argument("--pad", type=int)
    r.add_argument("--labels", dest="labels", action="store_true", default=None)
    r.add_argument("--no-labels", dest="labels", action="store_false")
    r.add_argument("--scale", type=int)
    r.add_argument("--radius", type=int, help="fixed radius (0 = auto)")
    r.add_argument("--nudge-x", type=int, dest="nudge_x")
    r.add_argument("--midline-bias", type=int, dest="midline_bias")

    args = p.parse_args()
    if args.cmd == "window":
        print_window(args.tiles[0], args.tiles[1])
        return 0
    if args.cmd == "radar":
        return print_radar(args)
    p.print_help()
    return 1


if __name__ == "__main__":
    sys.exit(main())
