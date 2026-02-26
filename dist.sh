#!/bin/bash
# R47 Transparent Firmware Builder (v3 - Dynamic Filtering)
# This script executes the upstream 'dist' logic but filters out legacy 
# hardware builds (Packages 1, 2, 3) and enforces correct versioning.
set -e

# 1. Calculate the SwissMicros Core Hash (8 chars)
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
# This intercepts the build system's 'git describe' calls.
SHADOW_BIN_DIR="$(pwd)/.shadow_bin"
mkdir -p "$SHADOW_BIN_DIR"
cat <<EOF > "$SHADOW_BIN_DIR/git"
#!/bin/bash
if [[ "\$*" == *"describe"* ]]; then
    echo "$VERSION_OVERRIDE"
else
    exec /usr/bin/git "\$@"
fi
EOF
chmod +x "$SHADOW_BIN_DIR/git"

# Inject shadow wrapper
ORIG_PATH="$PATH"
export PATH="$SHADOW_BIN_DIR:$PATH"

# 3. Dynamic Filtering of the 'dist' script
# We filter out lines 24-31 (Legacy DM42 packages) to prevent size errors.
# We also ensure the VERSION override is passed to every 'make' call.
echo "--- 🛡️ Filtering Legacy Hardware Builds (Packages 1, 2, 3) ---"
FILTERED_DIST=".dist_filtered.sh"

# Create a version of the 'dist' script that:
# - Removes the problematic lines 24-31
# - Appends our VERSION override to all 'make' commands
sed '24,31d' dist | sed "s/make /make VERSION=\"$VERSION_OVERRIDE\" /g" > "$FILTERED_DIST"
chmod +x "$FILTERED_DIST"

# 4. Execute the Filtered Build
echo "🚀 Starting Surgical Build..."
bash "./$FILTERED_DIST"

# 5. Cleanup
export PATH="$ORIG_PATH"
rm -rf "$SHADOW_BIN_DIR"
rm -f "$FILTERED_DIST"

echo "--- ✅ Transparent Filtered Build Complete: $VERSION_OVERRIDE ---"
