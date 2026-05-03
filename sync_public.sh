#!/bin/bash
# R47 Robust Public Sync (Surgical Mode)
set -e
echo "--- 🔄 Starting Robust Public Sync ---"

UPSTREAM_URL="https://gitlab.com/rpncalculators/c43.git"

# 1. Setup & Fetch from SwissMicros (Math Core)
if ! git remote | grep -q "upstream"; then
    echo "--- Adding Upstream (SwissMicros C43) ---"
    git remote add upstream "$UPSTREAM_URL"
fi
git fetch upstream

echo "--- Overlaying Math Core ---"
# checkout upstream core into working tree
git checkout upstream/master -- . 2>/dev/null || true

# 2. SURGICAL RESTORE ANDROID PORT (The Winner)
# We restore only specific files from HEAD (our public shell)
# to avoid 'git checkout dir' deleting files in that directory.
echo "--- Re-applying Android Port Patches ---"

# Root Infrastructure
git checkout HEAD -- .gitignore README.md Makefile build_android.sh sync_public.sh dist.sh 2>/dev/null || true

# Source Patches
git checkout HEAD -- src/c47/programming/input.c src/c47/programming/lblGtoXeq.c src/c47/screen.c 2>/dev/null || true

# Blueprint Preservation
find src -name "meson.build" -exec git checkout HEAD -- {} + 2>/dev/null || true
find dep -name "meson.build" -exec git checkout HEAD -- {} + 2>/dev/null || true

# Android Project Folder (This one we can restore safely as it's not in upstream)
git checkout HEAD -- android/ 2>/dev/null || true

echo "--- ✅ SYNC COMPLETE: Core updated, Patches restored surgically. ---"
