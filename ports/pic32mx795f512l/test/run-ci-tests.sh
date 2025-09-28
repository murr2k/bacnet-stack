#!/bin/bash
# CI Test Script for PIC32MX795F512L Port ABI Compatibility
# This script can be run locally or in CI/CD pipelines

set -e  # Exit on error

echo "================================================"
echo " PIC32MX795F512L Port - CI Test Suite"
echo "================================================"
echo ""

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Check if we're in the correct directory
if [ ! -f "test_abi_compatibility.c" ]; then
    echo -e "${RED}Error: Must run from ports/pic32mx795f512l/test directory${NC}"
    exit 1
fi

# Function to print status
print_status() {
    if [ $1 -eq 0 ]; then
        echo -e "${GREEN}✅ $2${NC}"
    else
        echo -e "${RED}❌ $2${NC}"
        return 1
    fi
}

# Clean previous builds
echo "1. Cleaning previous builds..."
make -f Makefile.host clean > /dev/null 2>&1
print_status $? "Clean completed"

# Create build directory
echo "2. Creating build directories..."
make -f Makefile.host host_build > /dev/null 2>&1
print_status $? "Directories created"

# Build with host compiler
echo "3. Building port with host compiler..."
make -f Makefile.host all > build.log 2>&1
if [ $? -eq 0 ]; then
    echo -e "${GREEN}✅ Build successful${NC}"
else
    echo -e "${RED}❌ Build failed${NC}"
    echo "Build errors:"
    tail -20 build.log
    exit 1
fi

# Run ABI compatibility test
echo "4. Running ABI compatibility test..."
./test_abi_host > test.log 2>&1
TEST_RESULT=$?

if [ $TEST_RESULT -eq 0 ]; then
    echo -e "${GREEN}✅ ABI test completed${NC}"

    # Check results
    if grep -q "BINARY COMPATIBLE" ABI_COMPATIBILITY_REPORT.md; then
        echo ""
        echo "================================================"
        echo -e "${GREEN} ✅ ALL TESTS PASSED ${NC}"
        echo "================================================"
        echo ""

        # Show summary
        grep -A2 "Executive Summary" ABI_COMPATIBILITY_REPORT.md
        echo ""
        grep "Compatibility Rate:" ABI_COMPATIBILITY_REPORT.md
        echo ""
        echo -e "${GREEN}The port maintains full binary compatibility!${NC}"
        exit 0
    else
        echo -e "${RED}❌ Compatibility issues detected${NC}"
        cat ABI_COMPATIBILITY_REPORT.md
        exit 1
    fi
else
    echo -e "${RED}❌ ABI test failed${NC}"
    cat test.log
    exit 1
fi