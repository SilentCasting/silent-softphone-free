# SPDX-FileCopyrightText: 2026 Ihor Martseniuk and Silent Softphone Free contributors
# SPDX-License-Identifier: GPL-3.0-or-later

"""Generates the Silent Softphone Free app icon in every format the build needs.

    python tools/make_icons.py

Writes to assets/icon/:
    app.svg          vector source (for design tools / future edits)
    app.ico          Windows executable icon, 16-256 px
    app.icns         macOS bundle icon, 16-1024 px
    app.png          256 px window icon embedded as a Qt resource

The glyph is Material Design "phone_in_talk" (Apache 2.0). Requires Pillow.
"""

import re
from pathlib import Path

from PIL import Image, ImageDraw

OUT_DIR = Path(__file__).resolve().parent.parent / "assets" / "icon"

GRADIENT_TOP = (91, 155, 255)
GRADIENT_BOTTOM = (29, 78, 216)
GLYPH_COLOR = (255, 255, 255)

# 24x24 viewBox, content spans 3..21 on both axes.
GLYPH_PATHS = [
    "M6.62 10.79c1.44 2.83 3.76 5.14 6.59 6.59l2.2-2.2c.27-.27.67-.36 1.02-.24 1.12.37 2.33.57 3.57.57"
    ".55 0 1 .45 1 1V20c0 .55-.45 1-1 1-9.39 0-17-7.61-17-17 0-.55.45-1 1-1h3.5c.55 0 1 .45 1 1 0 1.25"
    ".2 2.45.57 3.57.11.35.03.74-.25 1.02l-2.2 2.2z",
    "M19 12h2c0-4.97-4.03-9-9-9v2c3.87 0 7 3.13 7 7z",
    "M15 12h2c0-2.76-2.24-5-5-5v2c1.66 0 3 1.34 3 3z",
]
GLYPH_CENTER = 12.0
GLYPH_EXTENT = 18.0


class Variant:
    """Icon layout: macOS icons sit inside a padded squircle, Windows icons fill the tile."""

    def __init__(self, padding, radius, glyph_fraction):
        self.padding = padding            # fraction of the canvas
        self.radius = radius              # fraction of the body size
        self.glyph_fraction = glyph_fraction


MAC = Variant(padding=100 / 1024, radius=0.225, glyph_fraction=0.54)
WINDOWS = Variant(padding=0.02, radius=0.22, glyph_fraction=0.58)

_TOKEN = re.compile(r"[MmLlHhVvCcSsZz]|-?(?:\d+\.\d*|\.\d+|\d+)(?:[eE][-+]?\d+)?")


def flatten_path(d, steps=24):
    """Turns an SVG path (M/L/H/V/C/S/Z, absolute and relative) into polygons."""
    tokens = _TOKEN.findall(d)
    pos = 0
    polygons, current = [], []
    x = y = start_x = start_y = 0.0
    command, last_ctrl = None, None

    def take(n):
        nonlocal pos
        values = [float(t) for t in tokens[pos:pos + n]]
        pos += n
        return values

    def cubic(p0, p1, p2, p3):
        for i in range(1, steps + 1):
            t = i / steps
            u = 1 - t
            current.append((
                u * u * u * p0[0] + 3 * u * u * t * p1[0] + 3 * u * t * t * p2[0] + t * t * t * p3[0],
                u * u * u * p0[1] + 3 * u * u * t * p1[1] + 3 * u * t * t * p2[1] + t * t * t * p3[1],
            ))

    while pos < len(tokens):
        if tokens[pos].isalpha():
            command = tokens[pos]
            pos += 1
            if command in "Zz":
                if current:
                    polygons.append(current)
                current = []
                x, y = start_x, start_y
                last_ctrl = None
                continue

        relative = command.islower()
        dx, dy = (x, y) if relative else (0.0, 0.0)
        kind = command.upper()

        if kind == "M":
            nx, ny = take(2)
            if current:
                polygons.append(current)
            x, y = nx + dx, ny + dy
            start_x, start_y = x, y
            current = [(x, y)]
            command = "l" if relative else "L"  # implicit lineto after moveto
            last_ctrl = None
        elif kind == "L":
            nx, ny = take(2)
            x, y = nx + dx, ny + dy
            current.append((x, y))
            last_ctrl = None
        elif kind == "H":
            (nx,) = take(1)
            x = nx + dx
            current.append((x, y))
            last_ctrl = None
        elif kind == "V":
            (ny,) = take(1)
            y = ny + dy
            current.append((x, y))
            last_ctrl = None
        elif kind == "C":
            x1, y1, x2, y2, ex, ey = take(6)
            p1, p2, p3 = (x1 + dx, y1 + dy), (x2 + dx, y2 + dy), (ex + dx, ey + dy)
            cubic((x, y), p1, p2, p3)
            last_ctrl, (x, y) = p2, p3
        elif kind == "S":
            x2, y2, ex, ey = take(4)
            p1 = (2 * x - last_ctrl[0], 2 * y - last_ctrl[1]) if last_ctrl else (x, y)
            p2, p3 = (x2 + dx, y2 + dy), (ex + dx, ey + dy)
            cubic((x, y), p1, p2, p3)
            last_ctrl, (x, y) = p2, p3
        else:
            raise ValueError(f"unsupported path command {command!r}")

    if current:
        polygons.append(current)
    return polygons


