# PIC32MX795F512L Port - ABI Compatibility Tests

This directory contains Application Binary Interface (ABI) compatibility tests for the PIC32MX795F512L BACnet port. These tests verify that the port maintains binary compatibility with the published BACnet-stack API headers.

## Purpose

The ABI compatibility tests ensure that:
- The port can be distributed as a compiled library
- All API functions have correct signatures
- Binary compatibility is maintained with the BACnet-stack headers
- Users can link against the compiled port without recompilation

## Test Coverage

The tests verify 101 API functions across:
- **Analog Input (AI)** - 21 functions
- **Analog Output (AO)** - 19 functions
- **Binary Input (BI)** - 17 functions
- **Binary Output (BO)** - 21 functions
- **MS/TP Datalink** - 14 functions
- **Device Object** - 9 functions

## Running Tests Locally

### Prerequisites
- GCC compiler
- Make build tool
- Linux/Unix environment (or WSL on Windows)

### Quick Test
```bash
# Run the automated CI test script
./run-ci-tests.sh
```

### Manual Testing
```bash
# Clean previous builds
make -f Makefile.host clean

# Build with host compiler
make -f Makefile.host

# Run ABI compatibility test
./test_abi_host

# View the report
cat ABI_COMPATIBILITY_REPORT.md
```

## GitHub Actions CI/CD

These tests automatically run in GitHub Actions when:
- Code is pushed to master/main/develop branches
- Pull requests are opened
- Changes affect the port or BACnet API files

### Workflow Status
The workflow is defined in `.github/workflows/pic32mx-abi-test.yml`

### PR Comments
For pull requests, the workflow automatically comments with test results.

## Test Files

### Core Test Files
- `test_abi_compatibility.c` - Main ABI verification test
- `test_framework.h/c` - Test framework with assertions
- `run-ci-tests.sh` - Automated test runner script

### Build Files
- `Makefile.host` - Builds port with host compiler for testing
- `Makefile.abi` - Alternative makefile for cross-compiled testing

### Output Files
- `test_abi_host` - Test executable
- `ABI_COMPATIBILITY_REPORT.md` - Generated compatibility report
- `build.log` - Compilation output
- `test.log` - Test execution output

## Interpreting Results

### Success Output
```
✅ BINARY COMPATIBLE
Compatibility Rate: 100.0%
```

This means the port can be safely distributed as a compiled library.

### Failure Output
```
❌ COMPATIBILITY ISSUES
Missing functions: X
Signature mismatches: Y
```

This indicates ABI breaks that must be fixed before distribution.

## Adding New Tests

To add tests for new API functions:

1. Edit `test_abi_compatibility.c`
2. Add function declarations in the appropriate section
3. Add `VERIFY_FUNCTION_EXISTS()` calls in the test function
4. Update the expected count in results validation

## Troubleshooting

### Common Issues

**Issue**: `xc.h: No such file or directory`
**Solution**: The tests use TEST_MODE to compile with host compiler, not XC32

**Issue**: Undefined reference to symbols
**Solution**: Add missing symbols to `stubs.c`

**Issue**: Test executable not found
**Solution**: Run `make -f Makefile.host` first

## Continuous Integration

### GitHub Actions Badge
Add to your README:
```markdown
![PIC32MX ABI Test](https://github.com/YOUR_USERNAME/bacnet-stack/actions/workflows/pic32mx-abi-test.yml/badge.svg)
```

### Running in Other CI Systems

The tests can run in any CI system that supports:
- Ubuntu/Debian Linux
- GCC compiler
- Basic shell scripting

Example for Jenkins:
```groovy
stage('ABI Test') {
    steps {
        sh 'cd ports/pic32mx795f512l/test && ./run-ci-tests.sh'
    }
}
```

## License

These tests are part of the BACnet-stack project and follow the same MIT license.

## Contact

Maintainer: Murray Kopit <murr2k@gmail.com>
