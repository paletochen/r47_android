#!/bin/bash
set -e

# =============================================================================
# R47 Android Automated Verification Suite & Presubmit Harness
# =============================================================================
# Runs all native C invariant tests and Android Kotlin unit tests.
# =============================================================================

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$PROJECT_ROOT"

# Ensure JDK environment matches build_android.sh
if [ -n "$JAVA_HOME" ] && [ ! -d "$JAVA_HOME" ]; then
    unset JAVA_HOME
fi

if [ -z "$JAVA_HOME" ]; then
    for jvm in /usr/lib/jvm/java-17-openjdk-amd64 /usr/lib/jvm/java-21-openjdk-amd64 /usr/lib/jvm/default-java; do
        if [ -d "$jvm" ]; then
            export JAVA_HOME="$jvm"
            break
        fi
    done
fi

echo "================================================================="
echo "  R47 Android Test Suite & Presubmit Verification"
echo "================================================================="
echo "Project Root: $PROJECT_ROOT"
if [ -n "$JAVA_HOME" ]; then
    echo "JAVA_HOME:    $JAVA_HOME"
fi
echo "================================================================="
echo ""

# Track failures
FAILED_SUITES=0

# -----------------------------------------------------------------------------
# 1. Repository & Protected Files Integrity
# -----------------------------------------------------------------------------
echo "[1/3] Checking Repository & Protected Files Integrity..."
echo "-----------------------------------------------------------------"
REPO_INTEGRITY_OK=1
if [ ! -f "README.md" ] || ! head -n 1 README.md | grep -q "^# R47 Android Port" || [ "$(wc -l < README.md)" -lt 200 ]; then
    echo "  [FAIL] README.md is missing '# R47 Android Port' header or was overwritten by upstream!"
    REPO_INTEGRITY_OK=0
fi
if [ -f "README.md" ] && (grep -q "^# C47$" README.md || grep -Eq "^(<<<<<<<|=======|>>>>>>>)" README.md); then
    echo "  [FAIL] README.md contains upstream '# C47' header or unresolved Git conflict markers!"
    REPO_INTEGRITY_OK=0
fi
for req in build_android.sh test_android.sh sync_public.sh docs/ANDROID_TESTS.md docs/screens/ss_main_screen.png docs/skins/r47_replica.jpg; do
    if [ ! -f "$req" ]; then
        echo "  [FAIL] Required R47 file missing: $req"
        REPO_INTEGRITY_OK=0
    fi
done
CONFLICT_HITS=$(grep -rnE "^(<<<<<<< |=======$|>>>>>>> )" README.md RELEASE_NOTES.md Makefile .gitignore *.sh src/c47/ android/ tests/ docs/ANDROID_TESTS.md .github/ 2>/dev/null || true)
if [ -n "$CONFLICT_HITS" ]; then
    echo "  [FAIL] Unresolved Git conflict markers detected:"
    echo "$CONFLICT_HITS"
    REPO_INTEGRITY_OK=0
fi
ORIG_REJ_HITS=$(find src/c47 android -type f \( -name "*.orig" -o -name "*.rej" \) 2>/dev/null || true)
if [ -n "$ORIG_REJ_HITS" ]; then
    echo "  [FAIL] Stray patch .orig/.rej files detected:"
    echo "$ORIG_REJ_HITS"
    REPO_INTEGRITY_OK=0
fi
if [ $REPO_INTEGRITY_OK -eq 1 ]; then
    echo ">> Repository & Protected Files Integrity: PASSED"
else
    echo ">> Repository & Protected Files Integrity: FAILED"
    FAILED_SUITES=$((FAILED_SUITES + 1))
fi
echo ""

# -----------------------------------------------------------------------------
# 2. Native C Invariant Tests
# -----------------------------------------------------------------------------
echo "[2/3] Running Android Native C Invariant Tests..."
echo "-----------------------------------------------------------------"
if make -C tests/android clean && make -C tests/android; then
    echo ">> Native C Invariant Tests: PASSED"
else
    echo ">> Native C Invariant Tests: FAILED"
    FAILED_SUITES=$((FAILED_SUITES + 1))
fi
echo ""

# -----------------------------------------------------------------------------
# 3. Android Kotlin Unit Tests
# -----------------------------------------------------------------------------
echo "[3/3] Running Android Kotlin Unit Tests..."
echo "-----------------------------------------------------------------"
if (cd android && ./gradlew testDebugUnitTest --quiet); then
    echo ">> Kotlin Unit Tests: PASSED"
else
    echo ">> Kotlin Unit Tests: FAILED"
    FAILED_SUITES=$((FAILED_SUITES + 1))
fi
echo ""

# -----------------------------------------------------------------------------
# Summary Report
# -----------------------------------------------------------------------------
echo "================================================================="
if [ $FAILED_SUITES -eq 0 ]; then
    echo "  ALL TEST SUITES PASSED (0 Failures)"
    echo "================================================================="
    exit 0
else
    echo "  VERIFICATION FAILED ($FAILED_SUITES Suite(s) Failed)"
    echo "================================================================="
    exit 1
fi
