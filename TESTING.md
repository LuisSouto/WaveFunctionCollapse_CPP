# Testing Guide for Wave Function Collapse

## Running Tests Locally

### Basic Test Run
```bash
cd build
./bin/wfc_tests
```

### Run with CTest
```bash
cd build
ctest --output-on-failure
```

## Code Coverage

### Generate Coverage Report Locally

**Option 1: Using the provided script**
```bash
chmod +x scripts/generate_coverage.sh
./scripts/generate_coverage.sh
```

**Option 2: Manual coverage generation**
```bash
mkdir -p build && cd build
cmake -DWFC_ENABLE_COVERAGE=ON -DWFC_BUILD_TESTS=ON ..
make clean && make -j$(nproc)
./bin/wfc_tests

# Generate HTML report (requires gcovr)
gcovr --root .. --html-details coverage.html
```

### View Coverage Report
After generating, open `build/coverage.html` in your browser to see:
- Line coverage percentages
- Branch coverage
- Function coverage
- Files with uncovered lines highlighted

### Install Coverage Tools

**Ubuntu/Debian:**
```bash
sudo apt-get install gcovr
```

**macOS:**
```bash
brew install gcovr
```

**Fedora/RHEL:**
```bash
sudo dnf install gcovr
```

**Any OS (Python):**
```bash
pip install gcovr
```

## GitHub Actions CI/CD

### Automatic Test Runs
Tests automatically run when you:
- Push to `main`, `master`, or `develop` branches
- Create a pull request targeting these branches

### Workflow Details

The workflow (`.github/workflows/tests.yml`) does:
1. **Runs on multiple compilers** (GCC and Clang)
2. **Compiles the project** with proper flags
3. **Executes all unit tests**
4. **Generates coverage reports** and uploads to Codecov
5. **Publishes test results** to GitHub

### Viewing Test Results

#### On Pull Requests
- Checks appear at the bottom of the PR
- Tests must pass before merging (if branch protection is enabled)
- Click "Details" to see full test output

#### In Actions Tab
- Go to **Actions** tab in your repository
- Select a workflow run to see details
- Download artifacts (test results, coverage reports)

#### In Checks Tab
- Each commit shows test status
- Green checkmark = all tests passed
- Red X = tests failed

## Setting Up Branch Protection

To prevent merging when tests fail:

1. Go to **Settings** → **Branches**
2. Under "Branch protection rules", click **Add rule**
3. Enter branch name (e.g., `main`)
4. Enable:
   - ✅ **Require a pull request before merging**
   - ✅ **Require status checks to pass before merging**
   - Select checks: `test` and `coverage`
5. Click **Create** or **Save changes**

Now, PRs cannot be merged until all tests pass and coverage is generated.

## Codecov Integration

Coverage data is automatically uploaded to [Codecov](https://codecov.io):

1. Visit https://codecov.io
2. Sign in with GitHub
3. Your repository should appear automatically
4. Codecov will show:
   - Coverage trends over time
   - Coverage by file
   - Merge request impact on coverage

## Test Organization

Tests are organized into two test suites:

### SpriteHolderTest (12 tests)
Tests the `SpriteHolder` class:
- Constructor and getters
- Pixel data storage
- Hash computation
- Boundary conditions
- Error handling

### OverlappingPatternsTest (13 tests)
Tests the `OverlappingPatterns` class:
- Pattern extraction
- Hash consistency
- Uniqueness validation
- Edge cases

## Adding New Tests

To add tests for a new class:

1. Create a new file `src/my_class_test.cpp`
2. Include Google Test headers:
   ```cpp
   #include <gtest/gtest.h>
   #include <my_class.h>
   ```
3. Write test cases:
   ```cpp
   class MyClassTest : public ::testing::Test {
   protected:
       // Setup code here
   };
   
   TEST_F(MyClassTest, TestName) {
       // Test code
       EXPECT_EQ(expected, actual);
   }
   ```
4. Add to `tests/CMakeLists.txt`:
   ```cmake
   add_executable(wfc_tests
       sprite_holder_test.cpp
       overlapping_patterns_test.cpp
       my_class_test.cpp  # Add this line
   )
   ```
5. Rebuild and run tests

## Google Test Assertions

Common assertions:
- `EXPECT_EQ(a, b)` - Assert equality
- `EXPECT_NE(a, b)` - Assert not equal
- `EXPECT_LT(a, b)` - Assert less than
- `EXPECT_GT(a, b)` - Assert greater than
- `EXPECT_TRUE(condition)` - Assert true
- `EXPECT_FALSE(condition)` - Assert false
- `EXPECT_THROW(code, exception)` - Assert exception thrown
- `EXPECT_NO_THROW(code)` - Assert no exception

See [Google Test docs](https://google.github.io/googletest/) for more.

## Troubleshooting

### Tests fail locally but pass in GitHub Actions
- Check compiler version: `gcc --version` vs Actions
- Try building with both GCC and Clang:
  ```bash
  cmake -DCMAKE_CXX_COMPILER=clang++ ..
  ```

### Coverage report not generating
- Install gcovr: `pip install gcovr`
- Make sure to rebuild with `-DWFC_ENABLE_COVERAGE=ON`
- Rebuild clean: `make clean && make`

### Tests won't compile
- Make sure headers have include guards (added in setup)
- Check that all dependencies are installed
- Rebuild: `cmake .. && make clean && make`

## Continuous Integration Best Practices

1. **Commit regularly** - Small, focused commits are easier to debug
2. **Write tests as you code** - Not after
3. **Aim for high coverage** - But focus on critical paths first
4. **Red-Green-Refactor** - Write failing test, make it pass, refactor
5. **Keep tests fast** - Slow tests discourage running them frequently