def glyph_transform(variant, canvas):
    body = canvas * (1 - 2 * variant.padding)
    scale = body * variant.glyph_fraction / GLYPH_EXTENT
    center = canvas / 2
    return scale, center - GLYPH_CENTER * scale


def render(variant, size, supersample=4):
    canvas = size * supersample
    pad = round(canvas * variant.padding)
    body = canvas - 2 * pad

    gradient_mask = Image.linear_gradient("L").resize((body, body))
    body_fill = Image.composite(Image.new("RGB", (body, body), GRADIENT_BOTTOM),
                                Image.new("RGB", (body, body), GRADIENT_TOP), gradient_mask)
    shape_mask = Image.new("L", (body, body), 0)
    ImageDraw.Draw(shape_mask).rounded_rectangle((0, 0, body - 1, body - 1),
                                                 radius=round(body * variant.radius), fill=255)

    image = Image.new("RGBA", (canvas, canvas), (0, 0, 0, 0))
    image.paste(body_fill, (pad, pad), shape_mask)

    scale, offset = glyph_transform(variant, canvas)
    draw = ImageDraw.Draw(image)
    for path in GLYPH_PATHS:
        for polygon in flatten_path(path):
            draw.polygon([(px * scale + offset, py * scale + offset) for px, py in polygon],
                         fill=GLYPH_COLOR + (255,))

    return image.resize((size, size), Image.LANCZOS)


def write_svg(path, variant, size=1024):
    pad = size * variant.padding
    body = size - 2 * pad
    scale, offset = glyph_transform(variant, size)
    top = "#%02x%02x%02x" % GRADIENT_TOP
    bottom = "#%02x%02x%02x" % GRADIENT_BOTTOM
    glyph = "\n    ".join(f'<path d="{d}"/>' for d in GLYPH_PATHS)
    path.write_text(f"""<svg xmlns="http://www.w3.org/2000/svg" width="{size}" height="{size}" viewBox="0 0 {size} {size}">
  <!-- Generated by tools/make_icons.py; edit the script, not this file. -->
  <defs>
    <linearGradient id="bg" x1="0" y1="0" x2="0" y2="1">
      <stop offset="0" stop-color="{top}"/>
      <stop offset="1" stop-color="{bottom}"/>
    </linearGradient>
  </defs>
  <rect x="{pad:g}" y="{pad:g}" width="{body:g}" height="{body:g}" rx="{body * variant.radius:g}" fill="url(#bg)"/>
  <g fill="#ffffff" transform="translate({offset:g} {offset:g}) scale({scale:g})">
    {glyph}
  </g>
</svg>
""", encoding="utf-8")


def main():
    OUT_DIR.mkdir(parents=True, exist_ok=True)

    write_svg(OUT_DIR / "app.svg", MAC)

    windows = render(WINDOWS, 256)
    windows.save(OUT_DIR / "app.ico",
                 sizes=[(s, s) for s in (16, 20, 24, 32, 40, 48, 64, 128, 256)])
    windows.save(OUT_DIR / "app.png")

    mac = render(MAC, 1024)
    mac.save(OUT_DIR / "app.icns",
             append_images=[render(MAC, s) for s in (16, 32, 64, 128, 256, 512)])

    for name in ("app.svg", "app.ico", "app.icns", "app.png"):
        print(f"{name:18} {(OUT_DIR / name).stat().st_size:>8} bytes")


if __name__ == "__main__":
    main()
