#!/bin/bash
# R47 Transparent Firmware Builder (v2 - Shadow Identity)
# This script ensures both filenames AND internal OS metadata identify 
# correctly with the SwissMicros core hash without modifying core files.
set -e

# 1. Calculate the SwissMicros Core Hash (8 chars)
# Public shell: Prioritize 'upstream/master' (SwissMicros)
# Private vault: Prioritize 'upstream/master' then 'origin/master'
CORE_HASH=$(git rev-parse --short=8 upstream/master 2>/dev/null || \
            git rev-parse --short=8 origin/master 2>/dev/null || \
            echo "unknown")

if [ "$CORE_HASH" == "unknown" ]; then
    echo "⚠️ Warning: Could not find SwissMicros Core hash. Falling back to local HEAD."
    CORE_HASH=$(git rev-parse --short=8 HEAD)
fi

VERSION_OVERRIDE="${CORE_HASH}-mod"

echo "--- 🔍 Detected SwissMicros Core Identity: $VERSION_OVERRIDE ---"

# 2. Setup the Shadow Identity Wrapper
# This intercepts the build system's 'git describe' calls to prevent local 
# private commits from contaminating the internal firmware version.
SHADOW_BIN_DIR="$(pwd)/.shadow_bin"
mkdir -p "$SHADOW_BIN_DIR"
cat <<EOF > "$SHADOW_BIN_DIR/git"
#!/bin/bash
if [[ "\$*" == *"describe"* ]]; then
    # Return the core hash when the build system asks for the version
    echo "$VERSION_OVERRIDE"
else
    # Pass everything else to the real git
    exec /usr/bin/git "\$@"
fi
EOF
chmod +x "$SHADOW_BIN_DIR/git"

# Inject the shadow wrapper into the path for the duration of this script
ORIG_PATH="$PATH"
export PATH="$SHADOW_BIN_DIR:$PATH"

echo "🧹 Cleaning..."
make clean

# 3. Build with Shadow Identity
# We pass VERSION to make for filenames, and the Shadow Git handles the internal vcs.h
echo "📦 Building dist_macos..."
make VERSION="$VERSION_OVERRIDE" BUILD_PC=build.rel build.rel sim simr47
make dist_macos VERSION="$VERSION_OVERRIDE" BUILD_PC=build.rel

echo "📦 Building dist_dmcp..."
make dist_dmcp VERSION="$VERSION_OVERRIDE"

echo "📦 Building dist_dmcp5..."
make dist_dmcp5 VERSION="$VERSION_OVERRIDE"

echo "📦 Building dist_dmcp5r47..."
make dist_dmcp5r47 VERSION="$VERSION_OVERRIDE"

echo "📦 Building dist_dmcpr47..."
make dist_dmcpr47 VERSION="$VERSION_OVERRIDE"

# 4. Cleanup Shadow Wrapper
export PATH="$ORIG_PATH"
rm -rf "$SHADOW_BIN_DIR"

echo "--- ✅ Transparent Build Complete: $VERSION_OVERRIDE ---"
echo "Internal metadata and filenames are now perfectly aligned with SwissMicros Core."
