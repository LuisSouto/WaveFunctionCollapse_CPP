#!/bin/bash

# Script to generate code coverage report
# Usage: ./scripts/generate_coverage.sh

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo -e "${BLUE}Generating code coverage report...${NC}"

# Create build directory if it doesn't exist
if [ ! -d "build" ]; then
    mkdir -p build
fi

# Check if gcov is installed
if ! command -v gcov &> /dev/null; then
    echo -e "${RED}Error: gcov is not installed${NC}"
    echo "On Ubuntu/Debian: sudo apt-get install gcovr"
    echo "On macOS: brew install gcovr"
    exit 1
fi

# Check if gcovr is installed (optional but recommended)
if ! command -v gcovr &> /dev/null; then
    echo -e "${BLUE}Note: gcovr not found. Installing gcovr is recommended for better reports.${NC}"
    echo "Install with: pip install gcovr"
    echo ""
fi

# Configure and build with coverage
echo -e "${BLUE}Building with coverage flags...${NC}"
cd build
cmake -DWFC_ENABLE_COVERAGE=ON ..
make clean
make -j$(nproc)

# Run tests
echo -e "${BLUE}Running tests...${NC}"
./bin/wfc_tests

# Generate coverage report using gcovr
if command -v gcovr &> /dev/null; then
    echo -e "${BLUE}Generating gcovr HTML report...${NC}"
    gcovr --root .. \
        --filter ".*(src|include)/.*" \
        --exclude ".*(_deps|/usr/|/usr/include/|/usr/lib/|/usr/local/|stb_image\..*|stb_image\.h).*" \
        --print-summary --html-details coverage.html
    echo -e "${GREEN}Coverage report generated: build/coverage.html${NC}"
    echo -e "${GREEN}Coverage is limited to project files under src/ and include/, excluding stb_image and external headers.${NC}"
else
    echo -e "${BLUE}Generating basic coverage using gcov...${NC}"
    
    # Create coverage directory
    mkdir -p coverage
    
    # Find and process .gcda files
    find . -name "*.gcda" -exec gcov -p -o {} {} \;
    
    echo -e "${GREEN}Coverage data generated in: build/coverage/${NC}"
    echo -e "${BLUE}Note: raw gcov output may include system headers. Install gcovr to filter coverage to your source files:${NC}"
    echo "  pip install gcovr"
    echo "  gcovr --root .. --filter '.*(src|include)/.*' --exclude '.*(/usr/|_deps/|/usr/include/|/usr/lib/|/usr/local/).*' --print-summary --html-details coverage.html"
fi

cd ..
echo -e "${GREEN}Code coverage generation complete!${NC}"
