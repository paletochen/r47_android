#!/usr/bin/env python3
# Print a band of rows of a 1-bit C47 screen capture as text, to read the softkey row without a viewer.
import struct, sys

path, top, bottom = sys.argv[1], int(sys.argv[2]), int(sys.argv[3])
data = open(path, 'rb').read()
offset, width, height, bpp = struct.unpack_from('<I', data, 10)[0], struct.unpack_from('<i', data, 18)[0], struct.unpack_from('<i', data, 22)[0], struct.unpack_from('<H', data, 28)[0]
rowBytes = ((width * bpp + 31) // 32) * 4
flip = height > 0
height = abs(height)
for y in range(top, bottom):
    row = y if not flip else height - 1 - y
    line = []
    for x in range(width):
        byte = data[offset + row * rowBytes + (x * bpp) // 8]
        bit = (byte >> (7 - (x % 8))) & 1 if bpp == 1 else byte
        line.append('#' if bit else ' ')
    print(''.join(line))
