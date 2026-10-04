Worked example: an A/B of the f and g shift glyph on two DM42 images.

This is how commit 7be1f2250, "f and g shift glyphs are drawn by one routine in screen.c", was checked on the DM42 firmware. The commit changes no behaviour, so
the test is that the image without it and the image with it push the same pixels to the panel, refresh for refresh, for every key sequence that draws or
clears the shift glyph.

The two images:
  Build package 4 on the commit before the change and copy C47.pgm, C47_qspi.bin and C47.elf from build.dmcp.p4/src/c47-dmcp/ into a folder named pre.
  Build it again on the change and copy the same three files into a folder named post. The build command is  make PKG=4 dmcp_pkg4 .

Running it, outside any sandbox:
  python3 tools/pgemu/example-fkey-ab/run_ab.py pre/C47.pgm post/C47.pgm
  The outputs go to tools/pgemu/out-fkey-ab, which .gitignore leaves out, or to the folder given as a third argument.

What it runs, nine sequences on each image, with  --ms-per-tick 1  and a fixed  --clock  so the same script gives the same frames twice:
  f, g, f three times, f then f, STO then f, STO then g, 123 then f, f then EXIT, and 1 then f SIN.
  f then EXIT is OFF on the C47, so that run stops at STAT_PGM_END before its snap and is compared on its frames alone.

What it checks:
  The prog_info byte counts of the two images, which must differ, or the two folders contain the same build.
  The frame digest of every refresh, pre against post, and the final capture of every sequence that ends on one.
  A control: the f capture must differ from the g capture, or no glyph reached the panel and the equal captures prove nothing.
  The top-left corner of both f captures, printed as text, so the glyph is seen without a picture viewer.

The reading on 2026-10-01, pre 714328 bytes and post 713824 bytes:
  f      frames same (77)  capture same
  g      frames same (80)  capture same
  off    frames same (86)  capture same
  ff     frames same (80)  capture same
  tamf   frames same (80)  capture same
  tamg   frames same (83)  capture same
  typef  frames same (86)  capture same
  fexit  frames same (85)  capture none in either run
  fsin   frames same (84)  capture same
  refreshes compared: 741
  control, the f and g captures differ: yes

Limits:
  The factory status bar never brings up the small glyph, so this covers the large glyph only. The small one was covered in the simulator, with t47 and
  --snapkeepshift over 1024 cases that set SBdate, SBtime, SBwoy and SBshfR; see res/SCRIPTS/cli_automation_examples.txt for that option.
  This script has not been run against an image with a deliberate fault in the glyph code. The simulator cases were: with the small-glyph test inverted, all
  1024 logs differed.
