cmake_minimum_required(VERSION 3.22.1)
project("c47-android")

# Android Build Definitions
add_definitions(
    -DANDROID_BUILD
    -DPC_BUILD          # Reusing PC logic where applicable (sim)
    -DLINUX             # closest platform match
    -DOS64BIT           # Android is 64-bit
    -DCALCMODEL=USER_R47
)

# Global GMP renames
add_definitions(-Dmpz_div_2exp=mpz_tdiv_q_2exp)
add_definitions(-Dmpz_fits_uint_p=mpz_fits_ulong_p)

# Forced include of mocks to ensure they are seen everywhere
add_compile_options("-include" "${CMAKE_CURRENT_SOURCE_DIR}/c47-android/android_mocks.h")

# Include directories
include_directories(
    c47
    c47/core
    c47/hal
    c47/ui
    c47/logicalOps
    c47/mathematics
    c47/programming
    c47/solver
    c47/browsers
    c47/distributions
    c47/c47Extensions
    decNumberICU
    generated
    c47-android
    gmp
)

# Source files - Recursively find all C files in c47 subdirectories
file(GLOB_RECURSE C47_SOURCES "c47/*.c")

file(GLOB DEC_SOURCES 
    decNumberICU/decContext.c
    decNumberICU/decDouble.c
    decNumberICU/decNumber.c
    decNumberICU/decPacked.c
    decNumberICU/decQuad.c
    decNumberICU/decimal128.c
    decNumberICU/decimal32.c
    decNumberICU/decimal64.c
)
file(GLOB_RECURSE GEN_SOURCES "generated/*.c")
file(GLOB_RECURSE ANDROID_SOURCES "c47-android/*.c")
file(GLOB GMP_SOURCES "gmp/*.c")

# Shared library
add_library(c47-android SHARED
    ${C47_SOURCES}
    ${DEC_SOURCES}
    ${GEN_SOURCES}
    ${ANDROID_SOURCES}
    ${GMP_SOURCES}
)

# Link libraries
find_library(log-lib log)
find_library(m-lib m)

target_link_libraries(c47-android
    ${log-lib}
    ${m-lib}
)

# 16KB page alignment
target_link_options(c47-android PRIVATE "-Wl,-z,max-page-size=16384")
