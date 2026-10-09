#!/bin/sh
# Build tools/pgemu/examples.pdf from examples.md. Run from the c43 repository root.
# Needs pandoc, xelatex and Ghostscript. The body font is TeX Gyre Heros. The page is cut to the
# ink plus a 5 mm border by docs/tools/pdfcrop-uniform.py, the crop the application notes publish with.
# Every intermediate goes in a temporary directory that is removed at the end, so the only file left
# in the tree is examples.pdf.
set -e

here=tools/pgemu
work=$(mktemp -d "${TMPDIR:-/tmp}/pgemu-examples.XXXXXX")
trap 'rm -rf "$work"' EXIT

# The body font is TeX Gyre Heros, loaded by file so no fontconfig entry is needed.
# fvextra breaks the long code lines; hyphenat and \sloppy break a long inline path or URL.
cat > "$work/header.tex" <<'EOF'
\usepackage{fvextra}
\fvset{breaklines=true, breakanywhere=true, fontsize=\small}
\usepackage[htt]{hyphenat}
\setmainfont{texgyreheros}[Extension=.otf, UprightFont=*-regular, BoldFont=*-bold, ItalicFont=*-italic, BoldItalicFont=*-bolditalic]
\sloppy
EOF

pandoc "$here/examples.md" -s -o "$work/examples.tex" -H "$work/header.tex" -V geometry:margin=1.5cm

# A failed run prints the end of its log, which the trap would otherwise remove along with the directory.
xelatex -interaction=nonstopmode -output-directory="$work" "$work/examples.tex" > "$work/xelatex1.log" 2>&1 || { tail -n 40 "$work/xelatex1.log" >&2; exit 1; }
xelatex -interaction=nonstopmode -output-directory="$work" "$work/examples.tex" > "$work/xelatex2.log" 2>&1 || { tail -n 40 "$work/xelatex2.log" >&2; exit 1; }

python3 docs/tools/pdfcrop-uniform.py "$work/examples.pdf" "$here/examples.pdf"
