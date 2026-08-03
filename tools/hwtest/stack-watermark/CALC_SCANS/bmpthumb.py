#!/usr/bin/env python3
"""Scratch: a 1-bit BMP as a coarse text thumbnail, so a whole screen fits in one look."""
import sys

path = sys.argv[1]
step = int(sys.argv[2]) if len(sys.argv) > 2 else 4
data = open(path, "rb").read()
offset = int.from_bytes(data[10:14], "little")
width = int.from_bytes(data[18:22], "little")
height = int.from_bytes(data[22:26], "little", signed=True)
bpp = int.from_bytes(data[28:30], "little")
stride = ((width * bpp + 31) // 32) * 4
rows = abs(height)


def bit(x, y):
    row = rows - 1 - y if height > 0 else y
    byte = data[offset + row * stride + (x >> 3)]
    return (byte >> (7 - (x & 7))) & 1


for y in range(0, rows, step):
    line = []
    for x in range(0, width, step):
        dark = 0
        for dy in range(step):
            for dx in range(step):
                if y + dy < rows and x + dx < width and bit(x + dx, y + dy):
                    dark += 1
        line.append("#" if dark > step else (":" if dark else " "))
    print("".join(line))
