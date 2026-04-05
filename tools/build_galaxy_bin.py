#!/usr/bin/env python3
"""
Emit assets/Galaxy.bin from `TheMilkyWay2` in GalaxyE `Sector.cs`.

The C++ loader expects raw RGB triplets (same as the C# `BinaryReader` loop over
Galaxy.bin). Each grayscale byte from TheMilkyWay2 is expanded to R=G=B.
"""

from __future__ import annotations

import argparse
import re
import sys
from pathlib import Path


def extract_milkyway2_bytes(sector_cs: Path) -> bytes:
    lines = sector_cs.read_text(encoding="utf-8", errors="replace").splitlines()
    # Line numbers in Sector.cs (1-based): TheMilkyWay2 starts at 18, closes before SystemDensity (ends `};` at 2085).
    start_idx = next(i for i, line in enumerate(lines) if "TheMilkyWay2" in line and "byte[]" in line)
    end_idx = start_idx
    while end_idx < len(lines):
        if lines[end_idx].strip() == "};":
            # Closing of TheMilkyWay2 is followed by #endregion for mwa, then SystemDensity.
            if end_idx + 1 < len(lines) and "#endregion" in lines[end_idx + 1]:
                break
        end_idx += 1
    block = "\n".join(lines[start_idx : end_idx + 1])
    hexes = re.findall(r"0x[0-9A-Fa-f]+", block, flags=re.I)
    out = bytearray()
    for h in hexes:
        b = int(h, 16) & 0xFF
        out.extend((b, b, b))
    return bytes(out)


def main() -> int:
    ap = argparse.ArgumentParser(description="Build Galaxy.bin from Sector.cs TheMilkyWay2")
    ap.add_argument(
        "--sector-cs",
        type=Path,
        default=Path(__file__).resolve().parents[1] / "GalaxyE-master" / "GalaxyE-master" / "GalaxyE" / "Galaxy" / "Sector.cs",
    )
    ap.add_argument(
        "--out",
        type=Path,
        default=Path(__file__).resolve().parents[1] / "assets" / "Galaxy.bin",
    )
    args = ap.parse_args()
    if not args.sector_cs.is_file():
        print(f"Sector.cs not found: {args.sector_cs}", file=sys.stderr)
        return 1
    data = extract_milkyway2_bytes(args.sector_cs)
    args.out.parent.mkdir(parents=True, exist_ok=True)
    args.out.write_bytes(data)
    print(f"Wrote {len(data)} bytes ({len(data) // 3} RGB pixels) -> {args.out}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
