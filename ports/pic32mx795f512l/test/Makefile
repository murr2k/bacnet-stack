# Makefile for PIC32MX795F512L BACnet Port API Regression Tests
# Author: Murray Kopit <murr2k@gmail.com>
# Date: 2025

# Use host compiler for tests
CC = gcc
CFLAGS = -Wall -Wextra -g -O0
CFLAGS += -I. -I.. -I../../../include -I../../../src
CFLAGS += -DTEST_MODE -DBACDL_MSTP -DMAX_APDU=480
CFLAGS += -DMAX_TSM_TRANSACTIONS=1 -DMAX_CHARACTER_STRING_BYTES=64
CFLAGS += -DMAX_OCTET_STRING_BYTES=64 -DBACAPP_MINIMAL
CFLAGS += -DPRINT_ENABLED=1 -DBACNET_PROTOCOL_REVISION=14

# Test source files
TEST_SRCS = \
	test_framework.c \
	test_main.c \
	test_ai_api.c \
	test_ao_api.c \
	test_bi_api.c \
	test_bo_api.c \
	test_device_api.c \
	test_mstp_api.c

# Port source files (to be tested)
PORT_SRCS = \
	../ai.c \
	../ao.c \
	../bi.c \
	../bo.c \
	../device.c \
	../dlmstp.c \
	../rs485.c \
	../stubs.c

# BACnet stack source files
BACNET_SRCS = \
	../../../src/bacnet/bacaddr.c \
	../../../src/bacnet/bacapp.c \
	../../../src/bacnet/bacdcode.c \
	../../../src/bacnet/bacerror.c \
	../../../src/bacnet/bacint.c \
	../../../src/bacnet/bacreal.c \
	../../../src/bacnet/bacstr.c \
	../../../src/bacnet/bactext.c \
	../../../src/bacnet/indtext.c \
	../../../src/bacnet/datetime.c \
	../../../src/bacnet/npdu.c \
	../../../src/bacnet/basic/sys/fifo.c \
	../../../src/bacnet/basic/sys/ringbuf.c \
	../../../src/bacnet/datalink/crc.c \
	../../../src/bacnet/datalink/mstp.c \
	../../../src/bacnet/datalink/cobs.c

# All source files
SRCS = $(TEST_SRCS) $(PORT_SRCS) $(BACNET_SRCS)

# Object files
OBJS = $(SRCS:.c=.o)

# Target executable
TARGET = run_tests

# Test artifacts
ARTIFACTS = \
	api_verification_report.html \
	API_VERIFICATION.md \
	test_results_*.xml \
	test_results_*.json

# Default target
all: $(TARGET)

# Build test executable
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ -lm

# Compile source files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Run tests and generate artifacts
test: $(TARGET)
	@echo "================================"
	@echo " Running API Verification Tests"
	@echo "================================"
	./$(TARGET)
	@echo ""
	@echo "Test artifacts generated:"
	@ls -la *.html *.md *.xml *.json 2>/dev/null | awk '{print "  " $$NF}'

# Quick test without full output
quick: $(TARGET)
	./$(TARGET) | grep -E "(PASS|FAIL|Total|Success)"

# Clean build files
clean:
	rm -f $(OBJS) $(TARGET) $(ARTIFACTS)

# View HTML report
report:
	@if [ -f api_verification_report.html ]; then \
		echo "Opening API verification report..."; \
		xdg-open api_verification_report.html 2>/dev/null || \
		open api_verification_report.html 2>/dev/null || \
		echo "Please open api_verification_report.html in a browser"; \
	else \
		echo "No report found. Run 'make test' first."; \
	fi

# Check API compliance
check: test
	@echo ""
	@if grep -q "FULLY COMPLIANT" API_VERIFICATION.md; then \
		echo "✓ API COMPLIANCE VERIFIED"; \
		echo "✓ All interfaces match published BACnet-stack API"; \
		exit 0; \
	else \
		echo "✗ API COMPLIANCE FAILED"; \
		echo "✗ Review test failures in API_VERIFICATION.md"; \
		exit 1; \
	fi

.PHONY: all test quick clean report check