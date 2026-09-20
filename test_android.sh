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
# 1. Native C Invariant Tests
# -----------------------------------------------------------------------------
echo "[1/2] Running Android Native C Invariant Tests..."
echo "-----------------------------------------------------------------"
if make -C tests/android clean && make -C tests/android; then
    echo ">> Native C Invariant Tests: PASSED"
else
    echo ">> Native C Invariant Tests: FAILED"
    FAILED_SUITES=$((FAILED_SUITES + 1))
fi
echo ""

# -----------------------------------------------------------------------------
# 2. Android Kotlin Unit Tests
# -----------------------------------------------------------------------------
echo "[2/2] Running Android Kotlin Unit Tests..."
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
